#include "gambit/move.hpp"

namespace gambit {

namespace {

// Index is the promoted piece minus KNIGHT, the same order promotionFlag uses.
const char PROMOTION_LETTERS[4] = {'n', 'b', 'r', 'q'};

}  // namespace

std::string moveToUci(Move m) {
    if (m.isNull()) {
        return "0000";
    }
    std::string text = squareToString(m.from()) + squareToString(m.to());
    if (m.isPromotion()) {
        text += PROMOTION_LETTERS[m.promotionType() - KNIGHT];
    }
    return text;
}

Move moveFromUci(const std::string& text) {
    if (text.size() != 4 && text.size() != 5) {
        return NO_MOVE;
    }
    const Square from = squareFromString(text.substr(0, 2));
    const Square to = squareFromString(text.substr(2, 2));
    // A move from a square to itself is never legal, and it would collide with NO_MOVE.
    if (from == SQUARE_NONE || to == SQUARE_NONE || from == to) {
        return NO_MOVE;
    }
    if (text.size() == 4) {
        return Move(from, to, FLAG_QUIET);
    }
    for (int i = 0; i < 4; ++i) {
        if (text[4] == PROMOTION_LETTERS[i]) {
            return Move(from, to, promotionFlag(static_cast<PieceType>(KNIGHT + i), false));
        }
    }
    return NO_MOVE;
}

bool sameUciMove(Move a, Move b) {
    if (a.from() != b.from() || a.to() != b.to() || a.isPromotion() != b.isPromotion()) {
        return false;
    }
    return !a.isPromotion() || a.promotionType() == b.promotionType();
}

}  // namespace gambit
