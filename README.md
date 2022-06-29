# IEC_SBus

[2022-06-29]

這是一份於 SBus 協議下，解算 remote control (RC) receiver PWM 訊號的程式庫。
目前經過測試後，支援的晶片：
* Teensy 3.2
* ESP32 NodeMCU-32S
![nodemcu-32s-eps32-pinout](https://user-images.githubusercontent.com/91120147/176430811-2b2828b2-d854-4221-850a-9b14144b662f.jpeg)
[圖片來源：https://hk.botsheet.com/product/nodemcu-32s-eps32/]

## 傳輸協議：
* 總共會傳 25 個 byte。
* 第一個 byte 是起始 byte，定義成 `0x0F`。
* 最後二個 byte 則是結尾 `0x00`。
* 所以中間 22 個 byte 則是資料 byte，當收到之後，會利用 `位元運算` 來把它解算成實際的資料 (RC 命令)。

## 位元運算

## How to use?
* 硬體腳位：
以 ESP32 NodeMCU36S 為例：
將 receiver 的訊號腳位接到 Serial1 的 Rx1 (GPIO9)，電源線接電源腳位，接地線接接地腳位。
* 範例程式碼：
```cpp
#include <Arduino.h>
#include <IEC_SBus.h>

SBUS SBus(&Serial1);

void setup()
{
    Serial.begin(115200);
    SBus.Begin();
}

void loop()
{
    uint16_t RC_Channel[16];

    int isUpdated = SBus.Read(RC_Channel);
    if (isUpdated == 1)
    {
        for (int i = 0; i < 16; i++)
        {
            Serial.print(String() + RC_Channel[i] + " " );
        }
        Serial.println();
    }

    delay(1);
}
```
* 注意事項：根據經驗，在燒錄 ESP32 時，似乎 Rx1 腳位不能有接收到訊號。因此建議在燒錄失敗時，先把 receiver 拔掉，等待燒錄成功後，再把它燒錄進去。
