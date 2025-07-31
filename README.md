# OV7670 QVGA Image Capture with Arduino Mega + ESP8266

This project demonstrates interfacing an **OV7670 camera** with an **Arduino Mega 2560** board, capturing QVGA images (320×240), and transmitting the image data over UART to an embedded **ESP8266 module** for further processing or transmission.

### 🧠 Features

- Initializes OV7670 camera in **YUV422 format**
- Captures QVGA image frames and stores them on SD card
- Sends image data over **UART3** at 2Mbps to ESP01 client module
- Uses **ESP01 Client pin status** to trigger data transmission
- Implements **software image buffering** and structured UART transmission
- Fully compatible with **SD cards** using SPI interface on pin `CS = 13`

---

## 📦 Requirements

### Hardware

- Arduino Mega 2560
- OV7670 camera module (without FIFO)
- ESP8266 ESP01 module
- SD card module
- SD card formatted to FAT
- Jumper wires and external 3.3V regulator for OV7670 + ESP8266

### Software

- Arduino IDE
- `Wire.h` and `SD.h` libraries

---

## 🛠 Setup & Pin Configuration

| Peripheral       | Arduino Mega Pin    | Description               |
|------------------|---------------------|---------------------------|
| OV7670 PCLK      | PE4 (INT4)          | Pixel Clock               |
| OV7670 VSYNC     | PE5 (INT5)          | Vertical Sync             |
| OV7670 Data      | PORTA               | 8-bit parallel data bus   |
| XCLK to OV7670   | Pin 11 (output)     | 8MHz PWM clock source     |
| ESP01 INIT       | PC6                 | ESP init state            |
| ESP01 CLIENT     | PC7                 | ESP is ready for image    |
| SD card CS       | Pin 13              | SPI Chip Select           |
| UART TX          | Serial3 (TX3)       | Sends data to ESP8266     |

---

## 🧾 File Structure

Captured image frames are stored as `.RID` files containing raw pixel data:
- File size: ~76,800 bytes per image (320×240)
- Format: raw bytes sampled from `PINA`

---

## 🌀 Functions Explained

### `Init_OV7670()`
Resets and configures camera with basic register settings.

### `Init_QVGA()`
Sets up image size scaling and start/stop positions.

### `Init_YUV422()`
Enables YUV color format output from OV7670.

### `QVGA_Image(title)`
Captures and stores one QVGA frame from OV7670 to SD card.

### `send_data_uart3(data)`
Sends one byte over UART3 to ESP8266.

### `send_log(log)`
Wraps log messages in framed transmission protocol (`0x02` ... `0xFF`).

---

## 🚀 How It Works

1. Initializes camera and communication interfaces.
2. Waits for VSYNC pulse to synchronize frame capture.
3. Reads pixel data row by row and stores into SD card.
4. Checks if ESP01 client is ready using `ESP01_CLIENT_PIN`.
5. Sends image file byte-by-byte over UART3 with logging.
6. Captures next frame in loop.

---

## 📚 Notes

- Make sure ESP8266 is properly level-shifted if running at 3.3V.
- Image data is raw and requires post-processing on ESP or host device.
- Adjust `_delay_ms()` if frame rate or transmission stability needs tweaking.

---

## 🧑‍💻 Author

Modified by JNGCoding 
Original base by **Hardik Kalasua**

---

## 🔖 License

MIT License – free to use, modify, and distribute.
