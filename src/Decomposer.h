#pragma once

#include "IDecomposer.h"

/**
 * Szkielet implementacji dla Osoby 2.
 */
class Decomposer final : public IDecomposer {
public:
    [[nodiscard]] FloatData decompose(double value) const override;

    [[nodiscard]] double compose(const FloatData &data) const override;
};
