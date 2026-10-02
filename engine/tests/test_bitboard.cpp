// Tests for gambit/bitboard.hpp. Phase 1 holds only a smoke test that proves the framework
// and the gambit_core link work; the real bitboard tests are added in Phase 2.
#include <string>

#include "gambit/bitboard.hpp"
#include "minitest.hpp"

TEST(framework_smoke) {
    CHECK(1 + 1 == 2);
    CHECK_EQ(std::string("gam") + "bit", std::string("gambit"));
}

int main() {
    return minitest::run_all();
}
