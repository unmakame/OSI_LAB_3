#include "stringProcessor.hpp"

std::string StringProcessor::removeVowels(const std::string& input) {
    const std::string VOWELS = "aeiouAEIOU";
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), 
        [&VOWELS](char c) {
            return VOWELS.find(c) != std::string::npos;
        }), result.end());
    return result;
}

bool StringProcessor::shouldGoToPipe2(const std::string& input) {
    return input.length() > 10;
}