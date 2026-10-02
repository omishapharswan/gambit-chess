#include "gambit/bitboard.hpp"

namespace gambit {

std::string bitboardToString(Bitboard b) {
    std::string out;
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            out += (b & squareBb(makeSquare(file, rank))) != 0 ? '1' : '.';
        }
        out += '\n';
    }
    return out;
}

}  // namespace gambit
