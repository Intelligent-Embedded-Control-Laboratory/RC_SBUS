// 2023-01-19
#include <Arduino.h>
#include <sbus.h>

#define HW_SERIAL_INTERFACE_SBUS Serial1

IEC::SBUS sbus(HW_SERIAL_INTERFACE_SBUS);
uint16_t rc[16] = {0};

void setup()
{
    // Begin USB serial communication
    Serial.begin(115200);

    // Begin SBus data streaming
    sbus.begin();
}

void loop()
{
    // Pooling sbus data stream
    if (sbus.read(rc) == 1)
    {
        for (auto &c : rc)
        {
            Serial.print(c);
            Serial.print(" ");
        }
        Serial.println();
        Serial.flush();
    }
}