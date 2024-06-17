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
#ifndef ARITHMETIC_CODING_HPP
#define ARITHMETIC_CODING_HPP

#include <cstdlib>
#include <array>
#include <algorithm>
#include <cassert>
#include "bitstream.hpp"


void forward_range(uint32_t &low, uint32_t &high, uint32_t symbol_low, uint32_t symbol_high) {
    uint32_t range = high - low + 1;
    high = low + range * symbol_high / 0x7FFF - 1;
    low = low + range * symbol_low / 0x7FFF;
}


uint32_t backward_value(uint32_t value, uint32_t low, uint32_t high) {
    uint32_t range = high - low + 1;
    return ((value - low + 1) * 0x7FFF - 1) / range;
}

/**
 * @brief Template class for Arithmetic Encoding.
 * 
 * @tparam T The model type providing frequency and symbol information.
 */
template <typename T>
class ArithmeticEncoder {

public:
    T model;
    size_t bits_written;    ///< Number of bits written.
    size_t symbols_written; ///< Number of symbols written.

    /**
     * @brief Default constructor.
     */
    ArithmeticEncoder() : bits_written(0), symbols_written(0), low(0), high(T::MAX_CODE), pending_bits(0) {}


    /**
     * @brief Encodes a symbol and writes bits to the output stream.
     * 
     * @param symbol The symbol to encode.
     * @param bit_stream The output bit stream to write to.
     */
    void encode(const SymbolType symbol, OBitStream &bit_stream) {
        symbols_written++;
        
        // get the [low, high) cumulative frequency range of a symbol
        model.symbol_low_high(symbol, symbol_low, symbol_high);

        // rescale low and high
        forward_range(low, high, symbol_low, symbol_high);

        // write bits to output
        while (true) {
            if (high < T::Int50) {
                // Output a '0' bit since the range is in the lower half
                write_bits(0, bit_stream);
            } else if (low >= T::Int50) {
                // Output a '1' bit since the range is in the upper half
                write_bits(1, bit_stream);
            } else if (low >= T::Int25 && high < T::Int75) {
                // Adjust the range when it straddles the midpoint
                pending_bits++;
                low -= T::Int25;
                high -= T::Int25;
            } else {
                // Break the loop if no more bits can be output unambiguously
                break;
            }

            // Shift the range left by one bit and expand the high range
            low <<= 1;
            low &= T::MAX_CODE;

            high = (high << 1) | 1;
            high &= T::MAX_CODE;
        }
    }


    /**
     * @brief Flushes the remaining bits to the output stream.
     * 
     * @param bit_stream The output bit stream to write to.
     */
    void flush(OBitStream &bit_stream) {
        pending_bits++;
        if (low < T::Int25) {
            write_bits(0, bit_stream);
        } else {
            write_bits(1, bit_stream);
        }
    };

private:
    using SymbolType = typename T::SymbolType;
    using FrequencyType = typename T::FrequencyType;
    using IntType = typename T::IntType;

    FrequencyType symbol_low, symbol_high;
    IntType low, high;
    size_t pending_bits;

    /**
     * @brief Writes bits to the output stream.
     * 
     * @param b The bit to write.
     * @param bit_stream The output bit stream to write to.
     */
   void write_bits(const bool b, OBitStream &bit_stream) {
        bits_written += 1 + pending_bits;
        bit_stream.put(b);
        for (int i=0; i<pending_bits; ++i)
            bit_stream.put(!b);
        pending_bits = 0;
    }
};



template <typename T>
class ArithmeticDecoder {

public:
    size_t bits_read = 0;
    size_t symbols_read = 0;
    T model;


    ArithmeticDecoder() : bits_read(0), symbols_read(0), low(0), high(T::MAX_CODE), value(0), pending_bits(0) {}


    SymbolType decode(IBitStream &bit_stream) {
        bool b;
        SymbolType symbol;

        // Lookup the symbol based in the frequency
        uint32_t scaled_value = backward_value(value, low,  high);

        symbol = model.frequency_symbol(scaled_value);

        // get the [low, high) cumulative frequency range of a symbol
        model.symbol_low_high(symbol, symbol_low, symbol_high);

        // rescale low and high
        forward_range(low, high, symbol_low, symbol_high);

        while (true) {
            if (high < T::Int50) {
                // pass
            } else if (low >= T::Int50) {
                value -= T::Int50;
                low -= T::Int50;
                high -= T::Int50;
            } else if ((low >= T::Int25) && (high < T::Int75)) {
                value -= T::Int25;
                low -= T::Int25;
                high -= T::Int25;
            } else {
                break;
            }
            low <<= 1;
            high = (high << 1) | 1;

            bit_stream.get(b);
            value = (value << 1) | b;
        }

        return symbol;
    }

    void init(IBitStream &bit_stream) {
        bool b;
        value = 0;
        for (int i=0; i<17; ++i) {
            bit_stream.get(b);
            value = (value << 1) | b;
        }
    }

private:
    using SymbolType = typename T::SymbolType;
    using FrequencyType = typename T::FrequencyType;
    using IntType = typename T::IntType;

    FrequencyType symbol_low, symbol_high;
    IntType low, high, value;
    size_t pending_bits;
};

#endif