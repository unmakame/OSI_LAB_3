#pragma once

#include <string>
#include <algorithm>

class StringProcessor {
public:
    static std::string removeVowels(const std::string& input);
    static bool shouldGoToPipe2(const std::string& input);
};