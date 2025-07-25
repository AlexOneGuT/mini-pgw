#include "BCDHelper.hpp"
#include <cstdint>

std::vector<uint8_t> string_to_bcd(const std::string& imsi) {
    std::vector<uint8_t> bcd;
    size_t len = imsi.size();
    for (size_t i = 0; i < len; i += 2) {
        uint8_t high = (i < len) ? (imsi[i] - '0') : 0x0F;
        uint8_t low = (i + 1 < len) ? (imsi[i + 1] - '0') : 0x0F;
        bcd.push_back((high << 4) | low);
    }
    return bcd;
}

std::string bcd_to_string(const uint8_t* data, size_t length) {
    std::string imsi;
    for (size_t i = 0; i < length; ++i) {
        imsi += '0' + ((data[i] >> 4) & 0x0F);
        uint8_t low = data[i] & 0x0F;
        if (low != 0x0F) {
            imsi += '0' + low;
        }
    }
    return imsi;
}