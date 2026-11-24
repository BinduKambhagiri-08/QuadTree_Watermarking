# QuadTree-Based Invisible Image Watermark Embedding Using Divide and Conquer

[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![DAA Category](https://img.shields.io/badge/Category-DAA%20Mini--Project-purple.svg)]()
[![Build](https://img.shields.io/badge/Build-CMake%20%7C%20MinGW-green.svg)]()
[![Zero Dependencies](https://img.shields.io/badge/Dependencies-Self--Contained-orange.svg)]()

> A full-stack academic demonstration prototype implementing **Divide and Conquer** image decomposition, spatial variance texture analysis, and invisible LSB digital ownership watermarking in **C++17**.

---

## 1. Project Overview

Digital watermarking is an essential technique for copyright protection, ownership authentication, and digital rights management. Naive spatial watermarking modifies pixels arbitrarily, which introduces noticeable visual artifacts in smooth areas.

This project implements an intelligent, academic-grade solution using **Divide and Conquer**:
1. Recursively subdivides a 2D image matrix into quadrants using a **QuadTree**.
2. Evaluates local statistical variance $\sigma^2$ at leaf blocks to locate high-frequency textures.
3. Exploits **Human Visual System (HVS) spatial masking** by embedding ownership bits strictly into high-variance textured regions.
4. Uses **Least Significant Bit (LSB)** substitution on deterministic anchor pixels.
5. Blindly extracts and verifies digital signatures directly from the saved PNG pixel buffer.
6. Analyzes algorithmic time and space complexity under **Master Theorem Case 1**.

---

## 2. Technology Stack

- **Core Algorithm Engine:** C++17
- **Computer Vision:** OpenCV 4.x compatible architecture with built-in zero-friction image engine fallback
- **HTTP Server:** Embedded lightweight `cpp-httplib`
- **JSON Serialization:** `nlohmann/json`
- **Build System:** CMake 3.15+
- **Compiler:** GCC 16.2.0 (MinGW-w64 UCRT) / Clang / MSVC
- **Frontend Dashboard:** Vanilla HTML5, Modern CSS3, ES6+ JavaScript, Fetch API, HTML5 Canvas & SVG

---

## 3. Project Directory Structure

```text
QuadTree_Watermarking/
│
├── CMakeLists.txt              # Cross-platform CMake build configuration
├── README.md                   # Complete project documentation and run guide
├── run_server.bat              # One-click Windows launch script
├── run_server.sh               # One-click Linux/macOS launch script
│
├── include/
│   ├── ApiServer.h             # Embedded HTTP REST API endpoints and handlers
│   ├── ImageProcessor.h        # Ingestion, power-of-two normalization, base64
│   ├── QuadTree.h              # Recursive QuadTree Divide & Conquer data structure
│   ├── Watermark.h             # 64-bit header, LSB embedding, extraction, verification
│   ├── Metrics.h               # MSE, PSNR, modified pixels, std::chrono telemetry
│   └── cv_compat.h             # OpenCV 4.x abstraction with zero-friction fallback
│
├── src/
│   ├── main.cpp                # Application entry point and server startup
│   ├── ApiServer.cpp           # REST API routing, CORS, and request dispatching
│   ├── ImageProcessor.cpp      # Bilinear resizing, grayscale conversion, PNG export
│   ├── QuadTree.cpp            # Recursive 4-quadrant divide and variance calculation
│   ├── Watermark.cpp           # LSB bitstream manipulation and anchor encoding
│   ├── Metrics.cpp             # Quality analysis and benchmark reporting
│   └── stb_impl.cpp            # Self-contained image codec implementation
│
├── third_party/
│   ├── httplib.h               # Header-only C++ HTTP server
│   ├── json.hpp                # Header-only JSON parser
│   ├── stb_image.h             # Image loader
│   └── stb_image_write.h       # Lossless PNG image writer
│
├── frontend/
│   ├── index.html              # Academic dark navy dashboard UI
│   ├── style.css               # Clean styling, responsive layout, animations
│   └── script.js               # Frontend controller, Canvas/SVG charts, API calls
│
├── input/
│   └── sample_image.png        # Offline synthetic academic benchmark
│
├── output/
│   ├── original.png            # Normalized input image
│   ├── quadtree_visualization.png # Blue subdivision boundaries
│   ├── selected_regions.png    # Green textured vs red smooth regions
│   ├── watermarked_image.png   # Output watermarked PNG with intact LSBs
│   ├── difference_image.png    # Magnified difference map (|I - Iw| * 255)
│   ├── watermark_manifest.txt  # Coordinate manifest and embedding metadata
│   ├── extraction_report.txt   # Live extraction report
│   └── performance_report.txt  # DAA benchmark telemetry and PSNR analysis
│
├── docs/
│   ├── project_explanation.md  # Beginner-friendly system walkthrough
│   ├── algorithm.md            # Detailed algorithm pseudocode and formulas
│   ├── complexity_analysis.md  # Master Theorem Case 1 proof & recurrences
│   └── viva_questions.md       # 28 project-specific viva questions & answers
│
└── tests/
    ├── test_quadtree.cpp       # Unit tests for leaf counts, depth, and variance
    ├── test_watermark.cpp      # Unit tests for bitpacking, embedding, and recovery
    └── test_metrics.cpp        # Unit tests for MSE, PSNR, and difference counters
```

---

## 4. Compilation & Running Instructions

### Prerequisites
- A modern C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+).
- CMake 3.15 or newer.

### Quick Start (Windows)
Double-click `run_server.bat` or run:
```cmd
run_server.bat
```
This script configures CMake, compiles the C++ executable, launches the server on port 8080, and opens your default browser automatically.

### Manual Build Instructions (Windows / Linux / macOS)

1. Open your terminal in the `QuadTree_Watermarking` root directory:
```bash
# Create build directory
mkdir build
cd build

# Configure build with CMake
cmake ..

# Compile the server and unit tests
cmake --build .
```

2. Run the automated C++ unit tests:
```bash
ctest --output-on-failure
# Or run executables directly:
./test_quadtree
./test_watermark
./test_metrics
```

3. Launch the C++ backend server:
```bash
./QuadTreeServer --port 8080
```

4. Open your browser and navigate to:
```text
http://localhost:8080
```

---

## 5. Algorithmic Complexity

- **Recurrence Relation:** $T(n) = 4T(n/2) + o(n^2)$
- **Master Theorem Case:** **Case 1** ($a = 4, b = 2, n^{\log_b a} = n^2$)
- **Time Complexity:** $\Theta(n^2)$ for an $n \times n$ canvas with fixed leaf block size.
- **Space Complexity:** $O(\log n)$ call stack depth + $O((n/S)^2)$ leaf block memory.

---

## 6. Project Features & Verification

- **One-Click Demonstration:** The header contains a "Run Complete Demonstration" button that executes the entire pipeline in under 15 milliseconds.
- **Interactive Canvas:** Hover or click on any block in the QuadTree visualization to inspect its $(X, Y)$ coordinates, size, variance, and selection status.
- **No Mockups:** Every number, image, histogram, and metric originates from the actual C++ executable via HTTP REST API.
- **Lossless PNG Persistence:** Watermarks survive saving to disk and reloading, verifying genuine blind extraction.
