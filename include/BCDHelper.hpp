#pragma once
#include <string>
#include <vector>
#include <cstdint>

std::vector<uint8_t> string_to_bcd(const std::string& imsi);
std::string bcd_to_string(const uint8_t* data, size_t length);