#pragma once

#include "IDecomposer.h"
#include "IMultiplier.h"
#include <memory>

/**
 * Główna klasa zarządzająca symulacją.
 * Łączy pracę Osoby 2 (rozkład) i Osoby 3 (mnożenie).
 */
class Simulator {
public:
    Simulator(std::unique_ptr<IDecomposer> decomposer, std::unique_ptr<IMultiplier> multiplier);

    /**
     * Rozpoczyna proces mnożenia dwóch liczb krok po kroku.
     */
    void run(double a, double b) const;

private:
    std::unique_ptr<IDecomposer> decomposer;
    std::unique_ptr<IMultiplier> multiplier;
};
