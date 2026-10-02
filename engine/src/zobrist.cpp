#include "gambit/zobrist.hpp"

namespace gambit {

namespace detail {
Key pieceSquareKeys[PIECE_NB][SQUARE_NB];
Key sideKey;
Key castlingKeys[ALL_CASTLING + 1];
Key enPassantFileKeys[8];
}  // namespace detail

namespace {

// SplitMix64: a tiny generator whose output looks random for any seed, including small
// ones. Unsigned overflow wraps around by definition, which the algorithm relies on.
class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) : state_(seed) {}

    std::uint64_t next() {
        state_ += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state_;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

private:
    std::uint64_t state_;
};

// Arbitrary but fixed. The same seed on every run and every platform makes hashes
// repeatable, so a bug found once can be reproduced and tests can rely on stable keys.
constexpr std::uint64_t SEED = 0x6A09E667F3BCC908ULL;

struct KeyFiller {
    KeyFiller() {
        SplitMix64 rng(SEED);
        for (int p = 0; p < PIECE_NB; ++p) {
            for (int s = 0; s < SQUARE_NB; ++s) {
                detail::pieceSquareKeys[p][s] = rng.next();
            }
        }
        detail::sideKey = rng.next();

        // One key per single right, then every mask is the XOR of its rights.
        Key rightKeys[4];
        for (Key& key : rightKeys) {
            key = rng.next();
        }
        for (int mask = 0; mask <= ALL_CASTLING; ++mask) {
            Key combined = 0;
            for (int bit = 0; bit < 4; ++bit) {
                if ((mask & (1 << bit)) != 0) {
                    combined ^= rightKeys[bit];
                }
            }
            detail::castlingKeys[mask] = combined;
        }

        for (Key& key : detail::enPassantFileKeys) {
            key = rng.next();
        }
    }
};

// A global object runs its constructor before main(), so no caller can forget an init
// call. The rule that follows: no other global initializer may read these keys.
const KeyFiller keyFiller;

}  // namespace
}  // namespace gambit
