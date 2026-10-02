// Unit tests for moves and positions. Move encoding lives here because it is the
// closest of the five test executables.
#include <string>

#include "gambit/move.hpp"
#include "gambit/types.hpp"
#include "minitest.hpp"

using namespace gambit;

namespace {

// One letter per property that holds: C capture, P promotion, E en passant,
// K castling, D double pawn push. A readable string makes a failure easy to understand.
std::string describe(Move m) {
    std::string text;
    if (m.isCapture()) text += 'C';
    if (m.isPromotion()) text += 'P';
    if (m.isEnPassant()) text += 'E';
    if (m.isCastling()) text += 'K';
    if (m.isDoublePush()) text += 'D';
    return text;
}

}  // namespace

TEST(pack_and_unpack_round_trip) {
    for (int f = 0; f < SQUARE_NB; ++f) {
        for (int t = 0; t < SQUARE_NB; ++t) {
            for (int flags = 0; flags < 16; ++flags) {
                const Move m(static_cast<Square>(f), static_cast<Square>(t), flags);
                CHECK_EQ(m.from(), static_cast<Square>(f));
                CHECK_EQ(m.to(), static_cast<Square>(t));
                CHECK_EQ(m.flags(), flags);
            }
        }
    }
}

TEST(flag_predicates) {
    CHECK_EQ(describe(Move(E2, E3, FLAG_QUIET)), std::string(""));
    CHECK_EQ(describe(Move(E2, E4, FLAG_DOUBLE_PUSH)), std::string("D"));
    CHECK_EQ(describe(Move(E1, G1, FLAG_KING_CASTLE)), std::string("K"));
    CHECK_EQ(describe(Move(E1, C1, FLAG_QUEEN_CASTLE)), std::string("K"));
    CHECK_EQ(describe(Move(E4, D5, FLAG_CAPTURE)), std::string("C"));
    CHECK_EQ(describe(Move(E5, D6, FLAG_EN_PASSANT)), std::string("CE"));
    CHECK_EQ(describe(Move(E7, E8, promotionFlag(QUEEN, false))), std::string("P"));
    CHECK_EQ(describe(Move(B7, A8, promotionFlag(KNIGHT, true))), std::string("CP"));
}

TEST(promotion_flags) {
    CHECK_EQ(promotionFlag(KNIGHT, false), 8);
    CHECK_EQ(promotionFlag(BISHOP, false), 9);
    CHECK_EQ(promotionFlag(ROOK, false), 10);
    CHECK_EQ(promotionFlag(QUEEN, false), 11);
    CHECK_EQ(promotionFlag(KNIGHT, true), 12);
    CHECK_EQ(promotionFlag(QUEEN, true), 15);
    for (int pt = KNIGHT; pt <= QUEEN; ++pt) {
        for (int capture = 0; capture < 2; ++capture) {
            const Move m(A7, A8, promotionFlag(static_cast<PieceType>(pt), capture != 0));
            CHECK_EQ(m.promotionType(), static_cast<PieceType>(pt));
            CHECK_EQ(m.isCapture(), capture != 0);
        }
    }
}

TEST(null_move) {
    CHECK(NO_MOVE.isNull());
    CHECK_EQ(NO_MOVE.raw(), static_cast<std::uint16_t>(0));
    CHECK(Move() == NO_MOVE);
    CHECK_EQ(moveToUci(NO_MOVE), std::string("0000"));
    CHECK(!Move(A1, B1, FLAG_QUIET).isNull());
    CHECK(Move(E2, E4, FLAG_DOUBLE_PUSH) != Move(E2, E4, FLAG_QUIET));
    CHECK(Move(E2, E4, FLAG_QUIET) == Move(E2, E4, FLAG_QUIET));
}

TEST(uci_text) {
    CHECK_EQ(moveToUci(Move(E2, E4, FLAG_DOUBLE_PUSH)), std::string("e2e4"));
    CHECK_EQ(moveToUci(Move(E1, G1, FLAG_KING_CASTLE)), std::string("e1g1"));
    CHECK_EQ(moveToUci(Move(E1, C1, FLAG_QUEEN_CASTLE)), std::string("e1c1"));
    CHECK_EQ(moveToUci(Move(E5, D6, FLAG_EN_PASSANT)), std::string("e5d6"));
    CHECK_EQ(moveToUci(Move(E7, E8, promotionFlag(QUEEN, false))), std::string("e7e8q"));
    CHECK_EQ(moveToUci(Move(B7, A8, promotionFlag(KNIGHT, true))), std::string("b7a8n"));
    CHECK_EQ(moveToUci(Move(H7, H8, promotionFlag(BISHOP, false))), std::string("h7h8b"));
    CHECK_EQ(moveToUci(Move(A2, A1, promotionFlag(ROOK, false))), std::string("a2a1r"));
}

TEST(uci_parse) {
    const Move plain = moveFromUci("e2e4");
    CHECK_EQ(plain.from(), E2);
    CHECK_EQ(plain.to(), E4);
    CHECK(!plain.isPromotion());

    const Move promo = moveFromUci("e7e8q");
    CHECK_EQ(promo.from(), E7);
    CHECK_EQ(promo.to(), E8);
    CHECK(promo.isPromotion());
    CHECK_EQ(promo.promotionType(), QUEEN);

    // Every pair of different squares survives a text round trip.
    for (int f = 0; f < SQUARE_NB; ++f) {
        for (int t = 0; t < SQUARE_NB; ++t) {
            if (f == t) continue;
            const std::string text = squareToString(static_cast<Square>(f)) + squareToString(static_cast<Square>(t));
            const Move m = moveFromUci(text);
            CHECK_EQ(m.from(), static_cast<Square>(f));
            CHECK_EQ(m.to(), static_cast<Square>(t));
            CHECK_EQ(moveToUci(m), text);
        }
    }

    const char letters[4] = {'n', 'b', 'r', 'q'};
    for (int i = 0; i < 4; ++i) {
        const std::string text = std::string("a7a8") + letters[i];
        const Move m = moveFromUci(text);
        CHECK_EQ(m.promotionType(), static_cast<PieceType>(KNIGHT + i));
        CHECK_EQ(moveToUci(m), text);
    }
}

TEST(uci_parse_rejects_bad_input) {
    // Bad input must never crash the engine, so every malformed text maps to NO_MOVE.
    const char* bad[] = {"",      "e2",    "e2e",    "e2e4qq", "e2e9", "i2e4", "e2e4k",
                         "E2E4",  "e2e4Q", "0000",   "e2e2",   "e2 e4", "e2e4 ", "e2e4\n"};
    for (const char* text : bad) {
        CHECK(moveFromUci(text) == NO_MOVE);
    }
}

TEST(same_uci_move_ignores_flags) {
    // A parsed move has no capture or castling flags, yet it must match the generated move.
    CHECK(sameUciMove(moveFromUci("e1g1"), Move(E1, G1, FLAG_KING_CASTLE)));
    CHECK(sameUciMove(moveFromUci("e4d5"), Move(E4, D5, FLAG_CAPTURE)));
    CHECK(sameUciMove(moveFromUci("e2e4"), Move(E2, E4, FLAG_DOUBLE_PUSH)));
    CHECK(sameUciMove(moveFromUci("b7a8q"), Move(B7, A8, promotionFlag(QUEEN, true))));
    // Different squares or a different promotion piece must not match.
    CHECK(!sameUciMove(moveFromUci("b7a8n"), Move(B7, A8, promotionFlag(QUEEN, true))));
    CHECK(!sameUciMove(moveFromUci("e2e4"), Move(E2, E3, FLAG_QUIET)));
    CHECK(!sameUciMove(moveFromUci("d2e4"), Move(E2, E4, FLAG_QUIET)));
    // A promotion needs its piece letter in UCI text, so "b7a8" alone is not a promotion.
    CHECK(!sameUciMove(moveFromUci("b7a8"), Move(B7, A8, promotionFlag(QUEEN, true))));
}

int main() { return minitest::run_all(); }
