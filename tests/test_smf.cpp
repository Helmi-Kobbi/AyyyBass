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
    CHECK(bytes[7] == 6);
    CHECK(bytes[8] == 0); CHECK(bytes[9] == 0);
    CHECK(bytes[12] == 0x01); CHECK(bytes[13] == 0xE0);
    CHECK(bytes[14]=='M'); CHECK(bytes[15]=='T'); CHECK(bytes[16]=='r'); CHECK(bytes[17]=='k');
}
