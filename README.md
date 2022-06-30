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
* 序列埠傳輸方式：8E2。意思是一串列埠數據傳送會包含 8 bits (1 byte)，以及兩個停止 bits (2 stop bit)，此外有偶教驗 (even parity)。並且，開始訊號為反向。因此，
    * 在 teensy 初始化，會用
    ```cpp
    Serial1.begin(100000, SERIAL_8E1_RXINV); // 假設使用 Serail1 (Rx1) 作為接收 receiver 訊號的腳位
    ```
    因為不支持 8E2 所以用 8E1 也是可以。
    * 而 ESP32 NodeMCU-32S，則是
    ```cpp
    Serial1.begin(100000, SERIAL_8E2); // 假設使用 Serail1 (Rx1) 作為接收 receiver 訊號的腳位
    ```
    要注意的是，這裡就需要訊號反向電路。
## 位元運算

## How to use?
* 硬體腳位：
以 ESP32 NodeMCU-32S 為例：
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
            Serial.print(String() + RC_Channel[i] + " ");
        }
        Serial.println();
    }

    delay(1);
}
```
* 注意事項：根據經驗，在燒錄 ESP32 時，似乎 Rx1 腳位不能有接收到訊號。因此建議在燒錄失敗時，先把 receiver 拔掉，等待燒錄成功後，再把它燒錄進去。
