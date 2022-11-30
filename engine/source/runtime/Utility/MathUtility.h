#pragma once
#include <cstdint>
class FMathUtility
{
public:
    FMathUtility() = default;

    //将Value对齐Alignmnet, 即Value朝上取整, 直至%Align == 0
    //比如 RoundUp(5, 4) = 8, RoundUp(3, 4) = 4
    static uint32_t RoundUp(uint32_t Value, uint32_t Alignment);
};
