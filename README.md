# esp-led-matrix

ESP32 shows text sent over WiFi on a 32x8 MAX7219 LED matrix.

## Hardware
- ESP32 DevKit V1
- 4-in-1 MAX7219 32x8 matrix (FC-16)

## Wiring (matrix IN side to ESP32)
| Matrix | ESP32 |
|---|---|
| VCC | VIN |
| GND | GND |
| DIN | D23 |
| CLK | D18 |
| CS  | D5  |

Optional: 1000uF cap across VCC/GND, 330 ohm resistor in series on DIN.

## Setup
1. Arduino IDE: install board package **esp32 by Espressif Systems**, select **ESP32 Dev Module**.
2. Install library **MD_Parola** (with MD_MAX72XX).
3. Set `SSID` / `PASS` in `esp-led-matrix/esp-led-matrix.ino`, upload.

## Usage
On boot the matrix shows its IP. From any device on the same WiFi:

```
GET http://<esp-ip>/show?text=hello
```

Returns `ok`. Short text stays still, long text scrolls. ESP32 only joins 2.4 GHz WiFi.

## Web UI (phone, via Termux)
```
pkg install python git
git clone https://github.com/AZ9tumas/esp-led-matrix
cd esp-led-matrix/web
python -m http.server 8000
```
Open `http://localhost:8000` in the phone browser, enter the ESP32 IP once (it's remembered), type text, press Send.
