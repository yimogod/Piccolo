#include "MathUtility.h"

uint32_t FMathUtility::RoundUp(uint32_t Value, uint32_t Alignment)
{
    uint32_t Temp = Value + Alignment - static_cast<uint32_t>(1);
    return (Temp - Temp % Alignment);
}
