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
    o.push_back((v >> 24) & 0xFF); o.push_back((v >> 16) & 0xFF);
    o.push_back((v >> 8) & 0xFF);  o.push_back(v & 0xFF);
}
static void push16(std::vector<uint8_t>& o, uint16_t v) {
    o.push_back((v >> 8) & 0xFF); o.push_back(v & 0xFF);
}

std::vector<uint8_t> writeSmf(const std::vector<MidiEvent>& events, int bpm) {
    std::vector<uint8_t> track;

    uint32_t usPerQuarter = static_cast<uint32_t>(60000000.0 / (bpm > 0 ? bpm : 120));
    writeVarLen(track, 0);
    track.push_back(0xFF); track.push_back(0x51); track.push_back(0x03);
    track.push_back((usPerQuarter >> 16) & 0xFF);
    track.push_back((usPerQuarter >> 8) & 0xFF);
    track.push_back(usPerQuarter & 0xFF);

    int last = 0;
    for (const auto& e : events) {
        writeVarLen(track, static_cast<uint32_t>(e.tick - last));
        last = e.tick;
        track.push_back(e.status);
        track.push_back(e.data1);
        track.push_back(e.data2);
    }
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
