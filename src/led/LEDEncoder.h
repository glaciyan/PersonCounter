#ifndef LEDENCODER_H
#define LEDENCODER_H

#include "driver/rmt_tx.h"
#include "DataCode.h"

namespace led
{
    size_t led_encode(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
    esp_err_t led_reset(rmt_encoder_t *encoder);
    esp_err_t led_del(rmt_encoder_t *encoder);

    enum LEDEncoderState
    {
        RESET = 0,
        COLORS_DONE,
    };

    struct LEDEncoder final : public rmt_encoder_t
    {
        const uint16_t resetTime = 2800; // 280us

        LEDEncoderState state = RESET;

        rmt_encoder_handle_t bytes;
        rmt_encoder_handle_t copy;

        DataCode code0{3, 7};
        DataCode code1{7, 3};
        rmt_symbol_word_t resetCode;

        LEDEncoder()
        {
            resetCode.level0 = 0;
            resetCode.duration0 = resetTime;
            resetCode.level1 = 0;
            resetCode.duration1 = 0;

            // bytes encoder
            rmt_bytes_encoder_config_t bytesEncoderConfig;
            bytesEncoderConfig.bit0 = code0.word;
            bytesEncoderConfig.bit1 = code1.word;
            bytesEncoderConfig.flags.msb_first = 1;
            ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytesEncoderConfig, &bytes));

            // copy encoder
            rmt_copy_encoder_config_t copyEncoderConfig{};
            ESP_ERROR_CHECK(rmt_new_copy_encoder(&copyEncoderConfig, &copy));

            encode = led_encode;
            reset = led_reset;
            del = led_del;
        }
    };
}

#endif