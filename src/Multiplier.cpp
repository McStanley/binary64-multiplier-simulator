#include "Multiplier.h"

FloatData Multiplier::multiply(const FloatData &a, const FloatData &b) const {
    FloatData result{};
    // TODO (Osoba 3): Zaimplementować algorytm mnożenia mantys, dodawania wykładników i normalizacji
    result.rawBits = 0;
    result.sign = a.sign ^ b.sign;
    result.exponent = 0;
    result.mantissa = 0;
    result.type = FloatData::Type::Normal;
    return result;
}
