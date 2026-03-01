# Desk Status Display

An ESP32-S3 BLE-controlled LED matrix that shows your availability at work.

| Color  | Meaning                |
|--------|------------------------|
| Green  | Available              |
| Yellow | Busy but interruptible |
| Red    | Do Not Disturb         |

## Hardware

- [Waveshare ESP32-S3-Matrix](https://www.waveshare.com/esp32-s3-matrix.htm) (8x8 WS2812B, GPIO14)

## Flashing the firmware

Requires [PlatformIO](https://platformio.org/install).

```bash
# Build
pio run

# Build and upload (connect board via USB-C)
pio run -t upload

# Monitor serial output
pio device monitor
```

## Setting the status

### Option A: Python script (PC)

```bash
cd client
pip install -r requirements.txt

python set_status.py green      # Available
python set_status.py yellow     # Busy
python set_status.py red        # Do Not Disturb
python set_status.py off        # LEDs off

# If you know the MAC address (faster, skips scan):
python set_status.py --address AA:BB:CC:DD:EE:FF green
```

### Option B: nRF Connect (phone)

1. Install **nRF Connect** ([Android](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) / [iOS](https://apps.apple.com/app/nrf-connect-for-mobile/id1054362403))
2. Scan and connect to **DeskStatus**
3. Find service `12345678-1234-5678-1234-56789abcdef0`
4. Tap the write button on characteristic `...def1`
5. Write one of these hex values:
   - `00` — Off
   - `01` — Green (Available)
   - `02` — Yellow (Busy)
   - `03` — Red (Do Not Disturb)

## BLE details

| Property       | Value                                          |
|----------------|------------------------------------------------|
| Device name    | `DeskStatus`                                   |
| Service UUID   | `12345678-1234-5678-1234-56789abcdef0`         |
| Status char.   | `12345678-1234-5678-1234-56789abcdef1` (R/W)   |
