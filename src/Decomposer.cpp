#include "Decomposer.h"
#include <cstring>

FloatData Decomposer::decompose(const double value) const {
    FloatData data{};
    // TODO (Osoba 2): Zaimplementować rozkład liczby na części
    std::memcpy(&data.rawBits, &value, sizeof(double));
    data.sign = (data.rawBits >> 63) != 0;
    data.exponent = 0;
    data.mantissa = 0;
    data.type = FloatData::Type::Normal;
    return data;
}

double Decomposer::compose(const FloatData &data) const {
    double value;
    // TODO (Osoba 2): Zaimplementować składanie liczby z powrotem
    std::memcpy(&value, &data.rawBits, sizeof(double));
    return value;
}
