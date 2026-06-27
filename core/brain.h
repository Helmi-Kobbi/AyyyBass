#pragma once
#include "pattern.h"
#include <cstdint>

namespace ayyy {

// The shared "language" between body and brain. Kept a touch more general
// than acid strictly needs, because it is the long-lived contract every
// future brain (dub, detroit, ...) will speak. Changing it breaks all brains.
struct GenParams {
    uint8_t root        = 33;    // MIDI note (A1)
    int     scaleIndex  = 0;     // index into scaleTable()
    int     length      = 7;     // free cycle length (1..kMaxSteps)
    bool    ternary     = true;  // triplet / 6-8 feel
    float   density     = 0.5f;  // 0..1 sparse..busy
    int     octaveSpan  = 1;     // how many octaves the line may roam
};

class IBrain {
public:
    virtual ~IBrain() = default;
    virtual Pattern generate(uint32_t seed, const GenParams& p) const = 0;
    virtual Pattern mutate(const Pattern& in, uint32_t mutationSeed,
                           const GenParams& p) const = 0;
    virtual const char* name() const = 0;
};

} // namespace ayyy
