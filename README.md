# cap2root - Cap'n Proto to ROOT Converter

A simple tool to convert Cap'n Proto (*.cap) files to ROOT format, following TDD and KISS principles.

## Prerequisites

### 1. ROOT
ROOT should already be installed and in your PATH.

### 2. Cap'n Proto
Install Cap'n Proto on macOS:

```bash
brew install capnproto
```

Or on Linux (Ubuntu/Debian):

```bash
sudo apt-get install capnproto libcapnp-dev
```

Or build from source:

```bash
# Download and build Cap'n Proto
curl -O https://capnproto.org/capnproto-c++-1.0.2.tar.gz
tar zxf capnproto-c++-1.0.2.tar.gz
cd capnproto-c++-1.0.2
./configure
make -j6 check
sudo make install
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Converting to ROOT format

Convert a Cap'n Proto file to ROOT format:

```bash
./cap2root input.cap output.root
```

Example:

```bash
./cap2root 152Eu_walk_000001.cap output.root
```

### Inspecting Cap'n Proto files

Use the `capdump` utility to inspect Cap'n Proto files and see detailed information:

```bash
# Basic summary
./capdump input.cap

# Verbose mode - show all events
./capdump input.cap -v

# Show only first N packets
./capdump input.cap -n 5

# Combine options
./capdump input.cap -v -n 1
```

Example output:
```
Dumping Cap'n Proto file: 152Eu_walk_000001.cap
============================================================

=== Packet 0 ===
Type: PlainEvent
Events: 131

 Index   Mod    Ch    Energy      Timestamp
--------------------------------------------
     0     4     0      2726       588022760
     1     4     0      2593      1131965584
     ...
```

Options:
- `-v, --verbose`: Show detailed information for all events in each packet
- `-n NUM`: Show only first NUM packets (default: all)
- `-h, --help`: Show help message

## Running Tests

```bash
cd build
make test
# or run directly
./test_converter
```

## Project Structure

```
.
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── TreeData.h               # ROOT tree data structure
├── eventProto.capnp         # Cap'n Proto schema
├── src/
│   ├── main.cpp            # Main converter program
│   ├── capdump.cpp         # Cap'n Proto dump utility
│   ├── CapnpReader.h       # Cap'n Proto file reader
│   ├── CapnpReader.cpp
│   ├── RootWriter.h        # ROOT file writer
│   └── RootWriter.cpp
└── tests/
    ├── test_main.cpp       # Test runner
    ├── test_reader.cpp     # Reader tests
    └── test_writer.cpp     # Writer tests
```

## Utilities

The project includes two main utilities:

1. **cap2root** - Converts Cap'n Proto files to ROOT format
2. **capdump** - Inspects and displays Cap'n Proto file contents

## Supported Event Types

The converter supports all event types defined in eventProto.capnp:

- **PlainEvent** - Basic events (board, channel, energy, timestamp)
- **PsdEvent** - Events with PSD (Pulse Shape Discrimination)
- **WaveEvent** - Events with single waveform
- **DualWaveEvent** - Events with two waveforms
- **RawTimeEvent** - Events with fine timestamp
- **CrossEvent** - Events with trigger flags (good/lost trigger)
- **PsdWaveEvent** - Events with PSD and waveform
- **FullEvent** - Complete events with PSD and dual waveforms

## Output Format

The output ROOT file contains a TTree named "tree" with the following branches:

- Mod (UChar_t) - Board/Module number
- Ch (UChar_t) - Channel number
- TimeStamp (UInt64_t) - Timestamp
- FineTS (Double_t) - Fine timestamp or PSD value
- ChargeLong (UShort_t) - Energy
- ChargeShort (UShort_t) - Short charge or scaled PSD
- Extras (UInt_t) - Extra information
- RecordLength (UInt_t) - Waveform length
- Trace1 (vector<UShort_t>) - First waveform
- Trace2 (vector<UShort_t>) - Second waveform
- DTrace1 (vector<UChar_t>) - Digital trace 1
- DTrace2 (vector<UChar_t>) - Digital trace 2

## Design Principles

This project follows:

- **TDD (Test-Driven Development)**: Tests written before implementation
- **KISS (Keep It Simple, Stupid)**: Minimal complexity, straightforward code
- Clean architecture with separated concerns (Reader, Writer, Main)
