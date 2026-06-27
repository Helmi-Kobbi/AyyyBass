#pragma once
#include "pattern.h"
#include <cstdint>
#include <vector>

namespace ayyy {

constexpr int kPPQ = 480;
constexpr int kOverlap = 12; // legato overlap ticks for slides

struct MidiEvent {
    int     tick;    // absolute
    uint8_t status;  // 0x90 = note on, 0x80 = note off (channel 0)
    uint8_t data1;   // note
    uint8_t data2;   // velocity
};

inline int stepTicks(bool ternary) { return ternary ? 160 : 120; }

std::vector<MidiEvent> renderPattern(const Pattern& pat);

} // namespace ayyy
