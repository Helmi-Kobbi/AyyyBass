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
