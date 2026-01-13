#include "Multiplier.h"
#include <cstdint>

FloatData Multiplier::multiply(const FloatData &a, const FloatData &b) const
{
    // Stałe IEEE 754 dla binary64 (double)
    // bias = 1023, maksymalny wykładnik w polu expBits (11 bitów) to 2047 (0x7FF)
    constexpr int kBias = 1023;
    constexpr int kExpMax = 2047;

    // Maski na część ułamkową (mantysa) i "ukryty bit" (hidden bit) dla liczb normalnych.
    // Mantysa w double ma 52 bity, ale znacząca (significand) ma 53 bity (1.xxx),
    // gdzie ten "1" jest ukryty i występuje tylko dla Normal.
    constexpr uint64_t kFracMask = (1ULL << 52) - 1ULL; // 52 bity jedynek
    constexpr uint64_t kHiddenBit = 1ULL << 52;         // bit 52 (licząc od 0) - "1." dla normalnych

    // Funkcja pomocnicza: składa surowe bity double z:
    // sign (1 bit), expBits (11 bitów, już z biasem), frac (52 bity).
    auto packBits = [](uint64_t sign, uint64_t expBits, uint64_t frac) -> uint64_t
    {
        return (sign << 63) | (expBits << 52) | (frac & ((1ULL << 52) - 1ULL));
    };

    // Przygotowanie wyniku
    FloatData result{};

    // Znak wyniku w IEEE754: XOR znaków
    result.sign = a.sign ^ b.sign;

    // Obsługa przypadków specjalnych
    // Jeśli którykolwiek argument jest NaN -> wynik NaN (zwykle propagacja NaN)
    if (a.type == FloatData::Type::NaN)
        return a;
    if (b.type == FloatData::Type::NaN)
        return b;

    // Flagi ułatwiające logikę
    const bool aInf = (a.type == FloatData::Type::Inf);
    const bool bInf = (b.type == FloatData::Type::Inf);
    const bool aZero = (a.type == FloatData::Type::Zero);
    const bool bZero = (b.type == FloatData::Type::Zero);

    // Inf * 0 => NaN (nieoznaczoność)
    if ((aInf && bZero) || (bInf && aZero))
    {
        result.type = FloatData::Type::NaN;
        result.exponent = 0; // w tej strukturze exponent trzymamy "do wyświetlania", tu bez znaczenia
        result.mantissa = 1; // payload (żeby nie było to przypadkiem Inf)
        result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0x7FFULL, result.mantissa);
        return result;
    }

    // Inf * (cokolwiek niezerowego skończonego) => Inf
    if (aInf || bInf)
    {
        result.type = FloatData::Type::Inf;
        result.exponent = 0;
        result.mantissa = 0;
        result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0x7FFULL, 0);
        return result;
    }

    // 0 * x => 0 (z zachowaniem znaku: XOR znaków daje +0 lub -0)
    if (aZero || bZero)
    {
        result.type = FloatData::Type::Zero;
        result.exponent = 0;
        result.mantissa = 0;
        result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0, 0);
        return result;
    }

    //  Rozpakowanie do postaci roboczej (significand + exponent)
    auto unpackSigExp = [&](const FloatData &x, uint64_t &sig /*53*/, int &e /*unbiased*/)
    {
        if (x.type == FloatData::Type::Subnormal)
        {
            // Dla subnormalnych wykładnik jest bardzo mały, zwykle -1022.
            // U Ciebie Decomposer ustawia exponent na -1022 (1 - bias).
            e = static_cast<int>(x.exponent);

            // Subnormalne NIE mają ukrytego bitu, więc sig = sama mantysa (52 bitów max)
            sig = x.mantissa;

            // Żeby później mnożenie/normalizacja działały jednolicie,
            // próbujemy "znormalizować" subnormalną: przesuwamy sig w lewo,
            // aż pojawi się hidden bit na pozycji 52 (jeśli mantysa != 0).
            // Każde przesunięcie w lewo oznacza, że faktyczny wykładnik maleje o 1.
            while (sig != 0 && (sig & kHiddenBit) == 0)
            {
                sig <<= 1;
                e -= 1;
            }
        }
        else
        {
            // Normal:
            // exponent już jest unbiased (np. dla 2.0 -> 1)
            e = static_cast<int>(x.exponent);

            // Dodajemy ukryty bit, aby mieć pełne 53 bity znaczącej:
            sig = kHiddenBit | (x.mantissa & kFracMask);
        }
    };

    // Rozpakuj oba czynniki do (sig, exp)
    uint64_t sigA = 0, sigB = 0;
    int expA = 0, expB = 0;
    unpackSigExp(a, sigA, expA);
    unpackSigExp(b, sigB, expB);

    // Mnożenie znaczących i dodanie wykładników
    // Używamy __uint128_t, żeby to pomieścić bez utraty informacji.
    __uint128_t prod = static_cast<__uint128_t>(sigA) * static_cast<__uint128_t>(sigB);

    // Wykładniki się dodają (nadal unbiased)
    int exp = expA + expB; // nadal unbiased

    // Normalizacja iloczynu
    // Sprawdzamy bit 105:
    const __uint128_t bit105 = (static_cast<__uint128_t>(1) << 105);

    // Jeśli bit105 = 1, to wynik jest "za duży" (>=2),
    // więc przesuwamy mocniej (o 53) i zwiększamy wykładnik o 1.
    // Jeśli bit105 = 0, przesuwamy o 52, aby zostawić 53-bitową znaczącą.
    int shift = (prod & bit105) ? 53 : 52;
    if (shift == 53)
        exp += 1;

    // Wyciągamy "górne" 53 bity jako nową znaczącą (zawiera hidden bit)
    uint64_t sig = static_cast<uint64_t>(prod >> shift); // top 53 bity

    // Zaokrąglanie (round to nearest, ties to even)
    __uint128_t remMask = (static_cast<__uint128_t>(1) << shift) - 1;
    __uint128_t rem = prod & remMask;

    bool guard = (rem >> (shift - 1)) & 1;
    __uint128_t lowMask = (static_cast<__uint128_t>(1) << (shift - 1)) - 1;
    bool sticky = (rem & lowMask) != 0;

    // Decyzja o zaokrągleniu w górę
    if (guard && (sticky || (sig & 1ULL)))
    {
        sig += 1ULL;
        // Możliwa sytuacja: po dodaniu 1 dostajemy przeniesienie,
        // np. 1.111... + 1 -> 10.000...
        // Wtedy znów normalizujemy: przesuwamy i zwiększamy wykładnik.
        if (sig == (1ULL << 53))
        {
            sig >>= 1;
            exp += 1;
        }
    }

    // Spakowanie wyniku do formatu IEEE754 (rawBits)
    // Pole expBits w rawBits ma bias, więc dodajemy bias dopiero tutaj.
    int expBits = exp + kBias;

    // Overflow: wykładnik za duży -> Inf
    if (expBits >= kExpMax)
    {
        result.type = FloatData::Type::Inf;
        result.exponent = 0;
        result.mantissa = 0;
        result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0x7FFULL, 0);
        return result;
    }

    // Underflow: wykładnik za mały -> Subnormal lub Zero
    if (expBits <= 0)
    {
        // Chcemy zejść do expBits = 0 (subnormal). To oznacza przesunięcie znaczącej w prawo.
        int k = 1 - expBits; // o ile bitów przesunąć w prawo

        // Jeśli trzeba przesunąć "za dużo", to wynik jest 0
        if (k >= 64)
        {
            result.type = FloatData::Type::Zero;
            result.exponent = 0;
            result.mantissa = 0;
            result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0, 0);
            return result;
        }

        uint64_t sub = sig;

        // Rounding podczas zejścia do subnormal:
        // analogicznie używamy guard/sticky dla przesunięcia o k.
        uint64_t shiftedOutMask = (k >= 64) ? ~0ULL : ((1ULL << k) - 1ULL);
        uint64_t shiftedOut = sub & shiftedOutMask;
        bool g = (k > 0) ? ((sub >> (k - 1)) & 1ULL) : false;
        bool st = (k > 1) ? ((shiftedOut & ((1ULL << (k - 1)) - 1ULL)) != 0) : false;

        sub >>= k;
        if (g && (st || (sub & 1ULL)))
            sub += 1ULL;

        // Mantysa w subnormal to po prostu 52 bity (bez hidden bit)
        result.mantissa = sub & kFracMask;

        // Jeśli po przesunięciu i zaokrągleniu wyszło 0 -> Zero
        if (result.mantissa == 0)
        {
            // Subnormal: expBits = 0 w rawBits, a exponent do wyświetlania ustawiamy na -1022
            result.type = FloatData::Type::Zero;
            result.exponent = 0;
            result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0, 0);
        }
        else
        {
            result.type = FloatData::Type::Subnormal;
            result.exponent = static_cast<int16_t>(1 - kBias); // -1022
            result.rawBits = packBits(result.sign ? 1ULL : 0ULL, 0, result.mantissa);
        }
        return result;
    }

    // Normalny wynik:
    // - typ Normal
    // - exponent w strukturze trzymamy unbiased (do wyświetlania)
    // - w rawBits zapisujemy expBits (biased)
    result.type = FloatData::Type::Normal;
    result.exponent = static_cast<int16_t>(exp); // unbiased do logów/wyświetlania
    result.mantissa = sig & kFracMask;           // odcinamy hidden bit (zostaje 52-bitowa część ułamkowa)
    result.rawBits = packBits(result.sign ? 1ULL : 0ULL, static_cast<uint64_t>(expBits), result.mantissa);
    return result;
}