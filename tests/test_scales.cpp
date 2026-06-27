#include "doctest.h"
#include "scales.h"

TEST_CASE("scales resolve by name") {
    int idx = ayyy::scaleIndexByName("hijaz");
    CHECK(idx >= 0);
    const ayyy::Scale& s = ayyy::scaleAt(idx);
    CHECK(s.count == 7);
    CHECK(s.degrees[0] == 0);
    CHECK(s.degrees[1] == 1);   // Hijaz: flat 2nd
    CHECK(s.degrees[2] == 4);   // major 3rd
}

TEST_CASE("pentatonic minor has five notes") {
    const ayyy::Scale& s = ayyy::scaleAt(ayyy::scaleIndexByName("pentatonic"));
    CHECK(s.count == 5);
}

TEST_CASE("unknown name falls back to 0") {
    CHECK(ayyy::scaleIndexByName("nope") == 0);
}

TEST_CASE("degreeToNote maps within range") {
    const ayyy::Scale& s = ayyy::scaleAt(ayyy::scaleIndexByName("pentatonic"));
    int n = ayyy::degreeToNote(s, /*root*/33, /*degreeStep*/0);
    CHECK(n == 33);
    int up = ayyy::degreeToNote(s, 33, s.count); // one octave up
    CHECK(up == 45);
}
