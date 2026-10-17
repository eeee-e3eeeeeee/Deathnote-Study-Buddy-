# Deathnote-Study-Buddy-
The deathnote study buddy is a study buddy based off of the classic anime Deathnote. It has study timers, and notifies you if you get distracted and move it around. It plays looping gifs, and also when its not being used as a timer it can be used as a clock.
<img width="611" height="616" alt="Image" src="https://github.com/user-attachments/assets/159aaf3c-4210-44b7-9c57-7e5770daf287" />
## What it does
- **Live Clock:** Syncs the current time over Wi-Fi.
- **Pomodoro Modes:** Easily switch between 25-minute focus blocks, 5-minute breaks, and heavy 50-minute deep focus sessions.
- **Animated Backgrounds:** Streams looping GIFs.
- **Anti-Cheat Motion Detector:** Uses an LIS3DH accelerometer to monitor movement. If you shake or move the device too much, it flashes an "L IS WATCHING" warning to force you back to work.
- **Simple Controls:** Managed with two push buttons.

## Wiring & Pin Map
| Function | XIAO Pin |
|---|---|
| TFT DIN/MOSI | D10 |
| TFT CLK/SCK | D3 |
| TFT DC | D1 |
| TFT RST | D2 |
| TFT CS | GND *(only if your module supports permanently tied CS)* |
| LIS3DH SDA | D4 |
| LIS3DH SCL | D5 |
| Start/Pause Button | D6 to GND |
| Mode Button | D7 to GND |

## Getting Started (Arduino IDE)
1. Install the Arduino IDE along with the Espressif ESP32 board package.
2. Set your board target to **XIAO ESP32C3**.
3. Open the Library Manager and install these dependencies:
   - Adafruit GFX Library
   - Adafruit ST7735 and ST7789 Library
   - Adafruit LIS3DH
   - Adafruit Unified Sensor
   - AnimatedGIF (by Larry Bank)
5. Open `deathnotestudybuddy_oct11a.ino`, select the correct serial port, and hit upload.

## Adding Animations (`.gif`)
will update this when i get the physical board and can add gifs to it 
