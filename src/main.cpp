#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "soc/clk_tree_defs.h"
#include "LEDEncoder.h"
#include "led_strip.h"

#include <thread>

#define GPIO_LED (GPIO_NUM_48)


// GPIO assignment
#define LED_STRIP_BLINK_GPIO  48
// Numbers of the LED in the strip
#define LED_STRIP_LED_NUMBERS 1
// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

const char *TAG = "STAMP";

rmt_encoder_t *bytes;

led_strip_handle_t configure_led(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_NUMBERS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags{.invert_out = false,}                // whether to invert the output signal
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .flags{.with_dma = false,}              // DMA feature is available on ESP target like ESP32-S3
#endif
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return led_strip;
}

extern "C" void app_main()
{
    led_strip_handle_t led_strip = configure_led();
    bool led_on_off = false;

    ESP_LOGI(TAG, "Start blinking LED strip");
    while (1) {
        if (led_on_off) {
            /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
            for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++) {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 1, 1, 1));
            }
            /* Refresh the strip to send data */
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            ESP_LOGI(TAG, "LED ON!");
        } else {
            /* Set all LED off to clear all pixels */
            ESP_ERROR_CHECK(led_strip_clear(led_strip));
            ESP_LOGI(TAG, "LED OFF!");
        }

        led_on_off = !led_on_off;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    // rmt_tx_channel_config_t channelConfig{};
    // channelConfig.gpio_num = GPIO_LED;
    // channelConfig.clk_src = RMT_CLK_SRC_APB; // Check Power Management

    // channelConfig.resolution_hz = 10'000'000; // 10MHz -> 100ns period

    // channelConfig.mem_block_symbols = 64; // 24 * 10
    // channelConfig.trans_queue_depth = 4;   // 4 backlog items, source: my ass
    // channelConfig.flags.invert_out = false;
    // channelConfig.flags.with_dma = false;
    // channelConfig.flags.io_loop_back = false;
    // channelConfig.flags.io_od_mode = false;

    // ESP_LOGI(TAG, "res: %lu\n", channelConfig.resolution_hz);

    // rmt_channel_handle_t channel = nullptr;

    // // create channel
    // ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &channel));

    


    // // grb array, set to green
    

    // // enable the transmission channel
    // ESP_ERROR_CHECK(rmt_enable(channel));

    // // transmit data
    // rmt_transmit_config_t transmitConfig{};
    // transmitConfig.loop_count = 0;

    // blinker::LEDEncoder encoder{};

    // // rmt_bytes_encoder_config_t bytesEncoderConfig;
    // // bytesEncoderConfig.bit0 = encoder.code0;
    // // bytesEncoderConfig.bit1 = encoder.code1;
    // // ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytesEncoderConfig, &bytes));

    // ESP_ERROR_CHECK(rmt_transmit(channel, &encoder, &encoder.grb, 8*3, &transmitConfig));
    // ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
        
    // while (true)
    // {
    //     // ESP_LOGI(TAG, "LED ON");
    // }
}