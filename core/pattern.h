#pragma once
#include <cstdint>
#include <array>

namespace ayyy {

constexpr int kMaxSteps = 32;

struct Step {
    bool    active   = false; // false = rest
    uint8_t note     = 0;     // MIDI note number
    uint8_t velocity = 80;    // 1..127
    bool    accent   = false;
    bool    slide    = false; // slide/legato into the next active step
};

struct Pattern {
    std::array<Step, kMaxSteps> steps{};
    int      length  = 16;    // active cycle length, 1..kMaxSteps
    bool     ternary = false; // triplet / 6-8 feel
    uint32_t seed    = 0;     // the seed that produced this pattern
};

} // namespace ayyy
