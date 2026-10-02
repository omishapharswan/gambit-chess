// Attack lookup for every piece type. Knight, king and pawn attacks are plain tables
// indexed by square. Sliders use the classical ray method: one precomputed ray per
// direction, cut off behind the first blocker that a bit scan finds.
#ifndef GAMBIT_ATTACKS_HPP
#define GAMBIT_ATTACKS_HPP

#include "gambit/bitboard.hpp"
#include "gambit/types.hpp"

namespace gambit {

// The first four directions run toward higher square numbers, the last four toward
// lower ones. That order tells slideRay which end of a bitboard holds the nearest blocker.
enum Direction : int {
    DIR_N, DIR_E, DIR_NE, DIR_NW,
    DIR_S, DIR_W, DIR_SE, DIR_SW,
    DIR_NB
};

namespace detail {

// Filled once by attacks.cpp before main() starts and never written afterwards.
extern Bitboard knightTable[SQUARE_NB];
extern Bitboard kingTable[SQUARE_NB];
extern Bitboard pawnTable[COLOR_NB][SQUARE_NB];
extern Bitboard rayTable[DIR_NB][SQUARE_NB];  // squares from s to the board edge, s excluded

// Squares seen from s in direction d, up to and including the first blocker.
inline Bitboard slideRay(Direction d, Square s, Bitboard occupied) {
    const Bitboard ray = rayTable[d][s];
    const Bitboard blockers = ray & occupied;
    if (blockers == EMPTY_BB) {
        return ray;
    }
    // Rays toward higher squares meet their nearest blocker at the lowest set bit,
    // rays toward lower squares at the highest set bit.
    const Square first = d < DIR_S ? lsb(blockers) : msb(blockers);
    // The ray that starts at the blocker is exactly the part hidden behind it.
    return ray ^ rayTable[d][first];
}

}  // namespace detail

// Jumpers ignore blockers. Pawn attacks are the two diagonal capture squares only,
// never the push squares. Entries exist for every square, even ranks no pawn can use.
inline Bitboard knightAttacks(Square s) { return detail::knightTable[s]; }
inline Bitboard kingAttacks(Square s) { return detail::kingTable[s]; }
inline Bitboard pawnAttacks(Color c, Square s) { return detail::pawnTable[c][s]; }

// Sliders: 'occupied' holds the pieces of both colors. The result includes the first
// blocker in each direction whatever its color, so the caller removes its own pieces.
inline Bitboard rookAttacks(Square s, Bitboard occupied) {
    return detail::slideRay(DIR_N, s, occupied) | detail::slideRay(DIR_E, s, occupied) |
           detail::slideRay(DIR_S, s, occupied) | detail::slideRay(DIR_W, s, occupied);
}

inline Bitboard bishopAttacks(Square s, Bitboard occupied) {
    return detail::slideRay(DIR_NE, s, occupied) | detail::slideRay(DIR_NW, s, occupied) |
           detail::slideRay(DIR_SE, s, occupied) | detail::slideRay(DIR_SW, s, occupied);
}

inline Bitboard queenAttacks(Square s, Bitboard occupied) {
    return rookAttacks(s, occupied) | bishopAttacks(s, occupied);
}

}  // namespace gambit

#endif  // GAMBIT_ATTACKS_HPP
