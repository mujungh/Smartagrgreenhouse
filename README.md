# Smart Agriculture Greenhouse

This project provides a simple end-to-end greenhouse solution with:

- A Python web dashboard
- An ESP32-S3 greenhouse controller sketch
- An ESP32-CAM streaming sketch

## Features

- Display greenhouse temperature
- Display brightness
- Display cooling fan status
- Display SG90-driven shade panel status
- Show ESP32-CAM live stream
- Automatically enable the fan based on temperature
- Automatically move the shade panel based on brightness

## Structure

```text
smartagr/
|-- firmware/
|   |-- esp32cam_stream/
|   `-- esp32s3_greenhouse/
|-- public/
|-- server/
`-- requirements.txt
```

## Run The Python Server

```bash
pip install -r requirements.txt
python server/app.py
```

Open `http://localhost:3000`.

## API

- `GET /api/state`: return the current greenhouse state
- `POST /api/state`: update state from the ESP32-S3 using `Authorization: Bearer smartagr-demo-key`
- `GET /api/stream`: real-time dashboard updates via Server-Sent Events

## ESP32-S3 Setup

Update these values in `firmware/esp32s3_greenhouse/esp32s3_greenhouse.ino`:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `SERVER_URL`
- `API_KEY`
- `CAMERA_STREAM_URL`
- `LDR_PIN`
- `THERMISTOR_PIN`
- `FAN_PIN`
- `SERVO_PIN`

## ESP32-CAM Setup

Update these values in `firmware/esp32cam_stream/esp32cam_stream.ino`:

- `WIFI_SSID`
- `WIFI_PASSWORD`

After flashing, copy the serial output stream URL such as `http://192.168.1.88:81/stream`
into `CAMERA_STREAM_URL` in the ESP32-S3 sketch.

## Control Logic

- Fan on at `>= 30 C`
- Fan off at `<= 28 C`
- Shade closed at `>= 2500`
- Shade open at `<= 1800`
