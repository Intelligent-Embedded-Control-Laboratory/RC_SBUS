# IEC_SBus

[2022-06-29]

這是一份於 SBus 協議下，解算 remote control (RC) receiver PWM 訊號的程式庫。
目前經過測試後，支援的晶片：

- Teensy 3.2/4.0
- ESP32 NodeMCU-32S
  ![nodemcu-32s-eps32-pinout](https://user-images.githubusercontent.com/91120147/176430811-2b2828b2-d854-4221-850a-9b14144b662f.jpeg)
  [圖片來源：<https://hk.botsheet.com/product/nodemcu-32s-eps32/>]

## 序列埠傳輸協議

- 一般來說，MCU 裡面有專門處理序列埠的硬體晶片。
- 這個晶片接收到開始電位時會開始擷取資料，結束電位會停止擷取，最後把資料存到軟體 buffer。
- 開始電位會打一個單位時間的 LOW，如果格式是 1 個停止位元，就會打一個單位時間的 HIGH。(假設現在沒有奇偶校驗) **一個單位時間，就是 baud rate 的倒數。
- 中間傳輸的資料如果是 0100 0011，則對應的電位就是

  ```cpp
             資料位元組
      起始位 [0 1 0 0 0 0 1 1] 停止位
      LOW    [L H L L L L H H] HIGH
      ↑                        ↑
      給硬體晶片看，硬體晶片會把資料位元組存到 buffer。
  ```

- 如果現在有奇偶校驗，假設現在是偶校驗，這個是說會加上一個校驗位，讓資料位元組加上校驗位的 1 有偶數個。一樣以上面的例子說明：

  - 因為資料 `[0 1 0 0 0 0 1 1]` 裡面有 3 個，所以會在後面一位放一個 1，使得總共 1 的個數為偶數個。
  - 所以如果資料是收到 `[0 1 1 0 0 0 1 1]`，但偶校驗是 1，就可以知道資料發生 bit 翻轉 (錯誤)。但這種校驗方式的缺點就是如果同時錯兩個位元，奇偶校驗沒辦法偵錯，也無法知道錯在哪。

  ```cpp
             資料位元組        奇偶校驗位
      起始位 [0 1 0 0 0 0 1 1] 1         停止位
      LOW    [L H L L L L H H]           HIGH
      ↑                                  ↑
      給硬體晶片看，硬體晶片會把資料位元組存到 buffer。
  ```

## SBus 傳輸協議

- 序列埠端：
  - Baud rate 固定為 100000。
  - 序列埠的通訊協議採用 8E2，也就是在傳送一個位元組資料的同時，會傳送一個偶校驗位，以及兩個停止位元。
  - 要注意的是，SBus 的訊號是反向的，可以在硬體端加上訊號翻轉電路，也可以在軟體端設定 (如果有支援)。
- 資料解碼端：
  - 總共會傳 25 個 byte。
  - 第一個 byte 是起始 byte，定義成 `0x0F`。
  - 最後二個 byte 則是結尾 `0x00`。
  - 所以中間 22 個 byte 則是資料 byte，當收到之後，會利用 `位元運算` 來把它解算成實際的資料 (RC 命令)。
- 序列埠傳輸方式：8E2。意思是一串列埠數據傳送會包含 8 bits (1 byte)，以及兩個停止 bits (2 stop bit)，此外有偶教驗 (even parity)。並且，開始訊號為反向。因此，

  - 在 teensy 初始化，會用

  ```cpp
  Serial1.begin(100000, SERIAL_8E1_RXINV); // 假設使用 Serail1 (Rx1) 作為接收 receiver 訊號的腳位
  ```

  因為不支持 8E2 所以用 8E1 也是可以。

  - 而 ESP32 NodeMCU-32S，則是

  ```cpp
  Serial1.begin(100000, SERIAL_8E2); // 假設使用 Serail1 (Rx1) 作為接收 receiver 訊號的腳位
  ```

  要注意的是，這裡就需要反向器。

## 位元運算

- 首先，必須認知的是，在 SBus 通訊協議下，資料的位元寬度是 11 bits，也就是說，十進制的值是從 0 到 2047 共 2048 個。
- 但每次傳送資料會是以 8 bits 為一個單位傳送，所以會有 3 個 bits 在第二個位元組當中，圖示如下：

  ```cpp
      Byte[0] = [b8 b7 b6 b5 b4 b3 b2 b1]
      Byte[1] = [b16 b15 b14 b13 b12 b11 b10 b9]
      ...
      Byte[21] = ...
  ```

- 也就是說，第一筆資料包含在 `第一個位元組 Byte[0] 的 [b8 b7 b6 b5 b4 b3 b2 b1]` 以及 `第二個位元組 Byte[1] 的 [b11 b10 b9]`。
- 因此，我們要進行資料解碼時，我們先把 `第二個位元組左移 8 bits`，並且`跟第一個位元組擺在一起 (進行 or 運算)`，然後用 16 bits 寬度的資料型態容納這些資料，圖示說明如下：

  ```cpp
      變數                      位元
      (uint16_t) Byte[0]      : [0 0 0 0 0 0 0 0 , b8 b7 b6 b5 b4 b3 b2 b1]
      (uint16_t) Byte[1]      : [0 0 0 0 0 0 0 0 , b16 b15 b14 b13 b12 b11 b10 b9]
      (uint16_t) Byte[1] << 8 : [b16 b15 b14 b13 b12 b11 b10 b9 , 0 0 0 0 0 0 0 0]

      (uint16_t)(Byte[0]) 跟 (uint16_t)(Byte[1] << 8) 做 OR 運算會得到：
      [b16 b15 b14 b13 b12 b11 b10 b9 , b8 b7 b6 b5 b4 b3 b2 b1]
  ```

- 對應的程式碼：

  ```cpp
      // 這是一個 16 bits 的資料，相當於
      // [b16 b15 b14 b13 b12 b11 b10 b9 , b8 b7 b6 b5 b4 b3 b2 b1]
      (uint16_t) (Byte[0] | Byte[1] << 8);
  ```

- 但，[b16 b15 b14 b13 b12] 是屬於下一筆資料的，所以我們跟 `0x07FF` 進行 AND 運算，`0x07FF` 的二進制表示法是 `0B0000011111111111`，也就是說

  ```cpp
          [  0   0   0   0   0   1   1  1    1  1  1  1  1  1  1  1] // 這二個進行 AND 運算
          [b16 b15 b14 b13 b12 b11 b10 b9 , b8 b7 b6 b5 b4 b3 b2 b1] // 這二個進行 AND 運算
      >>  [  0   0   0   0   0 b11 b10 b9 , b8 b7 b6 b5 b4 b3 b2 b1]
  ```

- 所以，資料解碼方式如下：

  ```cpp
      uint16_t data[16];
      data[0] = (uint16_t)((Byte[0] | Byte[1] << 8) & 0x07FF);
  ```

- 其它的資料解碼按照相同的邏輯。

## How to use?

- 硬體腳位：
  以 ESP32 NodeMCU-32S 為例：
  將 receiver 的訊號腳位接到 Serial1 的 Rx1 (GPIO9)，電源線接電源腳位，接地線接接地腳位。
- 範例程式碼：

```cpp
#include <Arduino.h>
#include <sbus.h>

SBUS SBus(Serial1);

void setup()
{
    Serial.begin(115200);
    SBus.begin();
}

void loop()
{
    uint16_t RC_Channel[16];

    int isUpdated = SBus.read(RC_Channel);
    if (isUpdated == 1)
    {
        for (int i = 0; i < 16; i++)
        {
            Serial.print(RC_Channel[i]);
            Serial.print(" ");
        }
        Serial.println();
    }

    delay(1);
}
```

- 注意事項：根據經驗，在燒錄 ESP32 時，似乎 Rx1 腳位不能有接收到訊號。因此建議在燒錄失敗時，先把 receiver 拔掉，等待燒錄成功後，再把它燒錄進去。
