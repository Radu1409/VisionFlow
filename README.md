# VisionFlow

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Language](https://img.shields.io/badge/language-C-blue)]()
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)]()

> A modular, pipeline-based image processing service written in C — designed for embedded and desktop systems.

---

## Overview

VisionFlow is a **modular image processing pipeline service** built in C. It is designed around principles common in embedded multimedia systems: zero-copy frame passing, predictable memory usage, runtime configurability via JSON, and a clearly separated modular architecture.

The project is structured to demonstrate production-grade software engineering practices:
- Architecture-first design with documented decisions (ADRs)
- Independent, reusable library components
- Pipeline engine with a common processing unit interface
- CI/CD with build and code style enforcement
- Professional Git workflow with Conventional Commits

---

## Architecture

```
┌────────────────────────────────────────────────────┐
│                    vf_service                      │
│                                                    │
│  ┌──────────┐    ┌──────────────────────────────┐  │
│  │  Config  │───▶│       Pipeline Engine        │  │
│  │  Parser  │    │   (vf_pipeline_manager)      │  │
│  └──────────┘    │                              │  │
│                  │  ┌────────┐  ┌────────────┐  │  │
│  ┌──────────┐    │  │ camera │  │    file    │  │  │
│  │  Buffer  │───▶│  │  unit  │  │    unit    │  │  │
│  │ Manager  │    │  └────────┘  └────────────┘  │  │
│  └──────────┘    │  ┌──────────┐  ┌──────────┐  │  │
│                  │  │conversion│  │ encoder  │  │  │
│  ┌──────────┐    │  │   unit   │  │   unit   │  │  │
│  │  Logger  │    │  └──────────┘  └──────────┘  │  │
│  └──────────┘    │       ┌──────────────┐       │  │
│                  │       │ display unit │       │  │
│                  │       └──────────────┘       │  │
│                  └──────────────────────────────┘  │
└────────────────────────────────────────────────────┘

         ▲                         │
         │                         ▼
   test_clients/             vf_frames/
   (manual validation)       (test assets)
```

**Key Principles:**
- **Zero-copy** — frames passed by reference between units
- **Modular** — each library is independently testable
- **Config-driven** — pipeline topology defined via JSON
- **Common interface** — every unit implements init/process/deinit

---

## MVP Pipeline

```
file_unit → conversion_unit → file_unit
```

Read a raw frame from file, convert color space (RGB↔YUV), write result to file. End-to-end pipeline validation without hardware dependency.

---

## Modules

| Module | Location | Responsibility |
|--------|----------|----------------|
| vf_common | libraries/vf_common | Error model, common types |
| vf_logger | libraries/vf_logger | Structured logging (DEBUG/INFO/WARN/ERROR) |
| vf_file | libraries/vf_file | File I/O abstraction |
| vf_framebuffer | libraries/vf_framebuffer | Frame buffer abstraction |
| vf_conversion | libraries/vf_conversion | RGB↔YUV color space conversion |
| vf_camera | libraries/vf_camera | Camera capture abstraction |
| vf_encoder | libraries/vf_encoder | Video encoding (H264) |
| vf_display | libraries/vf_display | Frame display output |
| Pipeline Engine | service/src/core | Unit orchestration and execution |
| Config Parser | service/src/parser | JSON configuration parsing |
| Buffer Manager | service/src/buffer-mgr | Frame buffer pool management |

---

## Project Structure

```
VisionFlow/
├── libraries/          # Independent, reusable modules
│   ├── vf_common/      # Error model and common types
│   ├── vf_logger/      # Logging system
│   ├── vf_file/        # File I/O
│   ├── vf_framebuffer/ # Frame buffer abstraction
│   ├── vf_conversion/  # Color space conversion
│   ├── vf_camera/      # Camera capture
│   ├── vf_encoder/     # Video encoding
│   └── vf_display/     # Display output
├── service/            # Pipeline engine and processing units
│   └── src/
│       ├── core/       # Pipeline manager, processing unit interface
│       ├── parser/     # JSON config parser
│       ├── buffer-mgr/ # Buffer pool management
│       ├── common/     # Service utilities
│       └── units/      # Processing units (camera, file, conversion...)
├── test_clients/       # Manual validation per module
├── vf_frames/          # Test frame assets (raw, rgb, yuv)
├── docs/               # Architecture decisions and API docs
├── formatter/          # Code style configuration
├── LICENSE
└── README.md
```

---

## Prerequisites

```bash
# Ubuntu 22.04+
sudo apt-get install build-essential
```

---

## Build

```bash
# Build everything
make all

# Build only libraries
make libraries

# Build only service
make service

# Build test clients
make test_clients

# Clean all artifacts
make clean
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
