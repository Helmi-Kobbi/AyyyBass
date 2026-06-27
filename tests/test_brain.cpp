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

#include "maghrebi_acid.h"
#include "scales.h"

TEST_CASE("maghrebi acid is deterministic") {
    ayyy::MaghrebiAcid brain;
    ayyy::GenParams p; p.length = 7; p.scaleIndex = ayyy::scaleIndexByName("hijaz");
    ayyy::Pattern a = brain.generate(527, p);
    ayyy::Pattern b = brain.generate(527, p);
    for (int i = 0; i < ayyy::kMaxSteps; ++i) {
        CHECK(a.steps[i].active == b.steps[i].active);
        CHECK(a.steps[i].note   == b.steps[i].note);
        CHECK(a.steps[i].accent == b.steps[i].accent);
        CHECK(a.steps[i].slide  == b.steps[i].slide);
    }
}

TEST_CASE("generate respects length and never returns garbage") {
    ayyy::MaghrebiAcid brain;
    ayyy::GenParams p; p.scaleIndex = ayyy::scaleIndexByName("pentatonic");
    for (uint32_t seed = 1; seed <= 200; ++seed) {
        for (int len : {3, 5, 7, 12, 16}) {
            p.length = len;
            ayyy::Pattern pat = brain.generate(seed, p);
            CHECK(pat.length == len);
            int notes = 0, accents = 0, firstNote = -1; bool twoDistinct = false;
            for (int i = 0; i < len; ++i) {
                if (pat.steps[i].active) {
                    ++notes;
                    if (firstNote < 0) firstNote = pat.steps[i].note;
                    else if (pat.steps[i].note != firstNote) twoDistinct = true;
                }
                if (pat.steps[i].accent) ++accents;
            }
            CHECK(notes >= 2);
            CHECK(accents >= 1);
            CHECK(twoDistinct);
        }
    }
}

TEST_CASE("notes stay inside the requested octave span") {
    ayyy::MaghrebiAcid brain;
    ayyy::GenParams p; p.root = 33; p.octaveSpan = 1; p.length = 16;
    ayyy::Pattern pat = brain.generate(7, p);
    for (int i = 0; i < pat.length; ++i)
        if (pat.steps[i].active) {
            CHECK(pat.steps[i].note >= 33);
            CHECK(pat.steps[i].note <= 33 + 12);
        }
}

static int stepDiffs(const ayyy::Pattern& a, const ayyy::Pattern& b) {
    int d = 0;
    for (int i = 0; i < a.length; ++i) {
        const auto& x = a.steps[i]; const auto& y = b.steps[i];
        if (x.active != y.active || x.note != y.note ||
            x.accent != y.accent || x.slide != y.slide) ++d;
    }
    return d;
}

TEST_CASE("mutate keeps the vibe: small, deterministic change") {
    ayyy::MaghrebiAcid brain;
    ayyy::GenParams p; p.length = 12; p.scaleIndex = ayyy::scaleIndexByName("hijaz");
    ayyy::Pattern base = brain.generate(527, p);
    ayyy::Pattern m1 = brain.mutate(base, 1, p);
    ayyy::Pattern m2 = brain.mutate(base, 1, p);
    CHECK(stepDiffs(base, m1) >= 1);
    CHECK(stepDiffs(base, m1) <= 2);
    CHECK(stepDiffs(m1, m2) == 0);
    CHECK(m1.length == base.length);
}
