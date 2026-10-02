// Zobrist hashing keys. A position hash is the XOR of one random 64-bit key per feature
// (piece on square, black to move, castling rights, en-passant file). XOR is its own
// inverse, so make and unmake update the hash by XORing the same keys again.
#ifndef GAMBIT_ZOBRIST_HPP
#define GAMBIT_ZOBRIST_HPP

#include <cstdint>

#include "gambit/types.hpp"

namespace gambit {

using Key = std::uint64_t;

namespace detail {

// Filled once by zobrist.cpp before main() starts and never written afterwards.
extern Key pieceSquareKeys[PIECE_NB][SQUARE_NB];
extern Key sideKey;
extern Key castlingKeys[ALL_CASTLING + 1];
extern Key enPassantFileKeys[8];

}  // namespace detail

// Precondition: p != NO_PIECE and s != SQUARE_NONE.
inline Key pieceKey(Piece p, Square s) { return detail::pieceSquareKeys[p][s]; }

// XORed into the hash while black is to move.
inline Key sideToMoveKey() { return detail::sideKey; }

// Precondition: rights is a mask in 0..ALL_CASTLING. Each entry is the XOR of the keys
// of its single rights, so castlingKey(0) is 0 and a change from 'before' to 'after'
// is the single update castlingKey(before) ^ castlingKey(after).
inline Key castlingKey(int rights) { return detail::castlingKeys[rights]; }

// Only the file counts. The rank is implied by the side to move, so it adds no information.
// SQUARE_NONE gives 0, so "no en-passant square" needs no special case in the caller.
inline Key enPassantKey(Square ep) {
    return ep == SQUARE_NONE ? 0ULL : detail::enPassantFileKeys[fileOf(ep)];
}

}  // namespace gambit

#endif  // GAMBIT_ZOBRIST_HPP
