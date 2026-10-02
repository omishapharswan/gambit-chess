// A move packed into 16 bits: 6 bits from-square, 6 bits to-square, 4 bits flags.
// Two bytes keep move lists and the transposition table small, and two moves can be
// compared with a single integer comparison.
#ifndef GAMBIT_MOVE_HPP
#define GAMBIT_MOVE_HPP

#include <cstdint>
#include <string>

#include "gambit/types.hpp"

namespace gambit {

// Bit 2 (value 4) marks a capture and bit 3 (value 8) marks a promotion, so one AND answers
// "is this a capture" for plain captures, en passant and promotion captures alike.
// For promotions the two low bits hold the promoted piece (see promotionFlag).
// The values 6 and 7 are never used.
constexpr int FLAG_QUIET = 0;
constexpr int FLAG_DOUBLE_PUSH = 1;
constexpr int FLAG_KING_CASTLE = 2;
constexpr int FLAG_QUEEN_CASTLE = 3;
constexpr int FLAG_CAPTURE = 4;
constexpr int FLAG_EN_PASSANT = 5;
constexpr int FLAG_PROMOTION = 8;

// Precondition: pt is KNIGHT, BISHOP, ROOK or QUEEN. Result is in 8..15.
constexpr int promotionFlag(PieceType pt, bool capture) {
    return FLAG_PROMOTION | (capture ? FLAG_CAPTURE : 0) | (static_cast<int>(pt) - KNIGHT);
}

class Move {
public:
    // Value 0 means "no move". It is a1 to a1, which no legal move can be.
    constexpr Move() : data_(0) {}

    // Preconditions: both squares are real squares and moveFlags is in 0..15.
    // Castling is stored as the king move (e1g1), the same way UCI writes it.
    constexpr Move(Square fromSquare, Square toSquare, int moveFlags)
        : data_(static_cast<std::uint16_t>(fromSquare | (toSquare << 6) | (moveFlags << 12))) {}

    constexpr Square from() const { return static_cast<Square>(data_ & 63); }
    constexpr Square to() const { return static_cast<Square>((data_ >> 6) & 63); }
    constexpr int flags() const { return data_ >> 12; }
    constexpr std::uint16_t raw() const { return data_; }

    constexpr bool isNull() const { return data_ == 0; }
    constexpr bool isCapture() const { return (flags() & FLAG_CAPTURE) != 0; }
    constexpr bool isPromotion() const { return (flags() & FLAG_PROMOTION) != 0; }
    constexpr bool isEnPassant() const { return flags() == FLAG_EN_PASSANT; }
    constexpr bool isCastling() const { return flags() == FLAG_KING_CASTLE || flags() == FLAG_QUEEN_CASTLE; }
    constexpr bool isDoublePush() const { return flags() == FLAG_DOUBLE_PUSH; }

    // Precondition: isPromotion().
    constexpr PieceType promotionType() const { return static_cast<PieceType>(KNIGHT + (flags() & 3)); }

private:
    std::uint16_t data_;
};

static_assert(sizeof(Move) == sizeof(std::uint16_t), "Move must stay 16 bits");

constexpr Move NO_MOVE = Move();

constexpr bool operator==(Move a, Move b) { return a.raw() == b.raw(); }
constexpr bool operator!=(Move a, Move b) { return !(a == b); }

// "e2e4", "e7e8q", and "0000" for NO_MOVE.
std::string moveToUci(Move m);

// Parses "e2e4" or "e7e8q". Returns NO_MOVE for any other text, so bad input cannot crash us.
// Text alone cannot tell a capture from a quiet move, so only the squares and the promotion
// piece of the result are meaningful. Use sameUciMove to find the real move in a legal list.
Move moveFromUci(const std::string& text);

// True when a and b have the same squares and the same promotion piece (or both have none).
// All other flags are ignored, which is what makes a parsed move match a generated one.
bool sameUciMove(Move a, Move b);

}  // namespace gambit

#endif  // GAMBIT_MOVE_HPP
