#pragma once

#include "driver/rmt_tx.h"

namespace blinker
{
    size_t led_encode(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
    esp_err_t led_reset(rmt_encoder_t *encoder);
    esp_err_t led_del(rmt_encoder_t *encoder);

    enum LEDEncoderState
    {
        RESET = 0,
        COLORS_DONE,
    };

    struct LEDEncoder : public rmt_encoder_t
    {
        const uint16_t resetTime = 500; // 280us

        rmt_encoder_handle_t bytes;
        rmt_encoder_handle_t copy;
        LEDEncoderState state = RESET;
        uint8_t grb[3];

        rmt_symbol_word_t code0;
        rmt_symbol_word_t code1;
        rmt_symbol_word_t resetCode;

        LEDEncoder()
        {
            // tick timings of codes
            code0.level0 = 1;
            code0.duration0 = 3; // 300ns
            code0.level1 = 0;
            code0.duration1 = 9; // 700ns

            code1.level0 = 1;
            code1.duration0 = 9;
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

            grb[0] = 255;
            grb[1] = 0;
            grb[2] = 0;
        }
    };

    size_t led_encode(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
    {
        LEDEncoder *ledc = static_cast<LEDEncoder *>(encoder);
        rmt_encode_state_t sessionState = RMT_ENCODING_RESET;
        rmt_encode_state_t state = RMT_ENCODING_RESET;
        size_t encodedSymbols = 0;
        rmt_encoder_handle_t bytesEncoder = ledc->bytes;
        rmt_encoder_handle_t copyEncoder = ledc->copy;

        if (ledc->state == RESET)
        {
            encodedSymbols += bytesEncoder->encode(bytesEncoder, tx_channel, primary_data, data_size, &sessionState);
            if (sessionState & RMT_ENCODING_COMPLETE)
            {
                ledc->state = COLORS_DONE;
            }
            else if (sessionState & RMT_ENCODING_MEM_FULL)
            {
                state = RMT_ENCODING_MEM_FULL;
                goto ret;
            }
        }

        if (ledc->state == COLORS_DONE)
        {
            encodedSymbols += copyEncoder->encode(copyEncoder, tx_channel, &ledc->resetCode, sizeof(ledc->resetCode), &sessionState);
            if (sessionState & RMT_ENCODING_COMPLETE)
            {
                ledc->state = RESET;
                state = RMT_ENCODING_COMPLETE;
            }
            if (sessionState & RMT_ENCODING_MEM_FULL)
            {
                state = RMT_ENCODING_MEM_FULL;
                goto ret;
            }
        }

    ret:
        *ret_state = state;
        return encodedSymbols;
    }

    esp_err_t led_reset(rmt_encoder_t *encoder)
    {
        LEDEncoder *ledc = static_cast<LEDEncoder *>(encoder);
        rmt_encoder_reset(ledc->bytes);
        rmt_encoder_reset(ledc->copy);
        ledc->state = RESET;
        return ESP_OK;
    }

    esp_err_t led_del(rmt_encoder_t *encoder)
    {
        LEDEncoder *ledc = static_cast<LEDEncoder *>(encoder);
        rmt_del_encoder(ledc->bytes);
        rmt_del_encoder(ledc->copy);
        return ESP_OK;
    }

}