#include "doctest.h"
#include "pattern.h"
#include "brain.h"
#include <string>

TEST_CASE("pattern defaults are safe") {
    ayyy::Pattern p;
    CHECK(p.length >= 1);
    CHECK(p.steps[0].velocity >= 1);
    CHECK(p.steps[0].active == false);
}

namespace {
struct OneNoteBrain : ayyy::IBrain {
    ayyy::Pattern generate(uint32_t seed, const ayyy::GenParams& p) const override {
        ayyy::Pattern pat; pat.seed = seed; pat.length = p.length; pat.ternary = p.ternary;
        pat.steps[0].active = true; pat.steps[0].note = p.root; pat.steps[0].accent = true;
        return pat;
    }
    ayyy::Pattern mutate(const ayyy::Pattern& in, uint32_t, const ayyy::GenParams&) const override {
        return in;
    }
    const char* name() const override { return "one-note"; }
};
}

TEST_CASE("a brain conforms to the slot") {
    OneNoteBrain b;
    ayyy::GenParams p; p.length = 7; p.root = 33;
    ayyy::Pattern pat = b.generate(527, p);
    CHECK(std::string(b.name()) == "one-note");
    CHECK(pat.length == 7);
    CHECK(pat.seed == 527);
    CHECK(pat.steps[0].note == 33);
}
