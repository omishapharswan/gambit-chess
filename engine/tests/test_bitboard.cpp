// Unit tests for the lowest engine layer: basic types and bitboard helpers.
#include <string>

#include "gambit/bitboard.hpp"
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

TEST(masks_and_square_bits) {
    const Bitboard ranks[8] = {RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB,
                               RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB};
    Bitboard all = EMPTY_BB;
    for (const Bitboard r : ranks) {
        CHECK_EQ(popcount(r), 8);
        CHECK_EQ(all & r, EMPTY_BB);  // ranks must not overlap
        all |= r;
    }
    CHECK_EQ(all, ALL_BB);
    CHECK_EQ(popcount(FILE_A_BB), 8);
    CHECK_EQ(popcount(FILE_H_BB), 8);
    CHECK_EQ(squareBb(A1), 1ULL);
    CHECK_EQ(squareBb(H8), 1ULL << 63);
    CHECK_EQ(FILE_A_BB & RANK_1_BB, squareBb(A1));
    CHECK_EQ(FILE_H_BB & RANK_8_BB, squareBb(H8));
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(popcount(squareBb(s)), 1);
        CHECK((squareBb(s) & ranks[rankOf(s)]) != 0);
        CHECK_EQ((squareBb(s) & FILE_A_BB) != 0, fileOf(s) == 0);
        CHECK_EQ((squareBb(s) & FILE_H_BB) != 0, fileOf(s) == 7);
    }
}

TEST(counting) {
    CHECK_EQ(popcount(EMPTY_BB), 0);
    CHECK_EQ(popcount(ALL_BB), 64);
    CHECK_EQ(popcount(0xF0F0ULL), 8);
    CHECK(!moreThanOne(EMPTY_BB));
    CHECK(!moreThanOne(squareBb(D4)));
    CHECK(moreThanOne(squareBb(D4) | squareBb(D5)));
    CHECK(moreThanOne(ALL_BB));
}

TEST(bit_scans) {
    const Bitboard two = squareBb(C3) | squareBb(F7);
    CHECK_EQ(lsb(two), C3);
    CHECK_EQ(msb(two), F7);
    CHECK_EQ(lsb(ALL_BB), A1);
    CHECK_EQ(msb(ALL_BB), H8);
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(lsb(squareBb(s)), s);
        CHECK_EQ(msb(squareBb(s)), s);
    }
}

TEST(pop_lsb_walks_set_bits_in_order) {
    // popLsb changes its argument, so each call result goes into a local first.
    Bitboard b = squareBb(A1) | squareBb(E4) | squareBb(H8);
    const Square first = popLsb(b);
    const Square second = popLsb(b);
    CHECK_EQ(first, A1);
    CHECK_EQ(second, E4);
    CHECK_EQ(b, squareBb(H8));
    const Square third = popLsb(b);
    CHECK_EQ(third, H8);
    CHECK_EQ(b, EMPTY_BB);
}

TEST(shifts_move_one_step) {
    const Bitboard e4 = squareBb(E4);
    CHECK_EQ(north(e4), squareBb(E5));
    CHECK_EQ(south(e4), squareBb(E3));
    CHECK_EQ(east(e4), squareBb(F4));
    CHECK_EQ(west(e4), squareBb(D4));
    CHECK_EQ(northEast(e4), squareBb(F5));
    CHECK_EQ(northWest(e4), squareBb(D5));
    CHECK_EQ(southEast(e4), squareBb(F3));
    CHECK_EQ(southWest(e4), squareBb(D3));
}

TEST(shifts_do_not_wrap_around_the_board) {
    const Bitboard a4 = squareBb(A4);
    const Bitboard h4 = squareBb(H4);
    CHECK_EQ(west(a4), EMPTY_BB);
    CHECK_EQ(northWest(a4), EMPTY_BB);
    CHECK_EQ(southWest(a4), EMPTY_BB);
    CHECK_EQ(east(h4), EMPTY_BB);
    CHECK_EQ(northEast(h4), EMPTY_BB);
    CHECK_EQ(southEast(h4), EMPTY_BB);
    CHECK_EQ(north(RANK_8_BB), EMPTY_BB);
    CHECK_EQ(south(RANK_1_BB), EMPTY_BB);
    // A full board loses exactly the squares that would leave it.
    CHECK_EQ(popcount(north(ALL_BB)), 56);
    CHECK_EQ(popcount(east(ALL_BB)), 56);
    CHECK_EQ(popcount(northEast(ALL_BB)), 49);
}

TEST(bitboard_diagram) {
    const std::string expected =
        ".......1\n"
        "........\n"
        "........\n"
        "........\n"
        "........\n"
        "........\n"
        "........\n"
        "1.......\n";
    CHECK_EQ(bitboardToString(squareBb(A1) | squareBb(H8)), expected);
}

int main() { return minitest::run_all(); }
