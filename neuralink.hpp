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

#ifndef NEURALINK_HPP
#define NEURALINK_HPP

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <array>


// Symbols are 10 bit unsigned integers [0..1023] stored in the lowest bits of an unsigend 16 bit integer.
typedef uint16_t SymbolType;
typedef uint16_t FrequencyType;


/**
 * @brief Convert a signed 16-bit value to an unsigned 10-bit value.
 *
 * This function maps a signed 16-bit value from the WAV file to an unsigned 10-bit value.
 * The input value is shifted right by 6 bits and then an offset of 512 is added,
 * resulting in a value in the range [0, 1023].
 *
 * @param x The signed 16-bit input value.
 * @return The unsigned 10-bit output value in the range [0, 1023].
 */
SymbolType neuralink_16bit_to_10bit(int16_t x) {
    return (x >> 6) + 512;
}


/**
 * @brief Convert an unsigned 10-bit value back to a signed 16-bit value.
 *
 * This function is the inverse of map_16_10. It recovers the original signed 16-bit value
 * from the unsigned 10-bit value, reconstructing the discarded bits, specific for the
 * Neuralink data. It's unclear why it is like this, why are the lower 6 bit set like this?
 *
 * @param u The unsigned 10-bit input value in the range [0, 1023].
 * @return The reconstructed signed 16-bit output value.
 * @note This transform has some magic numbers that were experimentally recovered from example data.
 */
int16_t neuralink_10bit_to_16bit(SymbolType u) {
    double temp = (u - 512 + 0.5) * (64.0 + 1009.0 / 16384.0) - 0.5;
    return static_cast<int16_t>(std::trunc(temp));
}


/**
 * @brief Reads a 16-bit raw sample from the input stream and converts it to a 10-bit symbol.
 * 
 * This function attempts to read a 16-bit raw sample from the provided input stream. If the read operation
 * is successful, it converts the 16-bit raw sample to a 10-bit symbol using the `neuralink_16bit_to_10bit` function
 * and stores the result in the `symbol` parameter.
 * 
 * @param inputStream The input stream from which the 16-bit raw sample is read.
 * @param symbol Reference to a uint16_t variable where the 10-bit converted symbol will be stored.
 * @return Returns `true` if the read operation was successful and the symbol was converted and stored.
 *         Returns `false` if the end of the stream is reached or if a read error occurs.
 */
bool neuralink_read_symbol_from_stream(std::istream &inputStream, SymbolType &symbol) {
    int16_t raw_sample;
    if (inputStream.read(reinterpret_cast<char*>(&raw_sample), sizeof(raw_sample))) {
        symbol = neuralink_16bit_to_10bit(raw_sample);
        return true;
    }
    return false;
}


/**
 * @brief Converts a 10-bit symbol to a 16-bit raw sample and writes it to the output stream.
 * 
 * This function converts a 10-bit symbol to a 16-bit raw sample using the `neuralink_10bit_to_16bit` function
 * and writes the resulting 16-bit raw sample to the provided output stream.
 * 
 * @param outputStream The output stream to which the 16-bit raw sample is written.
 * @param symbol The 10-bit symbol to be converted and written to the stream.
 * @return Returns a reference to the output stream after the write operation.
 */
std::ostream &neuralink_write_symbol_to_stream(std::ostream &outputStream, uint16_t &symbol) {
    int16_t raw_sample;
    raw_sample = neuralink_10bit_to_16bit(symbol);
    return outputStream.write(reinterpret_cast<char*>(&raw_sample), sizeof(raw_sample));
}


/**
 * @brief Checks the WAV file header to ensure it is a 16-bit mono format.
 * 
 * This function verifies that the provided WAV file header is of the correct size (44 bytes)
 * and that it represents a 16-bit mono WAV file. If the header does not meet these criteria,
 * the function throws a runtime error.
 * 
 * @param header A vector of bytes representing the WAV file header.
 * @throws std::runtime_error if the header size is not 44 bytes or if the WAV format is not 16-bit mono.
 */
void neuralink_check_wav_header(const std::vector<uint8_t> &header) {
    if (header.size() != 44) {
        throw std::runtime_error("Invalid WAV header size");
    }
    uint16_t numChannels = *reinterpret_cast<const uint16_t*>(&header[22]);
    uint16_t bitsPerSample = *reinterpret_cast<const uint16_t*>(&header[34]);

    // Verify that this is a 16-bit mono WAV file
    if (numChannels != 1 || bitsPerSample != 16) {
        throw std::runtime_error("Unsupported WAV format, we only support 16 bit mono WAV data");
    }
}


double normal_cdf(double x, double loc, double scale) {
    // Standardize the input
    double standardized_x = (x - loc) / scale;
    
    // Compute the CDF using the error function
    return 0.5 * (1.0 + std::erf(standardized_x / std::sqrt(2.0)));
}


class Model {
public:
    // type aliases
    using SymbolType = uint16_t;
    using FrequencyType = uint16_t;
    using IntType = uint32_t;

    // we have a 10 bits signal with 1024 possible symbols 0,1,...,1023. 
    // Additionally we add 1 extra "STOP" symbol with id 1024
    static constexpr SymbolType NUM_SYMBOLS = 1025; // 1024 for the 10bit signal + 1 extra stop symbol

    // 
    static constexpr FrequencyType MAX_FREQUENCY = 0x7FFF; // 2^15 - 1
    static constexpr IntType MAX_CODE = 0x1FFFF; // 2^17 - 1
    static constexpr IntType Int25 = 0x8000; // 2^17 * 1/4 = 2^15
    static constexpr IntType Int50 = 0x10000; // 2^17 * 1/2 = 2^16
    static constexpr IntType Int75 = 0x18000; // 2^17 * 3/4 = 2^16 + 2^15


    // Constructor
    Model()
    {
        omega =  ltv / (1 - alpha - beta);

        // fill the ccft
        for (int i=0; i<NUM_DIST; ++i) {

            double max_p = cdf(
                NUM_SYMBOLS,  
                511.0, 
                cdf_scale[i], 
                cdf_w[i], 
                static_cast<double>(cdf_z[i]) /  Model::NUM_SYMBOLS
            );

            for (int j=1; j < NUM_SYMBOLS; ++j) {
                double p = cdf(
                    j, 
                    511.0, 
                    cdf_scale[i], 
                    cdf_w[i],
                    static_cast<double>(cdf_z[i]) /  Model::NUM_SYMBOLS
                );
                ccft[i][j] = static_cast<FrequencyType>(p / max_p * (MAX_FREQUENCY - NUM_SYMBOLS)) + j;
            }          
            ccft[i][0] = 0;
            ccft[i][NUM_SYMBOLS] = MAX_FREQUENCY;
        }
    }

    void symbol_low_high(const SymbolType symbol, FrequencyType &low, FrequencyType &high) const {
        IntType loc = symbol;
        loc += NUM_SYMBOLS;
        loc += active_symbol_shift;
        loc = loc % NUM_SYMBOLS;

        low = ccft[active_dist][loc];
        high = ccft[active_dist][loc + 1];
    };

    SymbolType frequency_symbol(const FrequencyType freq) const {
        SymbolType loc;

        auto it = std::upper_bound(ccft[active_dist].begin(), ccft[active_dist].end(), freq);
        if (it == ccft[active_dist].begin()) {
            loc = 0;
        } else {
            loc = std::distance(ccft[active_dist].begin(), it) - 1;
        }

        SymbolType symbol = static_cast<SymbolType>(
            static_cast<uint32_t>(loc + NUM_SYMBOLS - active_symbol_shift) % NUM_SYMBOLS
        );

        return symbol;
    };



    void update_state(SymbolType symbol) {
        double ds = symbol - mean;

        // outlier filter
        if (std::abs(ds) > outlier_level * stdev) {
            outlier_counter++;
        } else {
            outlier_counter = 0;
        }
        if (outlier_counter > 3) {
            outlier_counter = 0;
        }

        // update mean & std
        if (outlier_counter == 0) {
            mean = ma * mean + (1-ma) * symbol;
            stdev = std::sqrt(omega + alpha * stdev * stdev + beta * ds * ds);

            active_dist = std::distance(
                std_levels.begin(),
                std::lower_bound(std_levels.begin(), std_levels.end(), stdev)
            );
            active_dist = std::min<uint16_t>(active_dist, Model::NUM_DIST - 1);

            //
            active_symbol_shift = 511 - static_cast<SymbolType>(mean + (symbol - mean)*mrr);

        }
    };

private: 
    // Number of conditional symbol distributions
    static const uint16_t NUM_DIST = 4; 

    // Conditional Cummulative Frequency Table (ccft)
    std::array<std::array<FrequencyType, Model::NUM_SYMBOLS + 1>, Model::NUM_DIST> ccft;

    // index of the current distribution
    uint16_t active_dist = 0;
    int16_t active_symbol_shift = 0;

    // Dynamic state model
    double mean  = 511;
    double stdev = 8.0;
    double ma = 0.20;
    double ltv = 7.5;
    double alpha = 0.725;
    double beta  = 0.175;
    double outlier_level = 8.4;
    double mrr = 0.05;
    double omega ;
    uint16_t outlier_counter = 0;

    // conditional distribution constants
    std::array<double, NUM_DIST> std_levels = {16, 18, 20, 22};
    std::array<double, NUM_DIST> cdf_scale = {5.145, 6.035, 8.547, 20.05};
    std::array<double, NUM_DIST> cdf_w = {2.5E-4, 2.5E-4,  2.5E-4, 2.5E-4};
    std::array<double, NUM_DIST> cdf_z = {106.3, 82.84, 62.87, 61.86} ;


    double cdf(double x, double loc, double scale, double w, double z) {
        double p = (1.0 - w - z) * normal_cdf(x, loc, scale) + w;
        if (x >= loc) {
            p += z;
        }
        return p;
    }    
};


#endif