#ifndef LEDENCODER_H
#define LEDENCODER_H

#include "driver/rmt_tx.h"

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

        rmt_encoder_handle_t bytes;
        rmt_encoder_handle_t copy;
        LEDEncoderState state = RESET;

        rmt_symbol_word_t code0;
        rmt_symbol_word_t code1;
        rmt_symbol_word_t resetCode;

        LEDEncoder()
        {
            // tick timings of codes
            code0.level0 = 1;
            code0.duration0 = 3; // 300ns
            code0.level1 = 0;
            code0.duration1 = 7; // 700ns

            code1.level0 = 1;
            code1.duration0 = 7;
            code1.level1 = 0;
            code1.duration1 = 3;

            resetCode.level0 = 0;
            resetCode.duration0 = resetTime;
            resetCode.level1 = 0;
            resetCode.duration1 = resetTime;

            // bytes encoder
            rmt_bytes_encoder_config_t bytesEncoderConfig;
            bytesEncoderConfig.bit0 = code0;
            bytesEncoderConfig.bit1 = code1;
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