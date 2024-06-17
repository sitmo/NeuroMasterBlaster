/*
 * Copyright 2024 Thijs van den Berg, thijs@sitmo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#include <iostream>
#include <stdexcept>

class IBitStream {
public:
    explicit IBitStream(std::istream& input)
        : inputStream(input), inputBuffer(0), inputBufferPos(8) {}

    bool get(bool &bit) {
        if (inputBufferPos == 8) {
            if (!fillInputBuffer()) {
                return false; // End of stream or error
            }
        }
        bit = (inputBuffer >> (7 - inputBufferPos)) & 1;
        inputBufferPos++;
        return true;
    }

private:
    std::istream& inputStream;
    unsigned char inputBuffer;
    int inputBufferPos;

    bool fillInputBuffer() {
        inputBuffer = 0;
        if (inputStream.read(reinterpret_cast<char*>(&inputBuffer), 1)) {
            inputBufferPos = 0;
            return true;
        } else {
            return false;
        }
    }
};

class OBitStream {
public:
    explicit OBitStream(std::ostream& output)
        : outputStream(output), outputBuffer(0), outputBufferPos(0) {}

    ~OBitStream() {
        flush();
    }

    void put(bool bit) {
        outputBuffer = (outputBuffer << 1) | bit;
        outputBufferPos++;
        if (outputBufferPos == 8) {
            flush();
        }
    }

    void flush() {
        if (outputBufferPos > 0) {
            outputBuffer <<= (8 - outputBufferPos); // Align bits to the left
            outputStream.write(reinterpret_cast<char*>(&outputBuffer), 1);
            outputBuffer = 0;
            outputBufferPos = 0;
        }
    }
private:
    std::ostream& outputStream;
    unsigned char outputBuffer;
    int outputBufferPos;
};

#endif