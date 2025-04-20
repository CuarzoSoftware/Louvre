#ifndef LRRECT_H
#define LRRECT_H

#include <LRect.h>

class Louvre::LRRect : public LRect
{
public:
    LRRect(const LRect &rect = { 0 }, Int32 radTL = 0, Int32 radTR = 0, Int32 radBR = 0, Int32 radBL = 0) noexcept
        : LRect(rect), fRadTL(radTL), fRadTR(radTR), fRadBR(radBR), fRadBL(radBL)
    {}

    bool isValid() const noexcept
    {
        return
            w() >= 0 &&
            h() >= 0 &&
            fRadTL >= 0 &&
            fRadTR >= 0 &&
            fRadBR >= 0 &&
            fRadBL >= 0 &&
            fRadTL + fRadTR <= w() &&
            fRadBL + fRadBR <= w() &&
            fRadTL + fRadBL <= h() &&
            fRadTR + fRadBR <= h();
    }

    Int32 fRadTL { 0 };
    Int32 fRadTR { 0 };
    Int32 fRadBR { 0 };
    Int32 fRadBL { 0 };
};

#endif // LRRECT_H
