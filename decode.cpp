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
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include "wav.hpp"
#include "neuralink.hpp"
#include "arithmetic_coding.hpp"



void decodeStream(std::istream &inputStream, std::ostream &outputStream) {

    // read the WAV header from the input stream
    auto header = read_wav_header(inputStream);

    // check the validity of the WAV header
    neuralink_check_wav_header(header);

    // write the WAV header to the output stream
    write_wav_header(outputStream, header);

    // Create an input bitstream
    IBitStream inputBitStream(inputStream);

    // main loop: process symbols from the input WAV stream
    ArithmeticDecoder<Model> decoder;
    Model::SymbolType symbol;
    decoder.init(inputBitStream);
    while(true) {
        symbol = decoder.decode(inputBitStream);
        decoder.model.update_state(symbol);

        // stop symbol
        if (symbol == Model::NUM_SYMBOLS-1)
            break;

        // write to output file
        neuralink_write_symbol_to_stream(outputStream, symbol);
    }
}


int main(int argc, char* argv[]) {

    if (argc == 3) {
        
        const std::string inputFilePath = argv[1];
        const std::string outputFilePath = argv[2];

        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error opening input file: " << inputFilePath << std::endl;
            return EXIT_FAILURE;
        }

        // Open the output file stream
        std::ofstream outputFile(outputFilePath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error opening output file: " << outputFilePath << std::endl;
            return EXIT_FAILURE;
        }

        // Process the streams
        decodeStream(inputFile, outputFile);

        // Close the file streams
        inputFile.close();
        outputFile.close();

    } else if (argc == 1) {
        // Process standard input and output streams
        decodeStream(std::cin, std::cout);

    } else {
        std::cerr << "Usage: " << argv[0] << " [inputFile outputFile]" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

