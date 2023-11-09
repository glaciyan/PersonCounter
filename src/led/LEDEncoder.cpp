#include "LEDEncoder.h"

namespace led
{
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