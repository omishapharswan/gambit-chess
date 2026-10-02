// Basic chess vocabulary shared by every engine module: colors, pieces,
// squares and the small helpers that convert between them.
#ifndef GAMBIT_TYPES_HPP
#define GAMBIT_TYPES_HPP

#include <cstdint>
#include <string>

namespace gambit {

// Bit i is set when square i belongs to the set. 64 squares fit in one machine
// word, so set union and intersection become single OR / AND instructions.
using Bitboard = std::uint64_t;

enum Color : int { WHITE, BLACK, COLOR_NB };

enum PieceType : int { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB };

// White pieces come first, so piece = color * 6 + type and both parts can be
// recovered with one division or remainder.
enum Piece : int {
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PIECE,
    PIECE_NB = NO_PIECE  // NO_PIECE doubles as the array size
};

// Little-endian rank-file mapping: A1 = 0, B1 = 1, ..., H1 = 7, A2 = 8, ..., H8 = 63.
enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_NONE,
    SQUARE_NB = SQUARE_NONE  // SQUARE_NONE doubles as the array size
};

// Castling rights are a bit mask kept in a plain int, one bit per king and side.
// OO is king side, OOO is queen side (standard notation).
constexpr int WHITE_OO = 1;
constexpr int WHITE_OOO = 2;
constexpr int BLACK_OO = 4;
constexpr int BLACK_OOO = 8;
constexpr int ALL_CASTLING = 15;

// Each expression below mixes at most one enum with plain ints, which avoids
// the enum-with-enum arithmetic warnings of newer compilers.
constexpr Color opposite(Color c) { return static_cast<Color>(static_cast<int>(c) ^ 1); }

constexpr Piece makePiece(Color c, PieceType pt) {
    return static_cast<Piece>(static_cast<int>(c) * PIECE_TYPE_NB + pt);
}

// Precondition for typeOf and colorOf: p != NO_PIECE.
constexpr PieceType typeOf(Piece p) { return static_cast<PieceType>(static_cast<int>(p) % PIECE_TYPE_NB); }
constexpr Color colorOf(Piece p) { return static_cast<Color>(static_cast<int>(p) / PIECE_TYPE_NB); }

constexpr int fileOf(Square s) { return s & 7; }  // 0 = file a ... 7 = file h
constexpr int rankOf(Square s) { return s >> 3; }  // 0 = rank 1 ... 7 = rank 8
constexpr Square makeSquare(int file, int rank) { return static_cast<Square>(rank * 8 + file); }

// "e4" for a valid square, "-" for anything else (the FEN spelling of "no square").
std::string squareToString(Square s);

// Inverse of squareToString. Returns SQUARE_NONE for any text that is not exactly
// a lowercase file letter followed by a rank digit, so bad input cannot crash us.
Square squareFromString(const std::string& text);

}  // namespace gambit

#endif  // GAMBIT_TYPES_HPP
