#pragma once

#include <cstdint>
#include <string>

/**
 * Struktura przechowująca części liczby binarnej zmiennoprzecinkowej 64-bitowej (IEEE 754).
 */
struct FloatData {
    uint64_t rawBits; // Pełna reprezentacja 64-bitowa
    bool sign; // Znak (0 - dodatnia, 1 - ujemna)
    int16_t exponent; // Wykładnik (po uwzględnieniu biasu)
    uint64_t mantissa; // Mantysa (ułamkowa część)

    // Typy specjalne liczb
    enum class Type {
        Normal,
        Subnormal,
        Zero
    } type;

    // Pomocnicza funkcja do wypisywania stanu
    [[nodiscard]] std::string toString() const;
};
