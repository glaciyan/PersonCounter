#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "driver/rmt_tx.h"
#include "soc/clk_tree_defs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "led/Strip.h"
#include "led/Color.h"
#include "MainStrip.h"
#include "Animation.h"

#include <cmath>
#include <sstream>
#include <thread>
#include <numbers>
#include <unordered_map>

#define EXAMPLE_ESP_WIFI_SSID "esp32-wifi"
#define EXAMPLE_ESP_WIFI_PASS "AB54ADADNdfry832372DBNAORuBmacbe"
#define EXAMPLE_ESP_WIFI_CHANNEL 5
#define EXAMPLE_MAX_STA_CONN 6

#define MaxDeviceAge std::chrono::seconds(15)
// #define MaxDeviceAge std::chrono::minutes(4)

std::mutex devices_mutex;
std::unordered_map<std::string, std::chrono::system_clock::time_point> devices{};

led::Color black{0, 0, 0};
led::Color red{255, 0, 0};
led::Color green{0, 255, 0};
led::Color blue{0, 0, 255};
led::Color cyan{0, 255, 255};
led::Color white{255, 255, 255};

Animation clear{LEDAnimation::CLEAR, black};
Animation fade{LEDAnimation::FADE, black};
Animation red_pulse{LEDAnimation::PULSE, red};
Animation cyan_pulse{LEDAnimation::PULSE, cyan};
Animation green_pulse{LEDAnimation::PULSE, green};
Animation white_pulse{LEDAnimation::PULSE, white};

QueueHandle_t led_queue;

bool animationRunning = false;
std::mutex animationRunningMutex;

void stopAnimation()
{
    animationRunningMutex.lock();
    animationRunning = false;
    animationRunningMutex.unlock();
}

const char *LED_TAG = "LED";

void ledAnimationTask(void *parameters)
{
    auto highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
    ESP_LOGI(LED_TAG, "!!!!!!!!!!!!!!!!!!!!!!! Managed to get into Task with %o bytes used", highWaterMark);

    auto *led = (LED *)parameters;
    while (true)
    {
        Animation animation = clear;
        BaseType_t rs = xQueueReceive(led_queue, &animation, portMAX_DELAY);
        if (rs == pdFALSE)
        {
            ESP_LOGI(LED_TAG, "could not get value out of queue");
            continue;
        }

        switch (animation.type)
        {
        case LEDAnimation::CLEAR:
            led->color.clear();
            led->transmit();
            break;

        case LEDAnimation::FADE:
            animationRunningMutex.lock();
            animationRunning = true;
            animationRunningMutex.unlock();
            while (animationRunning)
            {
                TickType_t time = xTaskGetTickCount();
                double hue = (std::sin(time / 200.0) * 180) + 180;
                led->color.setFromHSV(hue, 1.0, 0.6);

                led->transmit();
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            break;

        case LEDAnimation::PULSE:
            animationRunningMutex.lock();
            animationRunning = true;
            animationRunningMutex.unlock();

            TickType_t startTime = xTaskGetTickCount();

            while (animationRunning)
            {
                TickType_t time = xTaskGetTickCount();
                if (startTime + PULSE_LENGHT_TICKS <= time)
                {
                    led->color.clear();
                    led->transmit();

                    stopAnimation();
                    break;
                }

                double value = (std::sin((time - startTime) / (double)PULSE_LENGHT_TICKS * std::numbers::pi));
                // led->color.setFromHSV(180, 1, value / 5);

                const double limit = 0.6;

                led->color.red = animation.color.red * value * limit;
                led->color.green = animation.color.green * value * limit;
                led->color.blue = animation.color.blue * value * limit;
                led->transmit();
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            break;
        }
    }

    vTaskDelete(nullptr);
}

int runAnimation(Animation *anim)
{
    BaseType_t rs = xQueueSend(led_queue, anim, 0);
    if (rs == errQUEUE_FULL)
    {
        ESP_LOGW(LED_TAG, "Couldn't send out animation. Queue was full.");
        return 1;
    }

    return 0;
}

int forceRunAnimation(Animation *anim)
{
    stopAnimation();

    BaseType_t rs = xQueueSendToFront(led_queue, anim, 0);
    if (rs == errQUEUE_FULL)
    {
        ESP_LOGW(LED_TAG, "Couldn't send out animation to the front of the queue. It was full.");
        return 1;
    }

    return 0;
}

std::string mac_to_stdstring(uint8_t mac[])
{
    std::stringstream out{};
    out << std::hex << static_cast<int>(mac[0]) << ":" << std::hex << static_cast<int>(mac[1]) << ":" << std::hex << static_cast<int>(mac[2]) << ":" << std::hex << static_cast<int>(mac[3]) << ":" << std::hex << static_cast<int>(mac[4]) << ":" << std::hex << static_cast<int>(mac[5]);
    return out.str();
}

const char *WIFI_TAG = "WIFI";

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_PROBEREQRECVED)
    {
        wifi_event_ap_probe_req_rx_t *event = (wifi_event_ap_probe_req_rx_t *)event_data;

        std::string mac = mac_to_stdstring(event->mac);

        devices_mutex.lock();
        devices.insert(std::pair{mac, std::chrono::system_clock::now()});
        devices_mutex.unlock();

        ESP_LOGI(WIFI_TAG, "Found someone (%s)", mac.c_str());
        ESP_LOGI(WIFI_TAG, "Unique Devices: %d", devices.size());

        forceRunAnimation(&cyan_pulse);
    }
}

const char *TAG = "Main";

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    LED led{};
    led.enable();

    led_queue = xQueueCreate(16, sizeof(Animation));

    BaseType_t rs = xTaskCreatePinnedToCore(ledAnimationTask, "LEDAnimation", CONFIG_MAIN_TASK_STACK_SIZE, &led, 1, nullptr, 1);

    if (rs != pdPASS)
    {
        ESP_LOGI(TAG, "Could not create Task");

        if (rs == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
        {
            ESP_LOGI(TAG, "Not enough memory");
        }
    }

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_wifi_set_event_mask(0));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr));

    wifi_config_t ap_config = {};
    strcpy((char *)ap_config.ap.ssid, EXAMPLE_ESP_WIFI_SSID);
    strcpy((char *)ap_config.ap.password, EXAMPLE_ESP_WIFI_PASS);
    ap_config.ap.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
    ap_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config_t wifi_config{ap_config};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WIFI SoftAP started.");
    forceRunAnimation(&green_pulse);
    runAnimation(&green_pulse);

    // show off mode
    // runAnimation(&fade);

    // int clear = LEDAnimation::CLEAR;
    // BaseType_t rs2 = xQueueSend(led_queue, &clear, 0);
    // if (rs2 == errQUEUE_FULL)
    // {
    //     ESP_LOGI(TAG, "could not send out animation queue was full");
    // }

    while (true)
    {
        ESP_LOGI(TAG, "Cleaning up old MACs...");

        devices_mutex.lock();

        int32_t removed_devices = 0;
        auto now = std::chrono::system_clock::now();
        for (auto it = devices.begin(); it != devices.end();)
        {
            auto device = *it;
            if (now - device.second > MaxDeviceAge)
            {
                it = devices.erase(it);
                removed_devices++;
            }
            else
            {
                it++;
            }
        }

        devices_mutex.unlock();

        if (removed_devices > 0)
        {
            ESP_LOGI(TAG, "Removed %" PRId32 " devices", removed_devices);
            runAnimation(&red_pulse);
        }

        ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    led.disable();
}