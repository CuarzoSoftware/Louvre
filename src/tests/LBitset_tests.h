#ifndef LBITSET_TESTS_H
#define LBITSET_TESTS_H

#include <LTest.h>
#include <LBitset.h>

using namespace Louvre;

enum Flags : UInt8
{
    A = static_cast<UInt8>(1) << 0,
    B = static_cast<UInt8>(1) << 1,
    C = static_cast<UInt8>(1) << 2,
    D = static_cast<UInt8>(1) << 3,
    E = static_cast<UInt8>(1) << 4,
    F = static_cast<UInt8>(1) << 5,
    G = static_cast<UInt8>(1) << 6,
    H = static_cast<UInt8>(1) << 7,
};

void LBitset_test_01()
{
    LSetTestName("LBitset_test_01");
    LBitset<Flags> bitset;
    LAssert("LBitset size should be 1 byte", sizeof(bitset) == 1);
}

void LBitset_test_02()
{
    LSetTestName("LBitset_test_02");
    LBitset<Flags> bitset;
    LAssert("LBitset value should be 0", bitset == 0 && bitset == 0);
}

void LBitset_test_03()
{
    LSetTestName("LBitset_test_03");
    LBitset<Flags> bitset { A | C };
    LAssert("LBitset value should be A | C", bitset == (A | C) && bitset == (A | C));
}

void LBitset_test_04()
{
    LSetTestName("LBitset_test_03");
    LBitset<Flags> bitset;
    bitset.add(A);
    LAssert("LBitset value should be A", bitset == (A) && bitset == (A));
    bitset |= B;
    LAssert("LBitset value should be A | B", bitset == (A | B) && bitset == (A | B));
    bitset = bitset | C;
    LAssert("LBitset value should be A | B | C", bitset == (A | B | C) && bitset == (A | B | C));
    bitset &= C;
    LAssert("LBitset value should be C", bitset == (C) && bitset == (C));
    bitset.remove(C);
    LAssert("LBitset value should be 0", bitset == 0 && bitset == 0);
    bitset = A | B | C | D | E | F;
    bitset =~bitset;
    LAssert("LBitset value should be G | H", bitset == (G | H) && bitset == (G | H));
}

void LBitset_run_tests()
{
    LBitset_test_01();
    LBitset_test_02();
    LBitset_test_03();
    LBitset_test_04();
}

#endif // LBITSET_TESTS_H
