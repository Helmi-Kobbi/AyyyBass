#include "maghrebi_acid.h"
#include "rng.h"
#include "scales.h"
#include <algorithm>
#include <cstdlib>

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

    // Find the scale degree that is a perfect fifth (the ostinato's anchor).
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
                if (r < 0.45f)      degree = 0;        // home to the root
                else if (r < 0.65f) degree = fifth;    // the fifth
                else if (r < 0.85f) degree += (rng.chance(0.5f) ? 1 : -1); // step
                else                degree += (rng.chance(0.5f) ? 2 : -2); // small leap
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

    // Slides: a small stepwise move into the next active note glides (the gesture).
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

    // --- Garbage rejection (deterministic, collision-free) ---
    if (pat.length >= 2) {
        // 1) at least two active steps
        int active = 0;
        for (int i = 0; i < pat.length; ++i) if (pat.steps[i].active) ++active;
        if (active < 2) {
            pat.steps[0].active = true;
            pat.steps[0].note = static_cast<uint8_t>(degreeToNote(scale, p.root, 0));
            pat.steps[0].accent = true;
            pat.steps[0].velocity = 110;
            int at = pat.length / 2;
            if (at == 0) at = 1;
            pat.steps[at].active = true;
            pat.steps[at].note = static_cast<uint8_t>(
                degreeToNote(scale, p.root, fifth > 0 ? fifth : 1));
            pat.steps[at].velocity = 78;
        }
        // 2) at least two distinct pitches
        int firstNote = -1; bool distinct = false;
        for (int i = 0; i < pat.length; ++i)
            if (pat.steps[i].active) {
                if (firstNote < 0) firstNote = pat.steps[i].note;
                else if (pat.steps[i].note != firstNote) distinct = true;
            }
        if (!distinct) {
            for (int i = pat.length - 1; i >= 0; --i)
                if (pat.steps[i].active) {
                    int alt = degreeToNote(scale, p.root, fifth > 0 ? fifth : 1);
                    if (alt == firstNote) alt = degreeToNote(scale, p.root, 1);
                    if (alt == firstNote) alt = firstNote + 12;
                    pat.steps[i].note = static_cast<uint8_t>(alt);
                    break;
                }
        }
    }
    // 3) at least one accent
    bool anyAccent = false;
    for (int i = 0; i < pat.length; ++i) if (pat.steps[i].accent) anyAccent = true;
    if (!anyAccent) {
        pat.steps[0].active = true;
        pat.steps[0].accent = true;
        pat.steps[0].velocity = 110;
    }

    return pat;
}

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
        case 1: // nudge one pitch
            if (out.steps[i].active) {
                int dir = rng.chance(0.5f) ? 1 : -1;
                int note = int(out.steps[i].note) + (dir > 0 ? 2 : -2);
                out.steps[i].note = static_cast<uint8_t>(std::max(0, std::min(127, note)));
            } else {
                out.steps[i].active = true;
                out.steps[i].note = static_cast<uint8_t>(degreeToNote(scale, p.root, 0));
            }
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

} // namespace ayyy
