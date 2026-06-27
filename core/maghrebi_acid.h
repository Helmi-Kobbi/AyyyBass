#pragma once
#include "brain.h"

namespace ayyy {

class MaghrebiAcid : public IBrain {
public:
    Pattern generate(uint32_t seed, const GenParams& p) const override;
    Pattern mutate(const Pattern& in, uint32_t mutationSeed,
                   const GenParams& p) const override;
    const char* name() const override { return "maghrebi-acid"; }
};

} // namespace ayyy
