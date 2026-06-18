# TSG Object Detector

C++ HTTP server that accepts an aerial/satellite image with geographic metadata, detects buildings via OpenCV contour analysis, and returns a JSON array with each building's bounding box and computed lat/lon.

## Build

1. Open `TSGObjectDetector.sln` in Visual Studio 2022.
2. Select **Debug|x64** or **Release|x64**.
3. Build → Build Solution (Ctrl+Shift+B).

The `opencv_world452d.dll` (Debug) or `opencv_world452.dll` (Release) is copied to the output directory by a post-build event.

## Run

Place `config.ini` next to the `.exe` (or in the working directory), then run the executable:

```
TSGObjectDetector.exe
```

The server starts on `0.0.0.0:8080` by default.

## Test

```bash
curl -X POST http://localhost:8080/detect \
  -F "image=@path/to/aerial.jpg" \
  -F "lat=32.0800" \
  -F "lon=34.7800" \
  -F "mpp=0.5"
```

Expected response:

```json
[
  {
    "object_id": 1,
    "bounding_box": [120, 300, 80, 60],
    "location": { "latitude": 32.0802, "longitude": 34.7815 }
  }
]
```

## Configuration (`config.ini`)

```ini
[server]
port = 8080
host = 0.0.0.0
```

## Architecture

- **Detector**: OpenCV contour-based stub — replace `Detector.cpp` to swap in an ONNX model without touching anything else.
- **GeoConverter**: Pure math, no dependencies.
- **JsonBuilder**: nlohmann/json single-header.
- **Server**: cpp-httplib single-header, `POST /detect` multipart/form-data.
