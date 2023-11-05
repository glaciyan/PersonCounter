#pragma once

#include "driver/rmt_tx.h"

size_t led_encode(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
}