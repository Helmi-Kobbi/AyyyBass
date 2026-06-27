#include "doctest.h"
#include "render.h"
#include "maghrebi_acid.h"
#include <algorithm>

TEST_CASE("ternary step is 160 ticks, binary is 120") {
    CHECK(ayyy::stepTicks(true) == 160);
    CHECK(ayyy::stepTicks(false) == 120);
}

TEST_CASE("each active step yields a note-on and a note-off") {
    ayyy::Pattern pat;
    pat.length = 2; pat.ternary = false;
    pat.steps[0].active = true; pat.steps[0].note = 33; pat.steps[0].velocity = 110;
    pat.steps[1].active = true; pat.steps[1].note = 36; pat.steps[1].velocity = 78;
    auto ev = ayyy::renderPattern(pat);
    int ons = 0, offs = 0;
    for (auto& e : ev) { if ((e.status & 0xF0) == 0x90 && e.data2 > 0) ++ons; else ++offs; }
    CHECK(ons == 2);
    CHECK(offs == 2);
}

TEST_CASE("slide makes the note overlap the next onset (legato)") {
    ayyy::Pattern pat;
    pat.length = 2; pat.ternary = false;
    pat.steps[0].active = true; pat.steps[0].note = 33; pat.steps[0].slide = true;
    pat.steps[1].active = true; pat.steps[1].note = 35;
    auto ev = ayyy::renderPattern(pat);
    int offTime = -1, onTime = -1;
    for (auto& e : ev) {
        bool on = ((e.status & 0xF0) == 0x90 && e.data2 > 0);
        if (!on && e.data1 == 33) offTime = e.tick;
        if (on && e.data1 == 35) onTime = e.tick;
    }
    CHECK(onTime == 120);
    CHECK(offTime > onTime);
}

TEST_CASE("events are sorted by tick") {
    ayyy::MaghrebiAcid brain; ayyy::GenParams p; p.length = 7;
    auto ev = ayyy::renderPattern(brain.generate(527, p));
    for (size_t i = 1; i < ev.size(); ++i) CHECK(ev[i-1].tick <= ev[i].tick);
}
