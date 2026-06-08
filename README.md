# VisionFlow

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue)]()
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)]()

> A modular, multithreaded camera pipeline service written in C for embedded and desktop Linux systems.

---

## Overview

VisionFlow (`vf_service`) is a modular image processing pipeline built in C.
It captures frames from a V4L2 camera device, passes them through a
producer-consumer pipeline, converts color space, and writes output frames to disk.

The project demonstrates production-grade embedded software engineering:

- Multithreaded pipeline with blocking queues and atomic shutdown
- Pre-allocated buffer pools with atomic reference counting
- Service lifecycle manager with clean init/start/stop/deinit separation
- Unix domain socket control channel with JSON protocol
- Per-unit instrumentation-based latency profiling
- Runtime control via CLI client without restarting the service
- Professional Git workflow: Kanban board, Conventional Commits, atomic commits

---

## Pipeline

**Current:**
```
camera_unit → stream_provider_unit → conversion_unit → file_out_unit
```

**Planned:**
```
camera_unit → stream_provider_unit → conversion_unit → encoder_unit → file_out_unit
                                                     ↘ display_unit
```

Each unit runs on a dedicated thread. Frames flow through pre-allocated buffer
pools and blocking queues. Shutdown follows consumer-first ordering to avoid
deadlocks on pipeline teardown.

---

## Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                         vf_service                           │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                     Pipeline Engine                    │  │
│  │                  (vf_pipeline_manager)                 │  │
│  │                                                        │  │
│  │   ┌─────────────┐     ┌──────────────────┐            │  │
│  │   │ camera_unit │────▶│ stream_provider   │            │  │
│  │   └─────────────┘     └────────┬─────────┘            │  │
│  │                                │                      │  │
│  │                       ┌────────▼─────────┐            │  │
│  │                       │   conv_unit      │            │  │
│  │                       └────────┬─────────┘            │  │
│  │                                │                      │  │
│  │                       ┌────────▼─────────┐            │  │
│  │                       │  file_out_unit   │            │  │
│  │                       └──────────────────┘            │  │
│  │                                                        │  │
│  │        [ encoder_unit ]  [ display_unit ]  ← planned  │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│   ┌──────────────────┐       ┌──────────────────────────┐   │
│   │   Buffer Manager │       │      Control Server      │   │
│   │  (pool + queue)  │       │   (Unix domain socket)   │◀──┼── vf_client
│   └──────────────────┘       └──────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘
```

---

## Modules

| Module          | Location                 | Status   | Responsibility                              |
|-----------------|--------------------------|----------|---------------------------------------------|
| vf_common       | libraries/vf_common      | ✅ Done  | Error model, common types                   |
| vf_logger       | libraries/vf_logger      | ✅ Done  | Structured logging (DEBUG/INFO/WARN/ERROR)  |
| vf_file         | libraries/vf_file        | ✅ Done  | File I/O abstraction                        |
| vf_framebuffer  | libraries/vf_framebuffer | ✅ Done  | Frame buffer with atomic reference counting |
| vf_conversion   | libraries/vf_conversion  | ✅ Done  | YUYV→RGB888 color space conversion          |
| vf_camera       | libraries/vf_camera      | ✅ Done  | V4L2 camera capture with mmap               |
| libvf_control   | libvf_control/           | ✅ Done  | Unix domain socket control server           |
| Pipeline Engine | service/src/core         | ✅ Done  | Unit orchestration and service lifecycle    |
| Buffer Manager  | service/src/buffer-mgr   | ✅ Done  | Pre-allocated pool and blocking queue       |
| vf_encoder      | libraries/vf_encoder     | 🔲 Planned | H.264 video encoding via libav            |
| vf_display      | libraries/vf_display     | 🔲 Planned | Frame display output via SDL2             |

---

## Project Structure

```
vf_service/
├── libraries/
│   ├── vf_common/          # Error model and common types
│   ├── vf_logger/          # Structured logging
│   ├── vf_file/            # File I/O abstraction
│   ├── vf_framebuffer/     # Frame buffer with reference counting
│   ├── vf_conversion/      # Color space conversion
│   ├── vf_camera/          # V4L2 camera capture
│   ├── vf_encoder/         # [planned] H.264 encoding
│   └── vf_display/         # [planned] Frame display
├── libvf_control/          # Unix socket control library
│   ├── public/             # Public headers
│   └── src/                # Server and handler implementation
├── service/
│   ├── public-inc/         # Service public headers
│   └── src/
│       ├── core/           # Pipeline manager and service lifecycle
│       ├── buffer-mgr/     # Buffer pool and blocking queue
│       ├── parser/         # JSON config parser
│       ├── notifier/       # Event notification
│       ├── units/
│       │   ├── camera/     # Camera capture unit
│       │   ├── stream-provider/  # Stream provider unit
│       │   ├── conversion/ # Color space conversion unit
│       │   ├── file/       # File output unit
│       │   ├── encoder/    # [planned] Encoder unit
│       │   └── display/    # [planned] Display unit
│       └── main.c
├── test_clients/
│   ├── vf_client/          # CLI control client (runtime IPC)
│   ├── vf_camera/          # Camera module validation
│   ├── vf_conversion/      # Conversion module validation
│   ├── vf_file/            # File module validation
│   ├── vf_framebuffer/     # Framebuffer module validation
│   ├── vf_logger/          # Logger module validation
│   ├── vf_buff_mgr/        # Buffer manager validation
│   └── vf_service/         # Service integration validation
├── docs/
│   └── architecture/       # Architecture overview and ADRs
├── vf_frames/
│   ├── conf/               # JSON pipeline configuration
│   ├── raw/                # Raw input test frames
│   ├── rgb/                # RGB input test frames
│   ├── yuv/                # YUV input test frames
│   └── out/                # Pipeline output frames
└── README.md
```

---

## Prerequisites

```bash
# Ubuntu 22.04+
sudo apt-get install build-essential
```

### Camera Simulation (no physical camera required)

VisionFlow uses a V4L2 virtual camera device fed by FFmpeg. This allows
full pipeline validation without hardware dependency.

```bash
# Install dependencies
sudo apt-get install ffmpeg v4l2loopback-dkms v4l-utils

# Load virtual camera kernel module
sudo modprobe v4l2loopback devices=1 video_nr=0 card_label="VirtualCam"

# Feed frames into the virtual device (run before starting vf_service)
ffmpeg -re -stream_loop -1 -i input.mp4 \
       -vf scale=640:480 \
       -pix_fmt yuyv422 \
       -f v4l2 /dev/video0
```

---

## Build

```bash
cd service/build
make all
```

After a successful build, `vf_service` and `vf_client` are symlinked
to `/usr/local/bin` and can be invoked from any working directory.

```bash
# Remove build artifacts and uninstall system binaries
make clean
```

---

## Run

```bash
# Start the pipeline (ensure FFmpeg feed is running first)
vf_service --camera
```

Output frames are written to `vf_frames/out/camera/` as raw RGB files.

---

## Runtime Control

`vf_client` connects to `vf_service` via a Unix domain socket and sends
JSON commands. The service does not need to be restarted.

```bash
vf_client --get_status   # Check if the pipeline is running
vf_client --get_stats    # Per-unit frame count and latency averages
vf_client --stop         # Stop the pipeline gracefully
vf_client --start        # Restart the pipeline
```

### Example responses

```json
{"status":"ok","data":{"service":"running"}}
```

```json
{"status":"ok","data":{"units":[
  {"name":"camera_in",       "frames":159, "avg_get_ms":5, "avg_process_ms":0, "avg_send_ms":0},
  {"name":"stream_provider", "frames":159, "avg_get_ms":6, "avg_process_ms":0, "avg_send_ms":0},
  {"name":"conversion",      "frames":156, "avg_get_ms":0, "avg_process_ms":5, "avg_send_ms":0},
  {"name":"file_out",        "frames":155, "avg_get_ms":2, "avg_process_ms":0, "avg_send_ms":3}
]}}
```

---

## Configuration

> 🔲 Planned — JSON-based pipeline configuration is under development.

The pipeline topology and parameters (device path, resolution, format,
buffer count) will be configurable via a JSON file, without recompilation.

---

## CI/CD

> 🔲 Planned — automated build and code style enforcement is under development.

Planned pipeline:
- Build verification on each push
- Static analysis (MISRA subset via Polyspace or cppcheck)
- Code style enforcement via clang-format

---

## Architecture Decision Records

Key design decisions are documented as ADRs in `docs/adr/`.

### ADR-001 — Producer-Consumer pipeline with blocking queues

**Decision:** Each pipeline unit runs on a dedicated thread. Units communicate
via blocking queues backed by pre-allocated buffer pools.

**Rationale:** Decouples unit execution rates, avoids busy-waiting, and maps
directly to the producer-consumer model used in embedded multimedia pipelines.
Blocking queues with `pthread_cond_wait` allow threads to sleep when no data
is available, reducing CPU usage compared to polling.

**Trade-offs:** Adds complexity in shutdown ordering. Consumer-first teardown
is required to avoid threads blocking indefinitely on an empty queue after
the producer has exited.

---

### ADR-002 — Pre-allocated buffer pool with atomic reference counting

**Decision:** Frame buffers are allocated once at startup in a fixed-size pool.
Units acquire buffers from the pool, pass them by reference, and release them
via atomic reference counting.

**Rationale:** Avoids dynamic allocation in the hot path, which is critical
for predictable latency in embedded systems. Reference counting allows
multiple units to hold a frame simultaneously without copying.

**Trade-offs:** Pool size is fixed at compile time. If all slots are in use,
`acquire` blocks until one is released. Pool capacity must be sized for the
worst-case pipeline depth.

---

### ADR-003 — Atomic shutdown with consumer-first teardown

**Decision:** Shutdown is triggered by setting an atomic flag and broadcasting
on all condition variables. Pipeline units are stopped in reverse order
(consumer-first).

**Rationale:** Stopping producers first would leave consumers blocked on empty
queues. Consumer-first teardown ensures each unit exits cleanly before its
upstream producer is stopped, avoiding deadlocks.

**Trade-offs:** Shutdown ordering must be maintained manually. Adding new units
to the pipeline requires updating the teardown sequence.

---

### ADR-004 — Unix domain socket for runtime control channel

**Decision:** `vf_service` exposes a control interface via a Unix domain socket
at `/tmp/vf_control.sock`. Commands and responses use a JSON-over-stream
protocol with newline termination.

**Rationale:** Unix domain sockets are a standard IPC mechanism on Linux,
suitable for local process-to-process communication. JSON provides a
human-readable, extensible protocol without requiring a binary serialization
library. The socket approach allows `vf_client` to be a standalone binary
with no shared state.

**Trade-offs:** The server processes one request at a time (single-threaded
accept loop). This is acceptable given that control commands are rare and
fast. If concurrent clients or long-running commands become a requirement,
a thread-per-connection or async model would be needed.

---

### ADR-005 — Service lifecycle manager separating resource allocation from execution

**Decision:** A dedicated lifecycle manager (`vf_service_t`) owns all pipeline
resources and exposes four operations: init, start, stop, and deinit.
Init allocates resources; start launches threads; stop joins threads; deinit
frees resources.

**Rationale:** Separating allocation from execution allows the pipeline to be
stopped and restarted without reallocating buffers or reinitializing the
camera. This is necessary for the runtime stop/start control channel to work
correctly.

**Trade-offs:** Shutdown flags in the buffer pool and queues must be explicitly
reset before each restart, since they are set during the previous stop sequence.

---

### ADR-006 — Instrumentation-based latency profiling per pipeline unit

**Decision:** Each unit records cumulative time spent in `get_data`,
`process_data`, and `send_data` phases. Averages are exposed via the
control channel as `get_stats`.

**Rationale:** Per-phase timing isolates bottlenecks without requiring an
external profiler. The instrumentation runs in production code with negligible
overhead (integer arithmetic and timestamp reads).

**Trade-offs:** Averages smooth over spikes. Per-frame timestamps are not
stored, so outlier frames cannot be identified post-hoc without additional
instrumentation.

---

## Memory Validation

The service has been verified with Valgrind Memcheck and Helgrind.

### Memcheck — memory leaks

```bash
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./service/build/vf_service --camera
```

**Result:**

```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 1,046 allocs, 1,046 frees, 9,150,763 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts
```

### Helgrind — thread race conditions

```bash
valgrind --tool=helgrind \
         ./service/build/vf_service --camera
```

**Result:**

```
ERROR SUMMARY: 0 errors from 0 contexts
```

---

## Contributing

All contributions follow a strict Git workflow:

- **Default branch:** `develop`
- **No direct push** to `develop` — all code via Pull Request
- **Branch naming:** `feature/VF-X-short-description`
- **Commit messages:** [Conventional Commits](docs/commit-messages.txt)
- **Merge strategy:** Rebase and merge only

See [docs/commit-messages.txt](docs/commit-messages.txt) for full conventions.

---

## License

MIT License — see [LICENSE](LICENSE) for details.
