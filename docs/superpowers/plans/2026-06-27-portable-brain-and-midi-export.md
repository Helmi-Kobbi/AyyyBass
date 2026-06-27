# AyyyBass — Portable Brain + MIDI Export Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the portable, I/O-free "Maghrebi acid" generation brain and a CLI that writes its patterns to a Standard MIDI File, so a deterministic acid bassline can be auditioned on a real TD-3 / in a DAW within days.

**Architecture:** A pure C++17 `core/` library (the *brain* — data model, seeded RNG, scales, generation + mutation) with zero I/O and zero dependencies, so it later compiles unchanged to wasm and ESP32. A separate `platform/midi/` layer (a *body*) renders a `Pattern` to MIDI events (ternary timing, slide as legato overlap, accent as velocity) and serializes a Standard MIDI File. An `apps/cli/` thin wrapper parses arguments and writes the file. Generation is seed-deterministic: the same seed + params always yields the same pattern (so a pattern has a stable number, and so everything is unit-testable with golden values).

**Tech Stack:** C++17 · a tiny Makefile (`make test`) using the system `clang++`/`c++` — cmake is intentionally avoided to keep zero setup · [doctest](https://github.com/doctest/doctest) (single-header unit tests) · hand-rolled SMF writer (no MIDI libraries — keeps it portable to embedded).

> **Build note (toolchain reality):** cmake is not installed; we use a wildcard Makefile instead. Wherever a task below says `cmake -S . -B build`, `cmake --build build`, or `ctest --test-dir build`, the real command is **`make test`** (compiles everything and runs the unit tests). New `.cpp` files are picked up automatically.

---

## Project management

**This is Plan 1 of 3.** Each plan ships working, testable software on its own and reuses the same `core/` brain.

| Plan | Deliverable | Body |
|------|-------------|------|
| **1 (this)** | Portable brain + MIDI export CLI | MIDI file |
| 2 | Browser playground — press the yellow smiley, hear it | wasm + Web Audio |
| 3 | ESP32-S3 hardware | firmware (button, OLED, encoder, MIDI out) |

**Milestones in this plan:**
- **M1 — Foundation:** repo builds, test harness runs (Tasks 1–2).
- **M2 — Brain:** deterministic Maghrebi-acid generation + mutation, with invariants tested (Tasks 3–7).
- **M3 — Sound out:** Pattern renders to a correct MIDI file via the CLI (Tasks 8–10).
- **M4 (separate effort) — "It feels good":** tune the brain *by ear* against Helmi's reference grooves (guembri lines, Gnawa/Stambeli/Mezoued, Sidi Mansour-type tracks). Not codeable in advance — explicitly iterative.

**Acceptance criteria for Plan 1 (definition of done):**
- `ctest` passes all unit tests.
- `ayyybass generate --seed 527 --scale hijaz --length 7 --density 0.55 --bpm 130 -o out.mid` produces a `.mid` that loops as a ternary, slide-laden acid bassline on a TD-3 / in Ableton.
- Same arguments always produce a byte-identical `.mid` (determinism).
- `core/` has no `#include` of any I/O, platform, or MIDI header.

**Risks & mitigations:**
- *Musical quality of the v0 brain is unknown* → that's expected; M4 handles it by ear. Plan 1 only guarantees a *valid, structured, deterministic* line, not a magic one.
- *MIDI slide behavior is synth-dependent* → we render slide as note overlap (legato), the most widely-compatible trigger for 303-style portamento; CC-based slide is deferred.
- *TD-3 accent over MIDI* → mapped to velocity (accent = high velocity) in v1; a CC/accent-channel option is deferred.

## File structure

```
AyyyBass/
├── CMakeLists.txt                      # build: core lib, midi lib, cli, tests
├── third_party/doctest/doctest.h       # vendored test header (fetched in Task 1)
├── core/                               # THE BRAIN — portable, no I/O, no deps
│   ├── pattern.h                       # Step, Pattern
│   ├── rng.h                           # deterministic seeded PRNG
│   ├── scales.h                        # scale tables (pentatonic, hijaz, ...)
│   ├── brain.h                         # IBrain interface + GenParams (the "slot")
│   ├── maghrebi_acid.h
│   └── maghrebi_acid.cpp               # the v0 brain
├── platform/midi/                      # A BODY — render + serialize MIDI
│   ├── render.h
│   ├── render.cpp                      # Pattern -> timed MIDI events
│   ├── smf_writer.h
│   └── smf_writer.cpp                  # events -> Standard MIDI File bytes
├── apps/cli/
│   └── main.cpp                        # arg parsing -> generate -> write .mid
└── tests/
    ├── test_main.cpp                   # doctest entry
    ├── test_rng.cpp
    ├── test_scales.cpp
    ├── test_brain.cpp
    ├── test_render.cpp
    └── test_smf.cpp
```

Boundary rule enforced by review: nothing under `core/` may include anything from `platform/`, `apps/`, or the standard I/O / file headers. The brain computes; bodies do I/O.

---

### Task 1: Build skeleton + test harness

**Files:**
- Create: `CMakeLists.txt`
- Create: `third_party/doctest/doctest.h` (fetched)
- Create: `tests/test_main.cpp`

- [ ] **Step 1: Fetch the doctest single header**

Run:
```bash
mkdir -p third_party/doctest
curl -L -o third_party/doctest/doctest.h \
  https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h
```
Expected: a ~7000-line header at that path.

- [ ] **Step 2: Write the CMake build**

Create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.16)
project(ayyybass CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ayyy_core
  core/maghrebi_acid.cpp)
target_include_directories(ayyy_core PUBLIC core)

add_library(ayyy_midi
  platform/midi/render.cpp
  platform/midi/smf_writer.cpp)
target_include_directories(ayyy_midi PUBLIC platform/midi)
target_link_libraries(ayyy_midi PUBLIC ayyy_core)

add_executable(ayyybass apps/cli/main.cpp)
target_link_libraries(ayyybass PRIVATE ayyy_core ayyy_midi)

enable_testing()
add_executable(ayyy_tests
  tests/test_main.cpp
  tests/test_rng.cpp
  tests/test_scales.cpp
  tests/test_brain.cpp
  tests/test_render.cpp
  tests/test_smf.cpp)
target_include_directories(ayyy_tests PRIVATE third_party/doctest)
target_link_libraries(ayyy_tests PRIVATE ayyy_core ayyy_midi)
add_test(NAME unit COMMAND ayyy_tests)
```

- [ ] **Step 3: Write the doctest entry point and a smoke test**

Create `tests/test_main.cpp`:
```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("harness runs") {
    CHECK(1 + 1 == 2);
}
```

Create empty stub files so CMake links (each will be filled by later tasks):
`tests/test_rng.cpp`, `tests/test_scales.cpp`, `tests/test_brain.cpp`, `tests/test_render.cpp`, `tests/test_smf.cpp` each containing only:
```cpp
#include "doctest.h"
```
And create the source stubs `core/maghrebi_acid.cpp`, `platform/midi/render.cpp`, `platform/midi/smf_writer.cpp`, `apps/cli/main.cpp` each with `int _ayyy_stub_##__LINE__ = 0;`-style placeholder — actually simplest: a single `namespace { int stub = 0; }` line in each .cpp, and `int main(){return 0;}` in `apps/cli/main.cpp`.

- [ ] **Step 4: Configure, build, run**

Run:
```bash
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
```
Expected: configures, compiles, `1/1 Test #1: unit ... Passed`.

- [ ] **Step 5: Commit**
```bash
git add CMakeLists.txt third_party tests core platform apps
git commit -m "build: cmake skeleton + doctest harness"
```

---

### Task 2: Pattern data model

**Files:**
- Create: `core/pattern.h`
- Test: `tests/test_brain.cpp` (a struct-shape check)

- [ ] **Step 1: Write a test that uses the model**

In `tests/test_brain.cpp`:
```cpp
#include "doctest.h"
#include "pattern.h"

TEST_CASE("pattern defaults are safe") {
    ayyy::Pattern p;
    CHECK(p.length >= 1);
    CHECK(p.steps[0].velocity >= 1);
    CHECK(p.steps[0].active == false);
}
```

- [ ] **Step 2: Run to verify it fails (compile error: no pattern.h)**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `pattern.h` not found.

- [ ] **Step 3: Implement the model**

Create `core/pattern.h`:
```cpp
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
```

- [ ] **Step 4: Build + run**

Run: `cmake --build build && ctest --test-dir build`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add core/pattern.h tests/test_brain.cpp
git commit -m "feat(core): pattern data model"
```

---

### Task 3: Deterministic RNG

**Files:**
- Create: `core/rng.h`
- Test: `tests/test_rng.cpp`

- [ ] **Step 1: Write failing tests**

`tests/test_rng.cpp`:
```cpp
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
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `rng.h` not found.

- [ ] **Step 3: Implement**

Create `core/rng.h`:
```cpp
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
```

- [ ] **Step 4: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add core/rng.h tests/test_rng.cpp
git commit -m "feat(core): deterministic xorshift32 rng"
```

---

### Task 4: Scale tables

**Files:**
- Create: `core/scales.h`
- Test: `tests/test_scales.cpp`

- [ ] **Step 1: Write failing tests**

`tests/test_scales.cpp`:
```cpp
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
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `scales.h` not found.

- [ ] **Step 3: Implement**

Create `core/scales.h`:
```cpp
#pragma once
#include <array>
#include <cstring>

namespace ayyy {

struct Scale {
    const char* name;
    std::array<int, 12> degrees; // semitone offsets from root
    int count;
};

inline const std::array<Scale, 4>& scaleTable() {
    static const std::array<Scale, 4> kScales = {{
        {"pentatonic", {0, 3, 5, 7, 10, 0,0,0,0,0,0,0}, 5},
        {"hijaz",      {0, 1, 4, 5, 7, 8, 10, 0,0,0,0,0}, 7},
        {"kurd",       {0, 1, 3, 5, 7, 8, 10, 0,0,0,0,0}, 7},
        {"nahawand",   {0, 2, 3, 5, 7, 8, 10, 0,0,0,0,0}, 7},
    }};
    return kScales;
}

inline int scaleCount() { return static_cast<int>(scaleTable().size()); }

inline const Scale& scaleAt(int i) {
    const auto& t = scaleTable();
    if (i < 0 || i >= static_cast<int>(t.size())) i = 0;
    return t[i];
}

inline int scaleIndexByName(const char* name) {
    const auto& t = scaleTable();
    for (int i = 0; i < static_cast<int>(t.size()); ++i)
        if (std::strcmp(t[i].name, name) == 0) return i;
    return 0;
}

// degreeStep can exceed scale size: wraps and adds octaves.
inline int degreeToNote(const Scale& s, int root, int degreeStep) {
    int oct = degreeStep / s.count;
    int idx = degreeStep % s.count;
    if (idx < 0) { idx += s.count; oct -= 1; }
    return root + oct * 12 + s.degrees[idx];
}

} // namespace ayyy
```

- [ ] **Step 4: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add core/scales.h tests/test_scales.cpp
git commit -m "feat(core): maghrebi-friendly 12-TET scale tables"
```

---

### Task 5: The brain interface (the "slot")

**Files:**
- Create: `core/brain.h`
- Test: append to `tests/test_brain.cpp`

- [ ] **Step 1: Write a failing test against the interface via a tiny fake**

Append to `tests/test_brain.cpp`:
```cpp
#include "brain.h"

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
```
(Add `#include <string>` at the top of the file.)

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `brain.h` not found.

- [ ] **Step 3: Implement the contract**

Create `core/brain.h`:
```cpp
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
```

- [ ] **Step 4: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add core/brain.h tests/test_brain.cpp
git commit -m "feat(core): IBrain contract + GenParams (the swappable slot)"
```

---

### Task 6: The Maghrebi-acid brain — generate()

**Files:**
- Create: `core/maghrebi_acid.h`
- Modify: `core/maghrebi_acid.cpp` (replace stub)
- Test: append to `tests/test_brain.cpp`

The v0 algorithm (guembri-rooted ostinato; tuned by ear later in M4):
1. Seed the RNG.
2. For each of `length` steps, it is a note with probability tied to `density`; the first step is always a note (a strong "one").
3. Note steps choose a scale degree by a weighted walk that favors the root and fifth (drone/ostinato character) and small steps — kept inside `octaveSpan`.
4. Accent the first step; accent later steps with a density-scaled probability.
5. Mark a slide when a note moves stepwise into the next note (the acid/Maghrebi glissando).
6. Garbage rejection: guarantee at least 2 notes, at least 1 accent, and at least 2 distinct pitches; if violated, deterministically inject them.

- [ ] **Step 1: Write failing tests (invariants + determinism)**

Append to `tests/test_brain.cpp`:
```cpp
#include "maghrebi_acid.h"

TEST_CASE("maghrebi acid is deterministic") {
    ayyy::MaghrebiAcid brain;
    ayyy::GenParams p; p.length = 7; p.scaleIndex = ayyy::scaleIndexByName("hijaz");
    ayyy::Pattern a = brain.generate(527, p);
    ayyy::Pattern b = brain.generate(527, p);
    for (int i = 0; i < ayyy::kMaxSteps; ++i) {
        CHECK(a.steps[i].active   == b.steps[i].active);
        CHECK(a.steps[i].note     == b.steps[i].note);
        CHECK(a.steps[i].accent   == b.steps[i].accent);
        CHECK(a.steps[i].slide    == b.steps[i].slide);
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
            int notes = 0, accents = 0;
            int firstNote = -1; bool twoDistinct = false;
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
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `maghrebi_acid.h` not found.

- [ ] **Step 3: Implement the header**

Create `core/maghrebi_acid.h`:
```cpp
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
```

- [ ] **Step 4: Implement generate() in the .cpp**

Replace `core/maghrebi_acid.cpp` with:
```cpp
#include "maghrebi_acid.h"
#include "rng.h"
#include "scales.h"
#include <algorithm>

namespace ayyy {

Pattern MaghrebiAcid::generate(uint32_t seed, const GenParams& p) const {
    Pattern pat;
    pat.seed = seed;
    pat.length = std::max(1, std::min(p.length, kMaxSteps));
    pat.ternary = p.ternary;

    Rng rng(seed);
    const Scale& scale = scaleAt(p.scaleIndex);
    const int span = std::max(1, p.octaveSpan);
    const int maxDegree = scale.count * span;

    // Degree 0 = root. The ostinato gravitates to root (0) and fifth.
    int fifth = 0;
    for (int d = 0; d < scale.count; ++d)
        if (scale.degrees[d] == 7) { fifth = d; break; }

    int degree = 0; // start on the root
    for (int i = 0; i < pat.length; ++i) {
        Step s;
        bool firstStep = (i == 0);
        s.active = firstStep || rng.chance(0.25f + 0.7f * p.density);
        if (s.active) {
            if (!firstStep) {
                float r = rng.nextFloat();
                if (r < 0.45f)        degree = 0;          // home to root
                else if (r < 0.65f)   degree = fifth;      // the fifth
                else if (r < 0.85f)   degree += (rng.chance(0.5f) ? 1 : -1); // step
                else                  degree += (rng.chance(0.5f) ? 2 : -2); // small leap
                degree = std::max(0, std::min(degree, maxDegree - 1));
            } else {
                degree = 0;
            }
            s.note = static_cast<uint8_t>(degreeToNote(scale, p.root, degree));
            s.accent = firstStep || rng.chance(0.15f + 0.35f * p.density);
            s.velocity = s.accent ? 110 : 78;
        }
        pat.steps[i] = s;
    }

    // Slides: a note that moves stepwise into the next active note glides.
    for (int i = 0; i < pat.length; ++i) {
        if (!pat.steps[i].active) continue;
        int j = i + 1;
        while (j < pat.length && !pat.steps[j].active) ++j;
        if (j < pat.length) {
            int interval = std::abs(int(pat.steps[j].note) - int(pat.steps[i].note));
            if (interval > 0 && interval <= 2 && rng.chance(0.5f))
                pat.steps[i].slide = true;
        }
    }

    // Garbage rejection (deterministic): >=2 notes, >=1 accent, >=2 distinct pitches.
    int notes = 0, accents = 0, firstNote = -1; bool distinct = false;
    for (int i = 0; i < pat.length; ++i) {
        if (pat.steps[i].active) {
            ++notes;
            if (firstNote < 0) firstNote = pat.steps[i].note;
            else if (pat.steps[i].note != firstNote) distinct = true;
        }
        if (pat.steps[i].accent) ++accents;
    }
    if (notes < 2) {
        int at = pat.length / 2;
        pat.steps[at].active = true;
        pat.steps[at].note = static_cast<uint8_t>(degreeToNote(scale, p.root, fifth));
        pat.steps[at].velocity = 78;
    }
    if (!distinct) {
        for (int i = 0; i < pat.length; ++i)
            if (pat.steps[i].active && pat.steps[i].note == firstNote) {
                pat.steps[i].note = static_cast<uint8_t>(degreeToNote(scale, p.root, fifth));
                break;
            }
    }
    if (accents < 1) { pat.steps[0].active = true; pat.steps[0].accent = true; pat.steps[0].velocity = 110; }

    return pat;
}

Pattern MaghrebiAcid::mutate(const Pattern& in, uint32_t mutationSeed,
                             const GenParams& p) const {
    return in; // implemented in Task 7
}

} // namespace ayyy
```

- [ ] **Step 5: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS (all invariant + determinism tests).

- [ ] **Step 6: Commit**
```bash
git add core/maghrebi_acid.h core/maghrebi_acid.cpp tests/test_brain.cpp
git commit -m "feat(core): maghrebi-acid v0 generate() with garbage rejection"
```

---

### Task 7: mutate() — one small change

**Files:**
- Modify: `core/maghrebi_acid.cpp` (replace the mutate stub)
- Test: append to `tests/test_brain.cpp`

- [ ] **Step 1: Write failing tests**

Append to `tests/test_brain.cpp`:
```cpp
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

    CHECK(stepDiffs(base, m1) >= 1);   // it changed something
    CHECK(stepDiffs(base, m1) <= 2);   // but only a nudge
    CHECK(stepDiffs(m1, m2) == 0);     // deterministic for same mutationSeed
    CHECK(m1.length == base.length);   // structure preserved
}
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build && ctest --test-dir build --output-on-failure 2>&1 | tail`
Expected: FAIL — `stepDiffs(base, m1) >= 1` fails (stub returns input unchanged).

- [ ] **Step 3: Implement mutate()**

In `core/maghrebi_acid.cpp`, replace the `mutate` body with:
```cpp
Pattern MaghrebiAcid::mutate(const Pattern& in, uint32_t mutationSeed,
                             const GenParams& p) const {
    Pattern out = in;
    Rng rng(mutationSeed ? mutationSeed : 1u);
    const Scale& scale = scaleAt(p.scaleIndex);

    int op = rng.below(4);
    int i = rng.below(out.length);
    switch (op) {
        case 0: // toggle a step on/off
            out.steps[i].active = !out.steps[i].active;
            if (out.steps[i].active && out.steps[i].note == 0)
                out.steps[i].note = static_cast<uint8_t>(degreeToNote(scale, p.root, 0));
            break;
        case 1: // nudge one pitch by a scale step
            if (out.steps[i].active) {
                int dir = rng.chance(0.5f) ? 1 : -1;
                int note = int(out.steps[i].note) + (dir > 0 ? 2 : -2);
                out.steps[i].note = static_cast<uint8_t>(std::max(0, std::min(127, note)));
            } else { out.steps[i].active = true; out.steps[i].note =
                static_cast<uint8_t>(degreeToNote(scale, p.root, 0)); }
            break;
        case 2: // flip an accent
            out.steps[i].accent = !out.steps[i].accent;
            out.steps[i].velocity = out.steps[i].accent ? 110 : 78;
            break;
        default: // inject/remove a slide
            out.steps[i].slide = !out.steps[i].slide;
            break;
    }
    return out;
}
```

- [ ] **Step 4: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 5: Commit**
```bash
git add core/maghrebi_acid.cpp tests/test_brain.cpp
git commit -m "feat(core): mutate() — single deterministic nudge"
```

---

### Task 8: Render a Pattern to timed MIDI events

**Files:**
- Create: `platform/midi/render.h`
- Modify: `platform/midi/render.cpp` (replace stub)
- Test: `tests/test_render.cpp`

Timing model: PPQ = 480. A step is a triplet-eighth (160 ticks) when `ternary`, else a 16th (120 ticks). A non-slide note gates at 50% of the step. A slide note extends past the next active note's onset by `kOverlap` ticks (legato → 303 portamento). Accent → velocity already set on the step.

- [ ] **Step 1: Write failing tests**

`tests/test_render.cpp`:
```cpp
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
    for (auto& e : ev) { if ((e.status & 0xF0) == 0x90 && e.data2 > 0) ++ons;
                         else ++offs; }
    CHECK(ons == 2);
    CHECK(offs == 2);
}

TEST_CASE("slide makes the note overlap the next onset (legato)") {
    ayyy::Pattern pat;
    pat.length = 2; pat.ternary = false;
    pat.steps[0].active = true; pat.steps[0].note = 33; pat.steps[0].slide = true;
    pat.steps[1].active = true; pat.steps[1].note = 35;
    auto ev = ayyy::renderPattern(pat);
    // find note-off of step 0 (note 33) and note-on of step 1 (note 35)
    int offTime = -1, onTime = -1;
    for (auto& e : ev) {
        bool on = ((e.status & 0xF0) == 0x90 && e.data2 > 0);
        if (!on && e.data1 == 33) offTime = e.tick;
        if (on && e.data1 == 35) onTime = e.tick;
    }
    CHECK(onTime == 120);          // step 1 onset = 1 * 120
    CHECK(offTime > onTime);       // step 0 still ringing → legato
}

TEST_CASE("events are sorted by tick") {
    ayyy::MaghrebiAcid brain; ayyy::GenParams p; p.length = 7;
    auto ev = ayyy::renderPattern(brain.generate(527, p));
    for (size_t i = 1; i < ev.size(); ++i) CHECK(ev[i-1].tick <= ev[i].tick);
}
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `render.h` not found.

- [ ] **Step 3: Implement the header**

Create `platform/midi/render.h`:
```cpp
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
```

- [ ] **Step 4: Implement render.cpp**

Replace `platform/midi/render.cpp` with:
```cpp
#include "render.h"
#include <algorithm>

namespace ayyy {

std::vector<MidiEvent> renderPattern(const Pattern& pat) {
    std::vector<MidiEvent> ev;
    const int st = stepTicks(pat.ternary);

    auto nextActive = [&](int i) {
        for (int j = i + 1; j < pat.length; ++j) if (pat.steps[j].active) return j;
        return -1;
    };

    for (int i = 0; i < pat.length; ++i) {
        const Step& s = pat.steps[i];
        if (!s.active) continue;
        int onTick = i * st;
        int offTick;
        if (s.slide) {
            int j = nextActive(i);
            offTick = (j >= 0 ? j * st : (i + 1) * st) + kOverlap;
        } else {
            offTick = onTick + st / 2;
        }
        ev.push_back({onTick,  0x90, s.note, s.velocity});
        ev.push_back({offTick, 0x80, s.note, 0});
    }

    std::stable_sort(ev.begin(), ev.end(), [](const MidiEvent& a, const MidiEvent& b) {
        if (a.tick != b.tick) return a.tick < b.tick;
        bool aOn = (a.status & 0xF0) == 0x90 && a.data2 > 0;
        bool bOn = (b.status & 0xF0) == 0x90 && b.data2 > 0;
        return (!aOn) && bOn; // at equal ticks, note-off before note-on
    });
    return ev;
}

} // namespace ayyy
```

- [ ] **Step 5: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 6: Commit**
```bash
git add platform/midi/render.h platform/midi/render.cpp tests/test_render.cpp
git commit -m "feat(midi): render pattern to timed events (ternary, slide legato)"
```

---

### Task 9: Standard MIDI File writer

**Files:**
- Create: `platform/midi/smf_writer.h`
- Modify: `platform/midi/smf_writer.cpp` (replace stub)
- Test: `tests/test_smf.cpp`

- [ ] **Step 1: Write failing tests (header bytes + varlen + round-trippable size)**

`tests/test_smf.cpp`:
```cpp
#include "doctest.h"
#include "smf_writer.h"
#include "render.h"

TEST_CASE("varlen encodes correctly") {
    std::vector<uint8_t> out;
    ayyy::writeVarLen(out, 0);     CHECK(out == std::vector<uint8_t>{0x00});
    out.clear();
    ayyy::writeVarLen(out, 128);   CHECK(out == std::vector<uint8_t>{0x81, 0x00});
    out.clear();
    ayyy::writeVarLen(out, 480);   CHECK(out == std::vector<uint8_t>{0x83, 0x60});
}

TEST_CASE("smf starts with a valid MThd header") {
    std::vector<ayyy::MidiEvent> ev = {
        {0, 0x90, 33, 100}, {120, 0x80, 33, 0}
    };
    auto bytes = ayyy::writeSmf(ev, /*bpm*/130);
    REQUIRE(bytes.size() > 22);
    CHECK(bytes[0]=='M'); CHECK(bytes[1]=='T'); CHECK(bytes[2]=='h'); CHECK(bytes[3]=='d');
    // header length 6
    CHECK(bytes[7] == 6);
    // format 0
    CHECK(bytes[8] == 0); CHECK(bytes[9] == 0);
    // division = 480 = 0x01E0
    CHECK(bytes[12] == 0x01); CHECK(bytes[13] == 0xE0);
    // track chunk follows
    CHECK(bytes[14]=='M'); CHECK(bytes[15]=='T'); CHECK(bytes[16]=='r'); CHECK(bytes[17]=='k');
}
```

- [ ] **Step 2: Run to verify it fails**

Run: `cmake --build build 2>&1 | head`
Expected: FAIL — `smf_writer.h` not found.

- [ ] **Step 3: Implement the header**

Create `platform/midi/smf_writer.h`:
```cpp
#pragma once
#include "render.h"
#include <cstdint>
#include <vector>

namespace ayyy {

void writeVarLen(std::vector<uint8_t>& out, uint32_t value);
std::vector<uint8_t> writeSmf(const std::vector<MidiEvent>& events, int bpm);

} // namespace ayyy
```

- [ ] **Step 4: Implement smf_writer.cpp**

Replace `platform/midi/smf_writer.cpp` with:
```cpp
#include "smf_writer.h"

namespace ayyy {

void writeVarLen(std::vector<uint8_t>& out, uint32_t value) {
    uint32_t buffer = value & 0x7F;
    while ((value >>= 7)) { buffer <<= 8; buffer |= ((value & 0x7F) | 0x80); }
    while (true) {
        out.push_back(static_cast<uint8_t>(buffer & 0xFF));
        if (buffer & 0x80) buffer >>= 8; else break;
    }
}

static void push32(std::vector<uint8_t>& o, uint32_t v) {
    o.push_back((v>>24)&0xFF); o.push_back((v>>16)&0xFF);
    o.push_back((v>>8)&0xFF);  o.push_back(v&0xFF);
}
static void push16(std::vector<uint8_t>& o, uint16_t v) {
    o.push_back((v>>8)&0xFF); o.push_back(v&0xFF);
}

std::vector<uint8_t> writeSmf(const std::vector<MidiEvent>& events, int bpm) {
    std::vector<uint8_t> track;

    // tempo meta at tick 0
    uint32_t usPerQuarter = static_cast<uint32_t>(60000000.0 / (bpm > 0 ? bpm : 120));
    writeVarLen(track, 0);
    track.push_back(0xFF); track.push_back(0x51); track.push_back(0x03);
    track.push_back((usPerQuarter>>16)&0xFF);
    track.push_back((usPerQuarter>>8)&0xFF);
    track.push_back(usPerQuarter&0xFF);

    int last = 0;
    for (const auto& e : events) {
        writeVarLen(track, static_cast<uint32_t>(e.tick - last));
        last = e.tick;
        track.push_back(e.status);
        track.push_back(e.data1);
        track.push_back(e.data2);
    }
    // end of track
    writeVarLen(track, 0);
    track.push_back(0xFF); track.push_back(0x2F); track.push_back(0x00);

    std::vector<uint8_t> out;
    out.push_back('M'); out.push_back('T'); out.push_back('h'); out.push_back('d');
    push32(out, 6);
    push16(out, 0);      // format 0
    push16(out, 1);      // one track
    push16(out, kPPQ);   // division
    out.push_back('M'); out.push_back('T'); out.push_back('r'); out.push_back('k');
    push32(out, static_cast<uint32_t>(track.size()));
    out.insert(out.end(), track.begin(), track.end());
    return out;
}

} // namespace ayyy
```

- [ ] **Step 5: Build + run**

Run: `cmake --build build && ctest --test-dir build --output-on-failure`
Expected: PASS.

- [ ] **Step 6: Commit**
```bash
git add platform/midi/smf_writer.h platform/midi/smf_writer.cpp tests/test_smf.cpp
git commit -m "feat(midi): standard midi file writer (format 0)"
```

---

### Task 10: The CLI

**Files:**
- Modify: `apps/cli/main.cpp` (replace stub)

- [ ] **Step 1: Implement the CLI**

Replace `apps/cli/main.cpp` with:
```cpp
#include "maghrebi_acid.h"
#include "scales.h"
#include "render.h"
#include "smf_writer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

static const char* argVal(int argc, char** argv, const char* key, const char* def) {
    for (int i = 1; i < argc - 1; ++i) if (std::strcmp(argv[i], key) == 0) return argv[i+1];
    return def;
}

int main(int argc, char** argv) {
    ayyy::GenParams p;
    uint32_t seed = static_cast<uint32_t>(std::atoi(argVal(argc, argv, "--seed", "527")));
    p.scaleIndex  = ayyy::scaleIndexByName(argVal(argc, argv, "--scale", "hijaz"));
    p.length      = std::atoi(argVal(argc, argv, "--length", "7"));
    p.density     = static_cast<float>(std::atof(argVal(argc, argv, "--density", "0.55")));
    p.root        = static_cast<uint8_t>(std::atoi(argVal(argc, argv, "--root", "33")));
    p.ternary     = std::strcmp(argVal(argc, argv, "--feel", "ternary"), "ternary") == 0;
    int bpm       = std::atoi(argVal(argc, argv, "--bpm", "130"));
    const char* out = argVal(argc, argv, "-o", "out.mid");

    ayyy::MaghrebiAcid brain;
    ayyy::Pattern pat = brain.generate(seed, p);
    auto events = ayyy::renderPattern(pat);
    auto bytes  = ayyy::writeSmf(events, bpm);

    std::ofstream f(out, std::ios::binary);
    if (!f) { std::fprintf(stderr, "cannot write %s\n", out); return 1; }
    f.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));

    std::printf("wrote %s  (seed %u, scale %s, length %d, %s, %d bpm)\n",
                out, seed, argVal(argc, argv, "--scale", "hijaz"),
                p.length, p.ternary ? "ternary" : "straight", bpm);
    return 0;
}
```

- [ ] **Step 2: Build**

Run: `cmake --build build`
Expected: builds `build/ayyybass`.

- [ ] **Step 3: Generate a file and eyeball it**

Run:
```bash
./build/ayyybass --seed 527 --scale hijaz --length 7 --density 0.55 --bpm 130 -o out.mid
ls -l out.mid
```
Expected: prints the summary line; `out.mid` exists and is non-empty.

- [ ] **Step 4: Verify determinism**

Run:
```bash
./build/ayyybass --seed 527 --scale hijaz --length 7 -o a.mid
./build/ayyybass --seed 527 --scale hijaz --length 7 -o b.mid
cmp a.mid b.mid && echo IDENTICAL
```
Expected: `IDENTICAL`.

- [ ] **Step 5: Listen (manual acceptance)**

Drag `out.mid` onto a TD-3 (via MIDI) or into Ableton/Bitwig on a 303-style synth. Confirm: it loops, it's ternary, it has accents and audible slides. (Quality tuning is M4, not this plan.)

- [ ] **Step 6: Commit**
```bash
git add apps/cli/main.cpp
git commit -m "feat(cli): generate a maghrebi-acid pattern to a midi file"
```

---

## Self-review (completed by author)

- **Spec coverage:** ternary feel ✓ (render + GenParams), 12-TET modes ✓ (scales), slide-as-core ✓ (render legato), accent ✓ (velocity), free length ✓ (length param + render), density ✓, seed-deterministic / shareable number ✓, body↔brain contract ✓ (IBrain), portable I/O-free core ✓ (boundary rule). Deferred per spec and NOT in this plan: canvas beat, OLED/face, taste-learning, microtonal, web/hardware bodies, multiple brains.
- **Placeholder scan:** no TBD/TODO; every code step is complete; the only intentionally-iterative item (musical quality) is scoped out as M4 with a clear reason.
- **Type consistency:** `GenParams`, `Pattern`, `Step`, `MidiEvent`, `IBrain::generate/mutate`, `renderPattern`, `writeSmf`, `writeVarLen`, `scaleIndexByName/scaleAt/degreeToNote` are used identically across tasks.
