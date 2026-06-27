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
        {"pentatonic", {{0, 3, 5, 7, 10, 0, 0, 0, 0, 0, 0, 0}}, 5},
        {"hijaz",      {{0, 1, 4, 5, 7, 8, 10, 0, 0, 0, 0, 0}}, 7},
        {"kurd",       {{0, 1, 3, 5, 7, 8, 10, 0, 0, 0, 0, 0}}, 7},
        {"nahawand",   {{0, 2, 3, 5, 7, 8, 10, 0, 0, 0, 0, 0}}, 7},
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
