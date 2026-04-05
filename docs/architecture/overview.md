# VisionFlow — Architecture Overview

**Version:** 0.1 — Initial  
**Status:** Living document — updated as modules are implemented

---

## What is VisionFlow?

VisionFlow is a modular, pipeline-based image processing service written in C. It receives frames from input sources (camera, file, synthetic), processes them through a configurable chain of processing units (color conversion, scaling, encoding), and delivers results to output sinks (display, file, shared memory).

The architecture is inspired by embedded multimedia systems used in the automotive industry — specifically the pattern of separating reusable library components from a service layer that orchestrates them via a pipeline engine.

---

## Design Principles

**1. Modular libraries — independent from the service**
Each module under `libraries/` compiles and tests independently. No library knows about the pipeline engine or other libraries unless explicitly linked.

**2. Common processing unit interface**
Every unit in the pipeline — regardless of whether it is an input source, a processing stage, or an output sink — implements the same interface:
```c
vf_error_t vf_unit_init(vf_unit_ctx_t *ctx, const vf_config_t *cfg);
vf_error_t vf_unit_process(vf_unit_ctx_t *ctx, vf_framebuffer_t *frame);
vf_error_t vf_unit_deinit(vf_unit_ctx_t *ctx);
```
The pipeline engine calls these functions without knowing what the unit does internally.

**3. Zero-copy frame passing**
Frames are passed between units by pointer — no pixel data is copied in the hot path. The buffer manager owns the memory; units borrow it temporarily during processing.

**4. Config-driven pipeline**
The pipeline topology — which units run, in what order, with what parameters — is defined in a JSON configuration file. No recompilation is needed to change pipeline behavior.

**5. Single responsibility per module**
Each library and each unit has one clearly defined responsibility. A unit that handles both encoding and display is a design error.

---

## System Layers

```
┌─────────────────────────────────────────┐
│              test_clients/              │  ← manual validation per module
└─────────────────────────────────────────┘
                     │
┌─────────────────────────────────────────┐
│               vf_service                │  ← single service process
│                                         │
│  ┌─────────────┐   ┌─────────────────┐  │
│  │   Pipeline  │   │  Buffer Manager │  │
│  │   Engine    │   │  (frame pool)   │  │
│  └──────┬──────┘   └─────────────────┘  │
│         │                               │
│  ┌──────▼──────────────────────────┐    │
│  │         Processing Units        │    │
│  │  camera → conversion → encoder │    │
│  │  file   → conversion → display │    │
│  └─────────────────────────────────┘    │
│                                         │
│  ┌─────────────┐   ┌─────────────────┐  │
│  │   Config    │   │     Logger      │  │
│  │   Parser    │   │                 │  │
│  └─────────────┘   └─────────────────┘  │
└─────────────────────────────────────────┘
                     │
┌─────────────────────────────────────────┐
│               libraries/                │  ← independent, reusable
│  vf_common  vf_logger  vf_file          │
│  vf_framebuffer  vf_conversion          │
│  vf_camera  vf_encoder  vf_display      │
└─────────────────────────────────────────┘
```

---

## Data Flow

```
[Input Source]
      │
      │  vf_framebuffer_t* (zero-copy)
      ▼
[Pipeline Engine]
      │
      ├──▶ [Unit 1: conversion]  ─── vf_conversion_process()
      │
      ├──▶ [Unit 2: encoder]     ─── vf_encoder_process()
      │
      └──▶ [Unit 3: file sink]   ─── vf_file_write()
```

Each unit receives a pointer to the same frame buffer — no copying occurs between stages unless explicitly required by the processing algorithm.

---

## MVP Pipeline

The first demonstrable end-to-end pipeline:

```
file_unit → conversion_unit → file_unit
```

- `file_unit` reads a raw RGB frame from disk
- `conversion_unit` converts RGB → YUV using `vf_conversion`
- `file_unit` writes the YUV frame to disk

This validates the entire pipeline architecture without requiring camera hardware or a display.

---

## Module Dependencies

```
vf_service
    ├── vf_common       (error types — no dependencies)
    ├── vf_logger       (depends on vf_common)
    ├── vf_file         (depends on vf_common)
    ├── vf_framebuffer  (depends on vf_common)
    ├── vf_conversion   (depends on vf_common, vf_framebuffer)
    ├── vf_camera       (depends on vf_common, vf_framebuffer)
    ├── vf_encoder      (depends on vf_common, vf_framebuffer)
    └── vf_display      (depends on vf_common, vf_framebuffer)
```

`vf_common` has no dependencies — it is always built first.

---

## Architecture Decisions

Significant architectural decisions are documented as ADRs (Architecture Decision Records) in this directory. Each ADR captures: the context, the decision made, the alternatives considered, and the consequences.

ADRs are added as decisions are made during development — not all upfront.

| ADR | Decision | Status |
|-----|----------|--------|
| ADR-001 | C as primary language | Planned |
| ADR-002 | Modular service architecture | Planned |
| ADR-003 | Zero-copy frame passing | Planned |
| ADR-004 | JSON configuration | Planned |

---

*This document is updated as the architecture evolves. Last updated: Sprint 1.*
