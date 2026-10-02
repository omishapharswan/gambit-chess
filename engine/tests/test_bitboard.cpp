// Unit tests for the lowest engine layer: basic types and bitboard helpers.
#include <string>

#include "gambit/types.hpp"
#include "minitest.hpp"

using namespace gambit;

TEST(square_coordinates) {
    CHECK_EQ(fileOf(A1), 0);
    CHECK_EQ(rankOf(A1), 0);
    CHECK_EQ(fileOf(H1), 7);
    CHECK_EQ(rankOf(H8), 7);
    CHECK_EQ(static_cast<int>(E4), 28);
    CHECK_EQ(makeSquare(4, 3), E4);
    CHECK_EQ(makeSquare(7, 7), H8);
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(makeSquare(fileOf(s), rankOf(s)), s);
    }
}

TEST(square_names) {
    CHECK_EQ(squareToString(A1), std::string("a1"));
    CHECK_EQ(squareToString(E4), std::string("e4"));
    CHECK_EQ(squareToString(H8), std::string("h8"));
    CHECK_EQ(squareToString(SQUARE_NONE), std::string("-"));
    CHECK_EQ(squareFromString("e4"), E4);
    CHECK_EQ(squareFromString("a1"), A1);
    CHECK_EQ(squareFromString("h8"), H8);
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(squareFromString(squareToString(s)), s);
    }
}

TEST(square_parsing_rejects_bad_input) {
    // Bad input must never crash the engine, so every malformed name maps to SQUARE_NONE.
    const char* bad[] = {"", "e", "e44", "i1", "a9", "a0", "E4", "4e", "-"};
    for (const char* text : bad) {
        CHECK_EQ(squareFromString(text), SQUARE_NONE);
    }
}

TEST(piece_helpers) {
    CHECK_EQ(makePiece(WHITE, KNIGHT), W_KNIGHT);
    CHECK_EQ(makePiece(BLACK, PAWN), B_PAWN);
    CHECK_EQ(makePiece(BLACK, KING), B_KING);
    CHECK_EQ(typeOf(B_QUEEN), QUEEN);
    CHECK_EQ(colorOf(B_QUEEN), BLACK);
    CHECK_EQ(colorOf(W_KING), WHITE);
    for (int c = 0; c < COLOR_NB; ++c) {
        for (int t = 0; t < PIECE_TYPE_NB; ++t) {
            const Piece p = makePiece(static_cast<Color>(c), static_cast<PieceType>(t));
            CHECK_EQ(colorOf(p), static_cast<Color>(c));
            CHECK_EQ(typeOf(p), static_cast<PieceType>(t));
        }
    }
}

TEST(opposite_color) {
    CHECK_EQ(opposite(WHITE), BLACK);
    CHECK_EQ(opposite(BLACK), WHITE);
}

TEST(castling_rights_are_distinct_bits) {
    // If every right owns its own bit, OR and sum give the same total.
    CHECK_EQ(WHITE_OO | WHITE_OOO | BLACK_OO | BLACK_OOO, ALL_CASTLING);
    CHECK_EQ(WHITE_OO + WHITE_OOO + BLACK_OO + BLACK_OOO, ALL_CASTLING);
}

int main() { return minitest::run_all(); }
