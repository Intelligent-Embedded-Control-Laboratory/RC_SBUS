#include "sbus.h"

namespace IEC
{
void SBUS::begin()
{
#if defined(ESP32) // ESP32
    pSerial->begin(100000, SERIAL_8E2); // 注意：需要訊號反向電路
#else
    // Teensy 3.2
    pSerial->begin(100000, SERIAL_8E1_RXINV); // SERIAL_8E1_RXINV: 訊號反向 (所以不需要額外建置訊號反向電路)
#endif
}

int SBUS::read(uint16_t *_dest_channels)
{
    isUpdated = 0; // Not yet ready
    while (pSerial->available() >= NUM_OF_BYTES_RECEV)
    {
        if ((uint8_t)(pSerial->read()) == start_byte)
        {
            pSerial->readBytes(buf, BUF_SIZE);

            if ((uint8_t)(pSerial->read()) == finish_byte && (uint8_t)(pSerial->read()) == finish_byte)
            {
                isUpdated = 1; // The SBUS stream is updated.
                decode(_dest_channels, buf); // Decode the SBUS data
            }
            else
            {
                isUpdated = -2; // Lost finish byte
            }
        }
        else
        {
            isUpdated = -1; // Lost begin byte
        }
    }
    return isUpdated;
}

void SBUS::decode(uint16_t *_dest_channels, uint8_t *_buffer)
{
    _dest_channels[0] = (uint16_t)((_buffer[0] | _buffer[1] << 8) & 0x07FF);
    _dest_channels[1] = (uint16_t)((_buffer[1] >> 3 | _buffer[2] << 5) & 0x07FF);
    _dest_channels[2] = (uint16_t)((_buffer[2] >> 6 | _buffer[3] << 2 | _buffer[4] << 10) & 0x07FF);
    _dest_channels[3] = (uint16_t)((_buffer[4] >> 1 | _buffer[5] << 7) & 0x07FF);
    _dest_channels[4] = (uint16_t)((_buffer[5] >> 4 | _buffer[6] << 4) & 0x07FF);
    _dest_channels[5] = (uint16_t)((_buffer[6] >> 7 | _buffer[7] << 1 | _buffer[8] << 9) & 0x07FF);
    _dest_channels[6] = (uint16_t)((_buffer[8] >> 2 | _buffer[9] << 6) & 0x07FF);
    _dest_channels[7] = (uint16_t)((_buffer[9] >> 5 | _buffer[10] << 3) & 0x07FF);
    _dest_channels[8] = (uint16_t)((_buffer[11] | _buffer[12] << 8) & 0x07FF);
    _dest_channels[9] = (uint16_t)((_buffer[12] >> 3 | _buffer[13] << 5) & 0x07FF);
    _dest_channels[10] = (uint16_t)((_buffer[13] >> 6 | _buffer[14] << 2 | _buffer[15] << 10) & 0x07FF);
    _dest_channels[11] = (uint16_t)((_buffer[15] >> 1 | _buffer[16] << 7) & 0x07FF);
    _dest_channels[12] = (uint16_t)((_buffer[16] >> 4 | _buffer[17] << 4) & 0x07FF);
    _dest_channels[13] = (uint16_t)((_buffer[17] >> 7 | _buffer[18] << 1 | _buffer[19] << 9) & 0x07FF);
    _dest_channels[14] = (uint16_t)((_buffer[19] >> 2 | _buffer[20] << 6) & 0x07FF);
    _dest_channels[15] = (uint16_t)((_buffer[20] >> 5 | _buffer[21] << 3) & 0x07FF);
}
}