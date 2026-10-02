#include "gambit/types.hpp"

namespace gambit {

std::string squareToString(Square s) {
    if (s < A1 || s >= SQUARE_NB) {
        return "-";
    }
    return std::string{static_cast<char>('a' + fileOf(s)), static_cast<char>('1' + rankOf(s))};
}

Square squareFromString(const std::string& text) {
    if (text.size() != 2) {
        return SQUARE_NONE;
    }
    const int file = text[0] - 'a';
    const int rank = text[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return SQUARE_NONE;
    }
    return makeSquare(file, rank);
}

}  // namespace gambit
