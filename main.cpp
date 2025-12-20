#include "Menu.h"
#include "Decomposer.h"
#include "Multiplier.h"
#include <memory>

int main() {
    auto decomposer = std::make_unique<Decomposer>();
    auto multiplier = std::make_unique<Multiplier>();

    Simulator simulator(std::move(decomposer), std::move(multiplier));

    Menu menu(simulator);
    menu.display();

    return 0;
}
