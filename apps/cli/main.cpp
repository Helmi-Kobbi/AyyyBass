#include "maghrebi_acid.h"
#include "scales.h"
#include "render.h"
#include "smf_writer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

static const char* argVal(int argc, char** argv, const char* key, const char* def) {
    for (int i = 1; i < argc - 1; ++i) if (std::strcmp(argv[i], key) == 0) return argv[i+1];
    return def;
}

int main(int argc, char** argv) {
    ayyy::GenParams p;
    uint32_t seed = static_cast<uint32_t>(std::atoi(argVal(argc, argv, "--seed", "527")));
    const char* scaleName = argVal(argc, argv, "--scale", "hijaz");
    p.scaleIndex  = ayyy::scaleIndexByName(scaleName);
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
                out, seed, scaleName, p.length, p.ternary ? "ternary" : "straight", bpm);
    return 0;
}
