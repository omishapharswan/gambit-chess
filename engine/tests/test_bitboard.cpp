// Unit tests for the lowest engine layer: basic types and bitboard helpers.
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "gambit/attacks.hpp"
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

namespace {

// Distance rules for the jump pieces, written from the rules of chess and not copied
// from the table builder, so a mistake in either one shows up as a mismatch.
bool isKnightStep(int fileDist, int rankDist) {
    return (fileDist == 1 && rankDist == 2) || (fileDist == 2 && rankDist == 1);
}

bool isKingStep(int fileDist, int rankDist) {
    return fileDist <= 1 && rankDist <= 1 && fileDist + rankDist > 0;
}

// Every square whose absolute file and rank distance from 'from' satisfies the rule.
Bitboard squaresByRule(Square from, bool (*rule)(int, int)) {
    Bitboard result = EMPTY_BB;
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square to = static_cast<Square>(i);
        if (rule(std::abs(fileOf(from) - fileOf(to)), std::abs(rankOf(from) - rankOf(to)))) {
            result |= squareBb(to);
        }
    }
    return result;
}

}  // namespace

TEST(knight_attacks_match_geometry) {
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(knightAttacks(s), squaresByRule(s, isKnightStep));
    }
}

TEST(king_attacks_match_geometry) {
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        CHECK_EQ(kingAttacks(s), squaresByRule(s, isKingStep));
    }
}

TEST(pawn_attacks_match_shifts) {
    // The one-step shifts were tested in the previous step, so they serve as the oracle.
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        const Bitboard b = squareBb(s);
        CHECK_EQ(pawnAttacks(WHITE, s), northEast(b) | northWest(b));
        CHECK_EQ(pawnAttacks(BLACK, s), southEast(b) | southWest(b));
    }
}

TEST(edge_and_corner_cases) {
    CHECK_EQ(knightAttacks(A1), squareBb(B3) | squareBb(C2));
    CHECK_EQ(popcount(knightAttacks(E4)), 8);
    CHECK_EQ(kingAttacks(H1), squareBb(G1) | squareBb(G2) | squareBb(H2));
    CHECK_EQ(popcount(kingAttacks(E4)), 8);
    CHECK_EQ(popcount(kingAttacks(H5)), 5);
    CHECK_EQ(pawnAttacks(WHITE, E4), squareBb(D5) | squareBb(F5));
    CHECK_EQ(pawnAttacks(BLACK, E4), squareBb(D3) | squareBb(F3));
    CHECK_EQ(pawnAttacks(WHITE, A2), squareBb(B3));  // an a-file pawn has one capture square
    CHECK_EQ(pawnAttacks(BLACK, H7), squareBb(G6));
    CHECK_EQ(pawnAttacks(WHITE, E8), EMPTY_BB);  // there is no rank 9 to attack
    CHECK_EQ(pawnAttacks(BLACK, E1), EMPTY_BB);
}

TEST(rook_attacks_stop_at_first_blocker) {
    CHECK_EQ(rookAttacks(A1, EMPTY_BB), (FILE_A_BB | RANK_1_BB) ^ squareBb(A1));
    CHECK_EQ(popcount(rookAttacks(D4, EMPTY_BB)), 14);
    // One blocker on each side of d4. Squares behind a blocker are hidden, but the
    // blocker itself stays attacked because a rook may capture it.
    const Bitboard blockers = squareBb(D6) | squareBb(G4) | squareBb(D2) | squareBb(B4);
    const Bitboard expected = squareBb(D5) | squareBb(D6) | squareBb(E4) | squareBb(F4) |
                              squareBb(G4) | squareBb(D3) | squareBb(D2) | squareBb(C4) |
                              squareBb(B4);
    CHECK_EQ(rookAttacks(D4, blockers), expected);
    // Pieces that are not on the rook's own lines change nothing.
    CHECK_EQ(rookAttacks(D4, blockers | squareBb(H8) | squareBb(A1)), expected);
}

TEST(bishop_attacks_stop_at_first_blocker) {
    CHECK_EQ(popcount(bishopAttacks(A1, EMPTY_BB)), 7);
    CHECK_EQ(popcount(bishopAttacks(D4, EMPTY_BB)), 13);
    const Bitboard blockers = squareBb(F6) | squareBb(F2) | squareBb(B2) | squareBb(B6);
    const Bitboard expected = squareBb(E5) | squareBb(F6) | squareBb(E3) | squareBb(F2) |
                              squareBb(C3) | squareBb(B2) | squareBb(C5) | squareBb(B6);
    CHECK_EQ(bishopAttacks(D4, blockers), expected);
    CHECK_EQ(bishopAttacks(D4, blockers | squareBb(D8) | squareBb(H4)), expected);
}

namespace {

// Small fixed-seed generator (xorshift64*). The test must be repeatable: a failure on
// one run has to fail on the next run too.
struct TestRng {
    std::uint64_t state;
    std::uint64_t next() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 2685821657736338717ULL;
    }
};

// Mixes medium, sparse and dense boards so that long open rays and tight blockers both occur.
std::vector<Bitboard> randomOccupancies() {
    TestRng rng{0x9E3779B97F4A7C15ULL};
    std::vector<Bitboard> boards;
    for (int i = 0; i < 100; ++i) {
        const Bitboard a = rng.next();
        const Bitboard b = rng.next();
        const Bitboard c = rng.next();
        boards.push_back(a);
        boards.push_back(a & b & c);
        boards.push_back(a | b);
    }
    return boards;
}

const int ROOK_STEPS[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
const int BISHOP_STEPS[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

// Walks square by square and stops after the first occupied square. It is slow but
// obviously right, and it uses no ray table or bit scan from the code under test.
Bitboard slowSlider(Square from, Bitboard occupied, const int (*steps)[2]) {
    Bitboard result = EMPTY_BB;
    for (int i = 0; i < 4; ++i) {
        int file = fileOf(from) + steps[i][0];
        int rank = rankOf(from) + steps[i][1];
        while (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            const Bitboard bit = squareBb(makeSquare(file, rank));
            result |= bit;
            if ((occupied & bit) != 0) {
                break;
            }
            file += steps[i][0];
            rank += steps[i][1];
        }
    }
    return result;
}

}  // namespace

TEST(random_rook_attacks_match_slow_reference) {
    const std::vector<Bitboard> boards = randomOccupancies();
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        for (const Bitboard occupied : boards) {
            // The slider's own square may be set too, as it is in a real game.
            CHECK_EQ(rookAttacks(s, occupied), slowSlider(s, occupied, ROOK_STEPS));
        }
    }
}

TEST(random_bishop_attacks_match_slow_reference) {
    const std::vector<Bitboard> boards = randomOccupancies();
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        for (const Bitboard occupied : boards) {
            CHECK_EQ(bishopAttacks(s, occupied), slowSlider(s, occupied, BISHOP_STEPS));
        }
    }
}

TEST(queen_attacks_are_rook_plus_bishop) {
    const std::vector<Bitboard> boards = randomOccupancies();
    for (int i = 0; i < SQUARE_NB; ++i) {
        const Square s = static_cast<Square>(i);
        for (const Bitboard occupied : boards) {
            CHECK_EQ(queenAttacks(s, occupied),
                     slowSlider(s, occupied, ROOK_STEPS) | slowSlider(s, occupied, BISHOP_STEPS));
        }
    }
    CHECK_EQ(popcount(queenAttacks(D4, EMPTY_BB)), 27);  // 14 rook squares + 13 bishop squares
    CHECK_EQ(popcount(queenAttacks(A1, EMPTY_BB)), 21);  // 14 + 7
}

TEST(adjacent_and_edge_blockers) {
    // A blocker next to the slider hides everything behind it.
    CHECK_EQ(rookAttacks(A1, squareBb(A2) | squareBb(B1)), squareBb(A2) | squareBb(B1));
    CHECK_EQ(bishopAttacks(H8, squareBb(G7)), squareBb(G7));
    // A blocker on the last square of a ray hides nothing, so the result is unchanged.
    CHECK_EQ(rookAttacks(A1, squareBb(A8) | squareBb(H1)), rookAttacks(A1, EMPTY_BB));
    CHECK_EQ(bishopAttacks(A1, squareBb(H8)), bishopAttacks(A1, EMPTY_BB));
    // Two blockers on one ray: only the nearer one counts.
    CHECK_EQ(rookAttacks(A1, squareBb(A4) | squareBb(A7)),
             squareBb(A2) | squareBb(A3) | squareBb(A4) | (RANK_1_BB ^ squareBb(A1)));
}

int main() { return minitest::run_all(); }
