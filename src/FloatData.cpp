#include "FloatData.h"
#include <sstream>
#include <iomanip>

std::string FloatData::toString() const {
    std::stringstream ss;
    ss << "Bity: 0x" << std::hex << std::setw(16) << std::setfill('0') << rawBits << std::dec
            << " [Znak: " << (sign ? "1 (-)" : "0 (+)")
            << ", Wykladnik: " << exponent
            << ", Mantysa: 0x" << std::hex << mantissa << std::dec << "]";
    return ss.str();
}
