#include "doctest.h"
#include "rng.h"

TEST_CASE("same seed yields same sequence") {
    ayyy::Rng a(527), b(527);
    for (int i = 0; i < 100; ++i) CHECK(a.next() == b.next());
}

TEST_CASE("different seeds diverge") {
    ayyy::Rng a(1), b(2);
    bool differ = false;
    for (int i = 0; i < 8; ++i) if (a.next() != b.next()) differ = true;
    CHECK(differ);
}

TEST_CASE("below(n) stays in range") {
    ayyy::Rng r(99);
    for (int i = 0; i < 1000; ++i) {
        int v = r.below(7);
        CHECK(v >= 0);
        CHECK(v < 7);
    }
}

TEST_CASE("nextFloat in [0,1)") {
    ayyy::Rng r(42);
    for (int i = 0; i < 1000; ++i) {
        float f = r.nextFloat();
        CHECK(f >= 0.0f);
        CHECK(f < 1.0f);
    }
}
