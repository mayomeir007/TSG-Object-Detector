# TSG Object Detector

C++ HTTP server that accepts an aerial/satellite image with geographic metadata, detects buildings using a YOLOv8 ONNX model, and returns a JSON array with each building's bounding box and computed lat/lon.

## Build

**Prerequisites:** Windows x64, MSVC v143 toolset (Visual Studio 2022 or Build Tools 2022).

### Option A — Visual Studio 2022 (with IDE)

1. Open `TSGObjectDetector.sln`.
2. Select **Debug|x64** or **Release|x64**.
3. Build → Build Solution (`Ctrl+Shift+B`).

### Option B — Visual Studio Build Tools 2022 (command-line, no IDE)

A lighter alternative (~4 GB vs ~8 GB). Install [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022), selecting the **Desktop development with C++** workload, then from a **Developer Command Prompt for VS 2022**:

```bat
msbuild TSGObjectDetector.sln /p:Configuration=Debug /p:Platform=x64
```

Replace `Debug` with `Release` for a release build.

---

Post-build events automatically copy the following to the output directory:
- `opencv_world452d.dll` / `opencv_world452.dll`
- `onnxruntime.dll`
- `config.ini`
- `Models\` folder

## Model

The YOLOv8 ONNX model is included in the repository at `Models/building_detector.onnx`.

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
| ONNX Runtime | 1.26.x | `Libraries/ONNXRuntime/` |
| cpp-httplib | 0.15.x | `Libraries/cpp-httplib/httplib.h` |
| nlohmann/json | 3.11.x | `Libraries/nlohmann/json.hpp` |

> **OpenCV** — headers and `.lib` are committed, but the DLLs (`opencv_world452.dll` / `opencv_world452d.dll`) exceed GitHub's file size limit (~800 MB each) and are excluded from the repository. Download [OpenCV 4.5.2 for Windows](https://github.com/opencv/opencv/releases/tag/4.5.2), extract, and copy both DLLs from `build\x64\vc14\bin\` into `Libraries/OpenCV/bin/`.

> **ONNX Runtime** — fully committed including `onnxruntime.dll` (1.26.x). No manual step needed.

