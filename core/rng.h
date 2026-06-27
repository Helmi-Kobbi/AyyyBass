#pragma once
#include <cstdint>

namespace ayyy {

class Rng {
public:
    explicit Rng(uint32_t seed) : state_(seed ? seed : 0x9E3779B9u) {}

    uint32_t next() {            // xorshift32
        uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        return x;
    }

    float nextFloat() {          // [0,1)
        return (next() >> 8) * (1.0f / 16777216.0f);
    }

    int below(int n) {           // [0,n)
        return n > 0 ? static_cast<int>(next() % static_cast<uint32_t>(n)) : 0;
    }

    bool chance(float p) { return nextFloat() < p; }

private:
    uint32_t state_;
};

} // namespace ayyy
