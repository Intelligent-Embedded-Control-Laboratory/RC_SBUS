#include "IEC_SBus.h"

void SBUS::Begin()
{
#if defined(ESP32) // ESP32
    pSerial->begin(100000, SERIAL_8E2); // 注意：需要訊號反向電路
#else
    // Teensy 3.2
    pSerial->begin(100000, SERIAL_8E1_RXINV); // SERIAL_8E1_RXINV: 訊號反向 (所以不需要額外建置訊號反向電路)
#endif
}

int SBUS::Read(uint16_t *Channel)
{
    isUpdated = 0;
    while (pSerial->available() >= NUM_OF_BYTES_RECEV)
    {
        if ((uint8_t)(pSerial->read()) == start_byte)
        {
            pSerial->readBytes(buf, 22);

            if ((uint8_t)(pSerial->read()) == finish_byte && (uint8_t)(pSerial->read()) == finish_byte)
            {
                isUpdated = 1;
                Decode(Channel, buf);
            }
            else
            {
                isUpdated = -2;
            }
        }
        else
        {
            isUpdated = -1;
        }
    }
    return isUpdated;
}

void SBUS::Decode(uint16_t *channels, uint8_t *_payload)
{
    // 來自 Bolder Flight Systems
    channels[0] = (uint16_t)((_payload[0] | _payload[1] << 8) & 0x07FF);
    channels[1] = (uint16_t)((_payload[1] >> 3 | _payload[2] << 5) & 0x07FF);
    channels[2] = (uint16_t)((_payload[2] >> 6 | _payload[3] << 2 | _payload[4] << 10) & 0x07FF);
    channels[3] = (uint16_t)((_payload[4] >> 1 | _payload[5] << 7) & 0x07FF);
    channels[4] = (uint16_t)((_payload[5] >> 4 | _payload[6] << 4) & 0x07FF);
    channels[5] = (uint16_t)((_payload[6] >> 7 | _payload[7] << 1 | _payload[8] << 9) & 0x07FF);
    channels[6] = (uint16_t)((_payload[8] >> 2 | _payload[9] << 6) & 0x07FF);
    channels[7] = (uint16_t)((_payload[9] >> 5 | _payload[10] << 3) & 0x07FF);
    channels[8] = (uint16_t)((_payload[11] | _payload[12] << 8) & 0x07FF);
    channels[9] = (uint16_t)((_payload[12] >> 3 | _payload[13] << 5) & 0x07FF);
    channels[10] = (uint16_t)((_payload[13] >> 6 | _payload[14] << 2 | _payload[15] << 10) & 0x07FF);
    channels[11] = (uint16_t)((_payload[15] >> 1 | _payload[16] << 7) & 0x07FF);
    channels[12] = (uint16_t)((_payload[16] >> 4 | _payload[17] << 4) & 0x07FF);
    channels[13] = (uint16_t)((_payload[17] >> 7 | _payload[18] << 1 | _payload[19] << 9) & 0x07FF);
    channels[14] = (uint16_t)((_payload[19] >> 2 | _payload[20] << 6) & 0x07FF);
    channels[15] = (uint16_t)((_payload[20] >> 5 | _payload[21] << 3) & 0x07FF);
}