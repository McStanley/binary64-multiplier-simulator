#include "Simulator.h"
#include <iostream>

Simulator::Simulator(std::unique_ptr<IDecomposer> decomposer, std::unique_ptr<IMultiplier> multiplier)
    : decomposer(std::move(decomposer)), multiplier(std::move(multiplier)) {
}

void Simulator::run(const double a, const double b) const {
    std::cout << "\n--- ROZPOCZECIE SYMULACJI MNOZENIA ---\n";
    std::cout << "Liczba A: " << a << "\n";
    std::cout << "Liczba B: " << b << "\n";

    // 1. Rozkład liczb
    const FloatData dataA = decomposer->decompose(a);
    const FloatData dataB = decomposer->decompose(b);

    std::cout << "[Krok 1] Rozklad liczb:\n";
    std::cout << "  A: " << dataA.toString() << "\n";
    std::cout << "  B: " << dataB.toString() << "\n";

    // 2. Mnożenie
    std::cout << "[Krok 2] Mnozenie mantys i korekcja wykladnikow...\n";
    const FloatData resultData = multiplier->multiply(dataA, dataB);
    std::cout << "  Wynik (dane): " << resultData.toString() << "\n";

    // 3. Złożenie wyniku
    const double result = decomposer->compose(resultData);
    std::cout << "[Krok 3] Wynik koncowy: " << result << "\n";

    // 4. Weryfikacja
    std::cout << "[Info] Oczekiwany wynik koncowy: " << (a * b) << "\n";
    std::cout << "--- KONIEC SYMULACJI ---\n";
}
