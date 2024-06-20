# NeuroMasterBlaster

Welcome to the NeuroMasterBlaster repository! This project is part of the Neuralink Compression Challenge, aimed at developing a lossless, streaming, zero-delay data compression algorithm specifically designed for Neuralink data. Our implementation is a highly efficient, low-memory, online streaming algorithm coded in C++. For more information about the competition, please visit the [Neuralink Compression Challenge page](https://content.neuralink.com/compression-challenge/README.html).

## Overview

NeuroMasterBlaster is designed to compress neural data with minimal latency and high efficiency. The primary goal is to achieve lossless compression of streaming data in real-time, for applications where delay and data integrity are critical, like playing video games with pure thought.

## Features

- **Fast Execution**: The algorithm is optimized for speed, ensuring minimal processing time. It compressed and decompresses 1 hour of neural activity data in 17 seconds.
- **Online Streaming**: Capable of handling streaming data with zero delay. On the competition data the compression-decompression latency is 0.3 microseconds. 
- **Low Memory Footprint**: Designed to use minimal memory resources, making it suitable for deployment on devices with limited memory.
- **Efficient C++ Implementation**: The core logic is implemented in C++ for optimal performance and resource management.

## Repository Structure

The source code is organized into several key components, each responsible for specific aspects of the compression algorithm:

### `wav.hpp`

This module handles the input and output operations for WAV files. It provides functions to read and write WAV files, which are  used for the neural activity data storage in this competition. For more details on the WAV file format, please refer to the [WAV file format specification](https://en.wikipedia.org/wiki/WAV).

### `neuralink.hpp`

Contains Neuralink-specific implementations, including:
- **Signal Normalization**: Ensures that the neural signals are normalized for consistent processing. The 10 bit neuralink source data seems to be transformed to 16 bit, we have routines that revert this.
- **Dynamic Predictive Probability Distribution**: Implements a dynamic symbol probability model combining a dynamic GARCH noise model, an AR1 mean model, and a uniform prior to predict the next signal value distribution.

### `bitstream.hpp`

Includes routines for reading and writing streams of individual bits instead of bytes, which is needed in the data compression process.

### `arithmetic_coding.hpp`

Implements the arithmetic encoder algorithm. Arithmetic coding is a form of entropy encoding used in lossless data compression. For more information on arithmetic coding, please refer to the [Wikipedia page on Arithmetic Coding](https://en.wikipedia.org/wiki/Arithmetic_coding).

### `encoder.cpp` and `decoder.cpp`

These files serve as command-line wrappers that integrate all the components. They provide executables for encoding and decoding data streams using the NeuroMasterBlaster algorithm.

## Usage

To compile and run the encoder and decoder, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/sitmo/NeuroMasterBlaster.git
   cd NeuroMasterBlaster
   ```

2. Build the project using the provided Makefile::
   ```bash
   make
   ```
## Running the Encoder and Decoder on Competition Data

To run the encoder and decoder on the competition data, follow these steps:

1. **Download the Competition Data**:
   - Navigate to the [Neuralink Compression Challenge page](https://content.neuralink.com/compression-challenge/README.html).
   - Download `data.zip` and `eval.sh` from the website.


2. **Prepare the Environment**:
   Ensure you have the necessary scripts and data in the appropriate directories.

3. **Running on Linux**:
   The competition provides an `eval.sh` script specifically for Linux. To run the evaluation on a Linux system:
   ```bash
   chmod +x eval.sh
   ./eval.sh
   ```

4. **Running on macOS**:
   We have added an `eval_osx.sh` script in our repository to facilitate running the evaluation on macOS. To run the evaluation on macOS:
   ```bash
   chmod +x eval_osx.sh
   ./eval_osx.sh
   ```

The `eval.sh` and `eval_osx.sh` scripts are designed to automate the process of encoding and decoding the provided dataset using the NeuroMasterBlaster algorithm. These scripts will invoke the necessary commands to process the data and evaluate the performance of the compression algorithm.


## License

This project is licensed under the Apache License Version 2.0. See the `LICENSE` file for details.

## References

- [Neuralink Compression Challenge](https://content.neuralink.com/compression-challenge/README.html)
- [WAV file format specification](https://en.wikipedia.org/wiki/WAV)
- [Arithmetic Coding](https://en.wikipedia.org/wiki/Arithmetic_coding)
