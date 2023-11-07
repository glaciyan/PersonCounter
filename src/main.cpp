#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "soc/clk_tree_defs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "LEDEncoder.h"
#include "color.h"

#include <cmath>
#include <thread>

const char *TAG = "Main";

rmt_encoder_t *bytes;

extern "C" void app_main()
{
    rmt_tx_channel_config_t channelConfig{};
    channelConfig.gpio_num = static_cast<gpio_num_t>(GPIO_LED);
    channelConfig.clk_src = RMT_CLK_SRC_APB; // Check Power Management

    channelConfig.resolution_hz = 10'000'000; // 10MHz -> 100ns period

    channelConfig.mem_block_symbols = 64;
    channelConfig.trans_queue_depth = 4; // 4 backlog items, source: my ass
    channelConfig.flags.invert_out = false;
    channelConfig.flags.with_dma = false;

    ESP_LOGI(TAG, "res: %lu\n", channelConfig.resolution_hz);

    rmt_channel_handle_t channel = nullptr;

    // create channel
    ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &channel));

    // grb array, set to green

    // transmit data
    rmt_transmit_config_t transmitConfig{};
    transmitConfig.loop_count = 0;

    blinker::LEDEncoder encoder{};

    // int32_t sequence[6][3] = {
    //     {1, 0, 0},
    //     {1, 1, 0},
    //     {0, 1, 0},
    //     {0, 1, 1},
    //     {0, 0, 1},
    //     {1, 0, 1},
    // };

    // int32_t currentSequence = 0;

    // enable the transmission channel
    ESP_ERROR_CHECK(rmt_enable(channel));

    Color color{};
    color.setFromHSV(130, 1., 1.);

    while (true)
    {
        TickType_t time = xTaskGetTickCount();
        // ESP_LOGI(TAG, "%f, %f", time / 100.0, std::sin(time / 100.0) * 255);
        // uint8_t brightness = std::floor(std::sin(time / 100.0) * 127.5) + 127.5;
        // uint16_t hue = std::ceil(std::sin(time / 100.0) * 127.5) + 127.5;
        // uint8_t hue = (time / 2) & 255;
        double hue = (std::sin(time / 100.0) * 180) + 180;
        printf(">hue:%f\n", hue);
        color.setFromHSV(hue, 1., 1);

        encoder.grb[0] = color.green;
        encoder.grb[1] = color.red;
        encoder.grb[2] = color.blue;
        printf(">red:%u\n", color.red);
        printf(">green:%u\n", color.green);
        printf(">blue:%u\n", color.blue);

        // encoder.grb[0] = sequence[currentSequence][0] * brightness;
        // encoder.grb[1] = sequence[currentSequence][1] * brightness;
        // encoder.grb[2] = sequence[currentSequence][2] * brightness;
        // ESP_LOGI(TAG, "brightness: %u", brightness);

        // send out color
        ESP_ERROR_CHECK(rmt_transmit(channel, &encoder, &encoder.grb, 3, &transmitConfig));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ESP_ERROR_CHECK(rmt_disable(channel));
}