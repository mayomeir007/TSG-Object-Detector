# TSG Object Detector

C++ HTTP server that accepts an aerial/satellite image with geographic metadata, detects buildings using a YOLOv8 ONNX model, and returns a JSON array with each building's bounding box and computed lat/lon.

## Build

**Prerequisites:** Visual Studio 2022 (v143 toolset), Windows x64.

1. Open `TSGObjectDetector.sln` in Visual Studio 2022.
2. Select **Debug|x64** or **Release|x64**.
3. Build → Build Solution (`Ctrl+Shift+B`).

Post-build events automatically copy the following to the output directory:
- `opencv_world452d.dll` / `opencv_world452.dll`
- `onnxruntime.dll`
- `config.ini`
- `Models\` folder

## Model

The YOLOv8 ONNX model is included in the repository at `Models/building_detector.onnx` (~12 MB). No separate download is needed — it is committed directly to git.

Expected model I/O format (standard YOLOv8 ONNX export):
- Input: `[1, 3, 640, 640]` — RGB, normalized 0–1, CHW layout
- Output: `[1, 4 + num_classes, 8400]` — `cx, cy, w, h` + per-class scores

To swap in a different model, replace the file path in `config.ini` under `[detector]`.

## Run

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

Error cases:
- Missing `config.ini` → exits immediately with a clear error message
- Missing form field → `400 Bad Request`
- Invalid image bytes → `400 Bad Request`

## Configuration (`config.ini`)

```ini
[server]
port = 8080
host = 0.0.0.0

[detector]
model          = Models\building_detector.onnx
confidence     = 0.5
nms            = 0.45
building_class = 0
```

| Key | Description |
|---|---|
| `model` | Path to the `.onnx` file (relative to the working directory or absolute) |
| `confidence` | Score threshold — anchors below this are discarded before NMS |
| `nms` | IoU threshold for non-maximum suppression |
| `building_class` | Class index in the model that corresponds to buildings |

## Architecture

- **Detector**: YOLOv8 ONNX inference via ONNX Runtime. Letterbox-resizes input to 640×640, runs inference, applies NMS, maps detections back to original image coordinates. Swap `Detector.cpp` to change the model without touching anything else.
- **GeoConverter**: Pure math — converts pixel bounding box centers to lat/lon given top-left corner and meters-per-pixel.
- **JsonBuilder**: nlohmann/json single-header.
- **Server**: cpp-httplib single-header, `POST /detect` multipart/form-data.

## Libraries (vendored)

| Library | Version | Location |
|---|---|---|
| OpenCV | 4.5.2 | `Libraries/OpenCV/` |
| ONNX Runtime | (downloaded separately) | `Libraries/ONNXRuntime/` |
| cpp-httplib | 0.15.x | `Libraries/cpp-httplib/httplib.h` |
| nlohmann/json | 3.11.x | `Libraries/nlohmann/json.hpp` |

> **ONNX Runtime** headers and binaries are not committed to this repository due to size. Download `onnxruntime-win-x64-*.zip` from the [ONNX Runtime releases page](https://github.com/microsoft/onnxruntime/releases) and extract into `Libraries/ONNXRuntime/` with the structure `include/`, `lib/`, `bin/`.
