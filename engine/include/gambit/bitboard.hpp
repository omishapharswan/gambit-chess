// Bitboard helpers: board masks, population count, bit scans and one-step shifts.
// Everything hot is header-only so the compiler can inline it into move generation.
#ifndef GAMBIT_BITBOARD_HPP
#define GAMBIT_BITBOARD_HPP

#include <string>

#include "gambit/types.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace gambit {

constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard ALL_BB = ~0ULL;

// Square i sits at bit i, so a whole file is one bit repeated every 8 positions
// and a whole rank is one byte.
constexpr Bitboard FILE_A_BB = 0x0101010101010101ULL;
constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;
constexpr Bitboard RANK_1_BB = 0xFFULL;
constexpr Bitboard RANK_2_BB = RANK_1_BB << 8;
constexpr Bitboard RANK_3_BB = RANK_1_BB << 16;
constexpr Bitboard RANK_4_BB = RANK_1_BB << 24;
constexpr Bitboard RANK_5_BB = RANK_1_BB << 32;
constexpr Bitboard RANK_6_BB = RANK_1_BB << 40;
constexpr Bitboard RANK_7_BB = RANK_1_BB << 48;
constexpr Bitboard RANK_8_BB = RANK_1_BB << 56;

// Precondition: s is a real square (not SQUARE_NONE), because shifting by 64 is undefined.
constexpr Bitboard squareBb(Square s) { return 1ULL << s; }

// True when at least two bits are set. b & (b - 1) clears the lowest set bit,
// so the result is zero only for zero or one bit. Used to detect double check.
constexpr bool moreThanOne(Bitboard b) { return (b & (b - 1)) != 0; }

// Compiler intrinsics are used instead of C++20 <bit> because CI and Docker build as C++17.
// Preconditions for lsb, msb and popLsb: b != 0.
inline int popcount(Bitboard b) {
#if defined(_MSC_VER)
    return static_cast<int>(__popcnt64(b));
#else
    return __builtin_popcountll(b);
#endif
}

inline Square lsb(Bitboard b) {
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, b);
    return static_cast<Square>(index);
#else
    return static_cast<Square>(__builtin_ctzll(b));
#endif
}

inline Square msb(Bitboard b) {
#if defined(_MSC_VER)
    unsigned long index;
    _BitScanReverse64(&index, b);
    return static_cast<Square>(index);
#else
    return static_cast<Square>(63 - __builtin_clzll(b));
#endif
}

// Removes the lowest set bit from b and returns its square: the standard way to walk a set.
inline Square popLsb(Bitboard& b) {
    const Square s = lsb(b);
    b &= b - 1;
    return s;
}

// One-step shifts. A move of +1 or +-7 or +-9 would wrap from one edge of the board
// to the opposite edge, so squares that would cross a file edge are masked out first.
// Shifts past rank 1 or rank 8 fall off the 64-bit word on their own.
constexpr Bitboard north(Bitboard b) { return b << 8; }
constexpr Bitboard south(Bitboard b) { return b >> 8; }
constexpr Bitboard east(Bitboard b) { return (b & ~FILE_H_BB) << 1; }
constexpr Bitboard west(Bitboard b) { return (b & ~FILE_A_BB) >> 1; }
constexpr Bitboard northEast(Bitboard b) { return (b & ~FILE_H_BB) << 9; }
constexpr Bitboard northWest(Bitboard b) { return (b & ~FILE_A_BB) << 7; }
constexpr Bitboard southEast(Bitboard b) { return (b & ~FILE_H_BB) >> 7; }
constexpr Bitboard southWest(Bitboard b) { return (b & ~FILE_A_BB) >> 9; }

// Eight lines of eight characters, rank 8 first, '1' for a set bit. Meant for test
// failure messages: a diagram is far easier to read than a 64-bit decimal number.
std::string bitboardToString(Bitboard b);

}  // namespace gambit

#endif  // GAMBIT_BITBOARD_HPP
