// Unit tests for Zobrist keys: the keys themselves and the XOR rules a position hash relies on.
#include <algorithm>
#include <cstddef>
#include <vector>

#include "gambit/bitboard.hpp"
#include "gambit/types.hpp"
#include "gambit/zobrist.hpp"
#include "minitest.hpp"

using namespace gambit;

namespace {

// Every key a position hash can XOR in. The "no castling right" and "no en-passant
// square" cases are left out because they are defined as 0.
std::vector<Key> allKeys() {
    std::vector<Key> keys;
    for (int p = 0; p < PIECE_NB; ++p) {
        for (int s = 0; s < SQUARE_NB; ++s) {
            keys.push_back(pieceKey(static_cast<Piece>(p), static_cast<Square>(s)));
        }
    }
    keys.push_back(sideToMoveKey());
    for (int rights = 1; rights <= ALL_CASTLING; ++rights) {
        keys.push_back(castlingKey(rights));
    }
    for (int file = 0; file < 8; ++file) {
        keys.push_back(enPassantKey(makeSquare(file, 2)));
    }
    return keys;
}

}  // namespace

TEST(keys_are_nonzero_and_distinct) {
    std::vector<Key> keys = allKeys();
    CHECK_EQ(keys.size(), static_cast<std::size_t>(12 * 64 + 1 + 15 + 8));
    // A zero key would make a feature invisible to the hash.
    CHECK(std::count(keys.begin(), keys.end(), 0ULL) == 0);
    // Two equal keys would make different features cancel each other out.
    std::sort(keys.begin(), keys.end());
    const bool allDistinct = std::adjacent_find(keys.begin(), keys.end()) == keys.end();
    CHECK(allDistinct);
}

TEST(keys_are_well_mixed) {
    // Random 64-bit numbers have 32 set bits on average. The mean over about 800 keys
    // varies by only about 0.15, so a mean outside 31..33 means a broken generator.
    const std::vector<Key> keys = allKeys();
    int total = 0;
    for (const Key key : keys) {
        total += popcount(key);
    }
    const int count = static_cast<int>(keys.size());
    CHECK(total > 31 * count);
    CHECK(total < 33 * count);
}

TEST(incremental_update_matches_hash_from_scratch) {
    // White knight g1 takes a black pawn on f3, then black is to move.
    const Key before = pieceKey(W_KNIGHT, G1) ^ pieceKey(B_PAWN, F3);
    Key key = before;
    key ^= pieceKey(W_KNIGHT, G1);  // knight leaves g1
    key ^= pieceKey(B_PAWN, F3);    // captured pawn disappears
    key ^= pieceKey(W_KNIGHT, F3);  // knight arrives on f3
    key ^= sideToMoveKey();         // turn passes to black
    CHECK_EQ(key, pieceKey(W_KNIGHT, F3) ^ sideToMoveKey());

    // Unmake: XORing the same keys again, in any order, restores the old hash.
    key ^= sideToMoveKey();
    key ^= pieceKey(W_KNIGHT, F3);
    key ^= pieceKey(B_PAWN, F3);
    key ^= pieceKey(W_KNIGHT, G1);
    CHECK_EQ(key, before);
}

TEST(hash_does_not_depend_on_move_order) {
    // Two move orders that reach the same position must give the same hash.
    const Key forward = pieceKey(W_KNIGHT, F3) ^ pieceKey(B_KNIGHT, F6) ^ pieceKey(W_KNIGHT, C3);
    const Key backward = pieceKey(W_KNIGHT, C3) ^ pieceKey(B_KNIGHT, F6) ^ pieceKey(W_KNIGHT, F3);
    CHECK_EQ(forward, backward);
}

TEST(castling_keys_combine_by_xor) {
    CHECK_EQ(castlingKey(0), 0ULL);
    CHECK_EQ(castlingKey(WHITE_OO | WHITE_OOO), castlingKey(WHITE_OO) ^ castlingKey(WHITE_OOO));
    CHECK_EQ(castlingKey(ALL_CASTLING),
             castlingKey(WHITE_OO) ^ castlingKey(WHITE_OOO) ^ castlingKey(BLACK_OO) ^ castlingKey(BLACK_OOO));
    // Losing one right: the single update before ^ after leaves exactly that right's key.
    CHECK_EQ(castlingKey(ALL_CASTLING) ^ castlingKey(ALL_CASTLING & ~WHITE_OO), castlingKey(WHITE_OO));
}

TEST(en_passant_key_uses_file_only) {
    CHECK_EQ(enPassantKey(SQUARE_NONE), 0ULL);
    CHECK_EQ(enPassantKey(E3), enPassantKey(E6));
    CHECK(enPassantKey(E3) != enPassantKey(D3));
    CHECK(enPassantKey(A3) != 0ULL);
}

int main() { return minitest::run_all(); }
