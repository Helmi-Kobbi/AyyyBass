#pragma once
#include "render.h"
#include <cstdint>
#include <vector>

namespace ayyy {

void writeVarLen(std::vector<uint8_t>& out, uint32_t value);
std::vector<uint8_t> writeSmf(const std::vector<MidiEvent>& events, int bpm);

} // namespace ayyy
