#ifndef DATACODE_H
#define DATACODE_H

#include "driver/rmt_tx.h"

namespace led
{
    struct DataCode final
    {
        rmt_symbol_word_t word;

        DataCode(uint16_t highTime, uint16_t lowTime)
        {
            word.level0 = 1;
            word.duration0 = highTime;
            word.level1 = 0;
            word.duration1 = lowTime;
        }
    };
}

#endif