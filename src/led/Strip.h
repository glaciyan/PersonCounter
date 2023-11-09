#ifndef LED_H
#define LED_H

#include "driver/rmt_tx.h"
#include "LEDEncoder.h"
#include "Color.h"

#include <array>
#include <cstring>

namespace led
{
    template <unsigned int GpioPin, uint16_t Leds>
    struct Strip final
    {
        Color color{};

        Strip()
        {
            rmt_tx_channel_config_t channelConfig{};
            channelConfig.gpio_num = static_cast<gpio_num_t>(GpioPin);
            channelConfig.clk_src = RMT_CLK_SRC_APB; // Check Power Management

            channelConfig.resolution_hz = 10'000'000; // 10MHz -> 100ns period

#ifdef CHIP_ESP32S3
            channelConfig.mem_block_symbols = 48;
#else
            channelConfig.mem_block_symbols = 64;
#endif

            channelConfig.trans_queue_depth = 4;
            channelConfig.flags.invert_out = false;
            channelConfig.flags.with_dma = false;

            ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &channel));

            transmitConfig.loop_count = 0;
        }

        void enable() const
        {
            ESP_ERROR_CHECK(rmt_enable(channel));
        }

        void disable() const
        {
            ESP_ERROR_CHECK(rmt_disable(channel));
        }

        void transmit()
        {
            auto colorArray = color.asGRB();

            // TODO quick hack to see if this works, do proper strip
            uint8_t buffer[Leds][3];

            for (int i = 0; i < Leds; i++)
            {
                std::memcpy(buffer[i], colorArray.data(), 3);
            }

            ESP_ERROR_CHECK(rmt_transmit(channel, &encoder, buffer, sizeof(Color) * Leds, &transmitConfig));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
        }

    private:
        rmt_channel_handle_t channel = nullptr;
        rmt_transmit_config_t transmitConfig{};
        LEDEncoder encoder{};
    };
}
#endif