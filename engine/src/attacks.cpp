#include "gambit/attacks.hpp"

namespace gambit {

namespace detail {
Bitboard knightTable[SQUARE_NB];
Bitboard kingTable[SQUARE_NB];
Bitboard pawnTable[COLOR_NB][SQUARE_NB];
Bitboard rayTable[DIR_NB][SQUARE_NB];
}  // namespace detail

namespace {

// Every offset is {file step, rank step}.
const int KNIGHT_JUMPS[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
const int KING_JUMPS[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
// A pawn captures one rank forward: up the board for white, down for black.
const int PAWN_JUMPS[COLOR_NB][2][2] = {{{-1, 1}, {1, 1}}, {{-1, -1}, {1, -1}}};
// Same order as the Direction enum.
const int RAY_STEPS[DIR_NB][2] = {{0, 1}, {1, 0}, {1, 1}, {-1, 1}, {0, -1}, {-1, 0}, {1, -1}, {-1, -1}};

bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

// Squares reached from 'from' by one jump per offset, skipping jumps that leave the board.
Bitboard jumpTargets(Square from, const int (*offsets)[2], int count) {
    Bitboard result = EMPTY_BB;
    for (int i = 0; i < count; ++i) {
        const int file = fileOf(from) + offsets[i][0];
        const int rank = rankOf(from) + offsets[i][1];
        if (onBoard(file, rank)) {
            result |= squareBb(makeSquare(file, rank));
        }
    }
    return result;
}

// Every square on the straight line from 'from' to the board edge, 'from' excluded.
Bitboard walkRay(Square from, int fileStep, int rankStep) {
    Bitboard ray = EMPTY_BB;
    int file = fileOf(from) + fileStep;
    int rank = rankOf(from) + rankStep;
    while (onBoard(file, rank)) {
        ray |= squareBb(makeSquare(file, rank));
        file += fileStep;
        rank += rankStep;
    }
    return ray;
}

struct TableFiller {
    TableFiller() {
        for (int i = 0; i < SQUARE_NB; ++i) {
            const Square s = static_cast<Square>(i);
            detail::knightTable[s] = jumpTargets(s, KNIGHT_JUMPS, 8);
            detail::kingTable[s] = jumpTargets(s, KING_JUMPS, 8);
            for (int c = 0; c < COLOR_NB; ++c) {
                detail::pawnTable[c][s] = jumpTargets(s, PAWN_JUMPS[c], 2);
            }
            for (int d = 0; d < DIR_NB; ++d) {
                detail::rayTable[d][s] = walkRay(s, RAY_STEPS[d][0], RAY_STEPS[d][1]);
            }
        }
    }
};

// A global object runs its constructor before main(), so no caller can forget an init
// call. The rule that follows: no other global initializer may read these tables.
const TableFiller tableFiller;

}  // namespace
}  // namespace gambit
