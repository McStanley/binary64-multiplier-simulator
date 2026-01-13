#pragma once

#include "IMultiplier.h"

/**
 * Szkielet implementacji dla Osoby 3.
 */
class Multiplier final : public IMultiplier
{
public:
    [[nodiscard]] FloatData multiply(const FloatData &a, const FloatData &b) const override;
};
