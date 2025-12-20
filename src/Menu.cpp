#include "Menu.h"
#include <iostream>
#include <limits>

Menu::Menu(Simulator &simulator) : simulator(simulator) {
}

void Menu::display() const {
    int choice = 0;
    while (true) {
        std::cout << "\n=== SYMULATOR MNOZENIA BINARNEGO 64-BIT ===\n";
        std::cout << "1. Uruchom symulacje mnozenia\n";
        std::cout << "2. Instrukcje\n";
        std::cout << "3. Autorzy\n";
        std::cout << "4. Zakoncz\n";
        std::cout << "Wybor: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Niepoprawny wybor. Sprobuj ponownie.\n";
            continue;
        }

        switch (choice) {
            case 1:
                runSimulation();
                break;
            case 2:
                showInstructions();
                break;
            case 3:
                showAuthors();
                break;
            case 4:
                std::cout << "Zamykanie programu...\n";
                return;
            default:
                std::cout << "Nie ma takiej opcji.\n";
        }
    }
}

void Menu::runSimulation() const {
    double a, b;
    const bool successA = readDouble(a, "Podaj pierwsza liczbe (double): ");
    if (!successA) return;

    const bool successB = readDouble(b, "Podaj druga liczbe (double): ");
    if (!successB) return;

    simulator.run(a, b);
}

bool Menu::readDouble(double &value, const std::string &prompt) {
    std::cout << prompt;
    if (!(std::cin >> value)) {
        std::cout << "Blad wczytywania liczby.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;
}

void Menu::showInstructions() {
    std::cout << "\n--- INSTRUKCJE ---\n";
    std::cout << "Program symuluje proces mnozenia dwoch liczb zmiennoprzecinkowych\n";
    std::cout << "zgodnie ze standardem IEEE 754 (64-bit / double).\n";
    std::cout << "Proces obejmuje:\n";
    std::cout << "1. Rozklad liczby na znak, wykladnik i mantyse.\n";
    std::cout << "2. Obsluge przypadkow specjalnych (Zero, NaN).\n";
    std::cout << "3. Mnozenie mantys i dodawanie wykladnikow.\n";
    std::cout << "4. Normalizacje i zaokraglanie wyniku.\n";
    std::cout << "Wiecej informacji o standardzie IEEE 754: https://pl.wikipedia.org/wiki/IEEE_754\n";
}

void Menu::showAuthors() {
    std::cout << "\n--- AUTORZY PROJEKTU (EZP8) ---\n";
    std::cout << "1. Stanislaw Olejniczak - Projektant struktur\n";
    std::cout << "2. Natalia Sagan - Specjalista od rozkladu liczb\n";
    std::cout << "3. Sebastian Ostrowski - Specjalista od mnozenia\n";
    std::cout << "4. Barbara Nowogrodzka - Tester i debuger (Kierownik)\n";
    std::cout << "5. Daniel Szmelter - Tester i debuger\n";
    std::cout << "6. Dominika Pakiela - Dokumentalista\n";
}
