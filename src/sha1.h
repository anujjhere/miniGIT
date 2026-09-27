#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>

inline std::string sha1(const std::string& input) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
             h3 = 0x10325476, h4 = 0xC3D2E1F0;

    std::string msg = input;
    uint64_t ml = msg.size() * 8;

    msg += (char)0x80;
    while (msg.size() % 64 != 56) msg += (char)0x00;

    for (int i = 7; i >= 0; i--)
        msg += (char)((ml >> (i * 8)) & 0xFF);

    for (size_t chunkStart = 0; chunkStart < msg.size(); chunkStart += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint8_t)msg[chunkStart + i * 4] << 24) |
                   ((uint8_t)msg[chunkStart + i * 4 + 1] << 16) |
                   ((uint8_t)msg[chunkStart + i * 4 + 2] << 8) |
                   ((uint8_t)msg[chunkStart + i * 4 + 3]);
        }
        for (int i = 16; i < 80; i++) {
            uint32_t v = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
            w[i] = (v << 1) | (v >> 31);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d);        k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;                   k = 0xCA62C1D6; }

            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
            e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
        }

        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << std::setw(8) << h0 << std::setw(8) << h1 << std::setw(8) << h2
        << std::setw(8) << h3 << std::setw(8) << h4;
    return out.str();
}