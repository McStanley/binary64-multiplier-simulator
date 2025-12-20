#pragma once

#include "Simulator.h"

/**
 * Klasa obsługująca interfejs konsolowy.
 */
class Menu {
public:
    explicit Menu(Simulator &simulator);

    void display() const;

private:
    Simulator &simulator;

    static void showAuthors();

    static void showInstructions();

    void runSimulation() const;

    static bool readDouble(double &value, const std::string &prompt);
};
