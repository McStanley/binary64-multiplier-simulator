#pragma once

#include "FloatData.h"

/**
 * Interfejs definiujący kontrakt na rozkładanie liczby 64-bitowej na składowe IEEE 754.
 */
class IDecomposer {
public:
    virtual ~IDecomposer() = default;

    /**
     * Rozkłada liczbę double na strukturę FloatData.
     * @param value Liczba wejściowa.
     * @return Dane o strukturze liczby (znak, wykładnik, mantysa, typ).
     */
    [[nodiscard]] virtual FloatData decompose(double value) const = 0;

    /**
     * Składa strukturę FloatData z powrotem do liczby double.
     * @param data Dane o strukturze liczby.
     * @return Wynikowa liczba double.
     */
    [[nodiscard]] virtual double compose(const FloatData &data) const = 0;
};
