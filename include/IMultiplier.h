#pragma once

#include "FloatData.h"

/**
 * Interfejs definiujący kontrakt na operację mnożenia dwóch struktur FloatData.
 */
class IMultiplier {
public:
    virtual ~IMultiplier() = default;

    /**
     * Mnoży dwie liczby w formacie FloatData.
     * @param a Pierwszy czynnik.
     * @param b Drugi czynnik.
     * @return Wynik mnożenia (znormalizowany i zaokrąglony).
     */
    [[nodiscard]] virtual FloatData multiply(const FloatData &a, const FloatData &b) const = 0;
};
