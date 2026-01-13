#include "Decomposer.h"
#include <cstdint>
#include <cstring>

FloatData Decomposer::decompose(const double value) const
{
    // Stałe IEEE 754 dla binary64 (double):
    // bias = 1023 (do konwersji wykładnika z postaci "biased" (0..2047) na "unbiased" (np. -1022..+1023))
    constexpr int kBias = 1023;

    // Maska na 52-bitową część ułamkową (mantysę / fraction).
    // W formacie double:
    // - 1 bit znaku
    // - 11 bitów wykładnika (biased)
    // - 52 bity mantysy (fraction)
    constexpr uint64_t kFracMask = (1ULL << 52) - 1ULL;

    // Struktura wyniku
    FloatData data{};

    // Pobieramy surowe bity double bez zmiany bitów (reinterpret przez memcpy jest bezpieczne).
    // data.rawBits będzie zawierało dokładnie IEEE754 binary64.
    std::memcpy(&data.rawBits, &value, sizeof(double));

    // Rozbijamy rawBits na pola:
    // - signBit: najwyższy bit (bit 63)
    // - expBits: 11 bitów wykładnika (bity 62..52)
    // - frac: 52 bity mantysy (bity 51..0)
    const uint64_t bits = data.rawBits;
    const uint64_t signBit = (bits >> 63) & 1ULL;
    const uint64_t expBits = (bits >> 52) & 0x7FFULL; // 0x7FF = 11 jedynek
    const uint64_t frac = bits & kFracMask;

    // Zapisujemy znak jako bool (true = ujemna)
    data.sign = (signBit != 0);

    // Zapisujemy "mantissę" jako 52-bitową część fraction (bez ukrytego bitu!)
    // Dla liczb Normal ukryty bit (1.) nie jest zapisany w IEEE754 — dodaje się go dopiero przy obliczeniach.
    data.mantissa = frac;

    // Na podstawie expBits i frac rozpoznajemy typ liczby (wg IEEE754):
    //
    // expBits = 0:
    // - jeśli frac = 0 -> Zero (+0 lub -0 zależnie od znaku)
    // - jeśli frac !=0 -> Subnormal (denormal)
    //
    // expBits = 2047 (0x7FF):
    // - jeśli frac = 0 -> Inf (+inf / -inf)
    // - jeśli frac !=0 -> NaN
    //
    // w pozostałych przypadkach -> Normal

    if (expBits == 0 && frac == 0)
    {
        // Zero: w IEEE754 to expBits=0 i frac=0.
        // Znak może być +0 lub -0 (signBit).
        data.type = FloatData::Type::Zero;

        // Wykładnik w strukturze nie ma tu znaczenia, ale ustawiamy 0 dla porządku.
        data.exponent = 0;
    }
    else if (expBits == 0)
    {
        // Subnormal: expBits=0 i frac != 0.
        data.type = FloatData::Type::Subnormal;

        // Dla subnormalnych efektywny wykładnik (unbiased) jest stały: 1 - bias = -1022.
        // (To jest definicja IEEE754 dla binary64)
        data.exponent = static_cast<int16_t>(1 - kBias); // -1022
    }
    else if (expBits == 0x7FFULL && frac == 0)
    {
        // Inf: expBits=2047 i frac=0.
        data.type = FloatData::Type::Inf;

        // exponent w strukturze niepotrzebny (Inf nie ma sensownego wykładnika), ustawiamy 0.
        data.exponent = 0;
    }
    else if (expBits == 0x7FFULL)
    {
        // NaN: expBits=2047 i frac != 0.
        data.type = FloatData::Type::NaN;

        data.exponent = 0;
    }
    else
    {
        // Normal: expBits w zakresie 1..2046.
        data.type = FloatData::Type::Normal;

        // Konwersja wykładnika z biased -> unbiased:
        // unbiased = expBits - bias.
        // Np:
        //  - 1.0 ma expBits=1023 => exponent=0
        //  - 2.0 ma expBits=1024 => exponent=1
        data.exponent = static_cast<int16_t>(static_cast<int>(expBits) - kBias);
    }

    return data;
}

double Decomposer::compose(const FloatData &data) const
{
    constexpr int kBias = 1023;
    constexpr uint64_t kFracMask = (1ULL << 52) - 1ULL;

    // 1) Zaczynamy od przygotowania pól do złożenia 64-bitowej liczby:
    //    - signBit (1 bit)
    //    - expBits (11 bitów, biased)
    //    - frac (52 bity)
    uint64_t signBit = data.sign ? 1ULL : 0ULL;
    uint64_t expBits = 0;
    uint64_t frac = data.mantissa & kFracMask;

    // 2) Dobieramy expBits i frac w zależności od typu:
    if (data.type == FloatData::Type::Zero)
    {
        // Zero: expBits=0 i frac=0 (znak zostaje, więc może być +0/-0)
        expBits = 0;
        frac = 0;
    }
    else if (data.type == FloatData::Type::Subnormal)
    {
        // Subnormal: expBits=0, frac = mantysa (musi być !=0 aby to było subnormal)
        expBits = 0;
    }
    else if (data.type == FloatData::Type::Inf)
    {
        // Inf: expBits=2047, frac=0
        expBits = 0x7FFULL;
        frac = 0;
    }
    else if (data.type == FloatData::Type::NaN)
    {
        // NaN: expBits=2047, frac != 0
        expBits = 0x7FFULL;

        // Jeśli ktoś podał mantysę=0, wymuszamy minimalny payload,
        // bo inaczej wyszłoby Inf (expBits=2047 i frac=0).
        if (frac == 0)
            frac = 1;
    }
    else
    {
        // Normal:
        // exponent w strukturze jest unbiased, więc musimy dodać bias,
        // aby otrzymać expBits do zapisu w IEEE754.
        // expBits = exponent + bias
        expBits = static_cast<uint64_t>(static_cast<int>(data.exponent) + kBias);
    }

    // Składamy 64-bitowe rawBits:
    // sign na bicie 63, expBits na bitach 62..52, frac na 51..0
    uint64_t bits = (signBit << 63) | (expBits << 52) | frac;

    // Z błów (bits) robimy double — znowu memcpy, żeby zachować dokładny wzorzec bitowy.
    double value;
    std::memcpy(&value, &bits, sizeof(double));
    return value;
}