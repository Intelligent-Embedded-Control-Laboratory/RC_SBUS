/**
 * @file IEC_SBus.h
 * @author Lee Ho-Yang and Li Yang-Rui (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-06-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <Arduino.h>

#define BUF_SIZE 22
#define NUM_OF_BYTES_RECEV 25

class SBUS
{
public:
    SBUS(HardwareSerial *_pSerial)
    {
        pSerial = _pSerial;
    }

    void Begin();
    int Read(uint16_t *Channel);

protected:
    HardwareSerial *pSerial;
    uint8_t buf[BUF_SIZE];
    const uint8_t start_byte = 0x0F;
    const uint8_t finish_byte = 0x00;

    int isUpdated;
    void Decode(uint16_t *channels, uint8_t *_payload);
};
