#ifndef LED_H
#define LED_H

#include "driver/rmt_tx.h"
#include "LEDEncoder.h"
#include "Color.h"

#include <array>

namespace led
{
    template <unsigned int GpioPin>
    struct Strip final
    {
        Color color{};

        Strip()
        {
            rmt_tx_channel_config_t channelConfig{};
            channelConfig.gpio_num = static_cast<gpio_num_t>(GpioPin);
            channelConfig.clk_src = RMT_CLK_SRC_APB; // Check Power Management

            channelConfig.resolution_hz = 10'000'000; // 10MHz -> 100ns period

            channelConfig.mem_block_symbols = 48;
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
            ESP_ERROR_CHECK(rmt_transmit(channel, &encoder, colorArray.data(), sizeof(Color), &transmitConfig));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, -1));
        }

    private:
        rmt_channel_handle_t channel = nullptr;
        rmt_transmit_config_t transmitConfig{};
        LEDEncoder encoder{};
    };
}
#endif