#include "utils.h"

std::string char2hex(uint8_t value) {
    char buffer[3]; // 2 characters for hex representation + 1 for null terminator

    // snprintf safely writes to the buffer, ensuring no overflow
    std::snprintf(buffer, sizeof(buffer), "%02x", value);

    // Construct a std::string from the buffer
    // The std::string constructor copies the content of buffer
    return std::string(buffer);
}

std::string array2String(uint8_t *packet, size_t packetSize)
{
    std::string result;
    result.reserve(packetSize);  // Reserve space to avoid multiple allocations

    for (size_t i = 0; i < packetSize; ++i) {
        result += char2hex(packet[i]);
        if(i+1 < packetSize) result += ":";
    }

    return result;
}

std::string valToString(UnitsValue units)
{
    switch(units)
    {
        case UnitsValue::mm: return "cm";
        case UnitsValue::inch: return "inch";
        default: return "unset";
    }
}

std::string valToString(UserLimitSetValue limits)
{

    switch(limits)
    {
        case UserLimitSetValue::None: return "None";
        case UserLimitSetValue::Max:  return "Max";
        case UserLimitSetValue::Min:  return "Min";
        case UserLimitSetValue::Both: return "Both";
        default: return "unset";
    }
}

std::string valToString(userLimitReachedValue limits)
{
    switch(limits)
    {
        case userLimitReachedValue::MaxReached: return "MaxReached";
        case userLimitReachedValue::MinReached: return "MinReached";
        default: return "unset";
    }
}

std::string valToString(TouchModeValue mode)
{
    switch(mode)
    {
        case TouchModeValue::Single: return "Single";
        case TouchModeValue::Continuous: return "Continuous";
        default: return "unset";
    }
}

std::string valToString(KillModeValue mode)
{
    switch(mode)
    {
        case KillModeValue::Kill: return "Kill";
        case KillModeValue::LetLive: return "LetLive";
        default: return "unset";
    }
}

std::string valToString(SensitivityValue sens)
{
    switch(sens)
    {
        case SensitivityValue::High: return "High";
        case SensitivityValue::Medium: return "Medium";
        case SensitivityValue::Low: return "Low";
        default: return "unset";
    }
}
