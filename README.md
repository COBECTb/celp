# CELP Audio Codec

A 4800 bps Code Excited Linear Prediction (CELP) audio codec implementation. This project provides a low-bitrate speech coding solution that compresses audio while maintaining intelligibility.

## Overview

This CELP (Code Excited Linear Prediction) codec is designed for very low bitrate speech coding at 1200 bps. It implements the core algorithms for encoding and decoding speech signals using linear predictive coding techniques.

### Key Features
- 4800 bps bitrate for extremely low bandwidth usage
- Frame-based processing (240 samples per frame)
- Efficient compression suitable for voice communication
- Cross-platform compatibility

## Build Instructions

### Prerequisites
- GCC or Clang compiler
- Make build system
- Standard C library

### Building the Project

```bash
# Build the library and executable
make all

# This creates:
# - libcelp/libcelp.a (static library)
# - celp (executable)
```

### Cleaning Build Artifacts

```bash
# Clean all build artifacts
make clean
```

## Usage

The executable supports three modes of operation:

### 1. Encoding

To encode a raw audio file to CELP format:

```bash
./celp encode <input_file> <output_file>
```

Example:
```bash
./celp encode input.raw output.celp
```

### 2. Decoding

To decode a CELP file back to raw audio:

```bash
./celp decode <input_file> <output_file>
```

Example:
```bash
./celp decode input.celp output.raw
```

### 3. Transcoding (Encode then Decode)

To process a file through both encoding and decoding:

```bash
./celp <input_file> <output_file>
```

Example:
```bash
./celp input.raw output.raw
```

## Input/Output Format

- **Input**: Raw 16-bit signed integer PCM audio at appropriate sampling rate
- **Output**: Compressed CELP data (144 bits per frame = 18 bytes per frame)
- **Frame size**: 240 samples per frame
- **Bitrate**: 1200 bps

## Technical Details

- Each audio frame contains 240 samples
- Each compressed frame is 144 bits (18 bytes)
- The codec processes frames sequentially
- The algorithm uses linear predictive coding with codebook-based excitation

## Library Interface

The codec can also be used as a library in other applications:

```c
#include "celp.h"

// Initialize the codec
celp_init(0);

// Encode a frame of 240 samples
short input[240];      // Input audio samples
char output[18];       // Output buffer (144/8 = 18 bytes)

int result = celp_encode(input, output);

// Decode a frame back to audio
short decoded[240];    // Decoded audio samples

int result = celp_decode(output, decoded);
```

## File Structure

```
.
├── README.md              # This file
├── celp.h                 # Public header file
├── main.c                 # Main executable source
├── makefile               # Build instructions
└── libcelp/              # Core codec implementation
    ├── Makefile          # Library build instructions
    ├── celp.c            # Main codec implementation
    ├── celp.h            # Internal header
    ├── *.h               # Various algorithm headers
    └── libcelp.a         # Compiled static library
```

## Error Codes

The codec functions return the following error codes:

- `CELP_OK (0)`: Operation completed successfully
- `CELP_TWOERR (1)`: Uncorrectable error in stream bits
- `CELP_LSPERR (2)`: Error encoding LSPs
- `CELP_ERR_DCODCBG (3)`: Error decoding codebook gain
- `CELP_ERR_DCODCBI (4)`: Error decoding codebook index
- `CELP_ERR_DELAY (5)`: Invalid pitch delay
- `CELP_ERR_GAINDECODE (6)`: Unquantised codebook gain
- `CELP_ERR_POLEFILT (7)`: Bad coefficients in pole filter
- `CELP_ERR_IMPULSE_LENGTH (8)`: Impulse response too long
- `CELP_ERR_MAXLP (9)`: MAXLP < MAXL in psearch.c
- `CELP_ERR_PITCH_TYPE (10)`: Bad pitch search type
- `CELP_ERR_DCODPG (11)`: Error decoding pitch gain

## License

This project appears to be based on work by Aleksandr Strokov (created July 30, 2017). See the source files for specific licensing information.
