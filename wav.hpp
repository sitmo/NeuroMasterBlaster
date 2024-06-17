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
#ifndef WAV_HPP
#define WAV_HPP

#include <vector>
#include <cstdint>
#include <iostream>

/**
 * @brief Reads the header of a 16-bit mono WAV file from an input stream.
 *
 * This function reads the first 44 bytes of a WAV file from the provided input stream
 * and returns them as a vector of uint8_t. 
 *
 * @param inStream The input stream from which to read the WAV header.
 * @return A vector containing the 44-byte WAV header.
 */
std::vector<uint8_t> read_wav_header(std::istream& inputStream) {
    std::vector<uint8_t> header;

    // Read the header
    header.resize(44);
    inputStream.read(reinterpret_cast<char*>(header.data()), header.size());

    return header;
}

/**
 * @brief Writes a WAV header to an output stream.
 *
 * This function writes the given WAV header to the provided output stream.
 * It assumes that the header vector contains the correct 44-byte WAV header.
 *
 * @param outStream The output stream to which the WAV header will be written.
 * @param header The vector containing the 44-byte WAV header.
 * @throws std::runtime_error If the header size is not 44 bytes.
 */
std::ostream &write_wav_header(std::ostream& outputStream, const std::vector<uint8_t>& header) {
    // Verify that the header size is correct
    if (header.size() != 44) {
        throw std::runtime_error("Invalid WAV header size");
    }

    // Write the header
    return outputStream.write(reinterpret_cast<const char*>(header.data()), header.size());
}

#endif