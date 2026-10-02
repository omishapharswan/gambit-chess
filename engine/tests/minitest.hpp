// Purpose: minimal self-written unit-test framework (no third-party code, no downloads).
// Usage: write TEST(name) { CHECK(...); } in a test file, then return minitest::run_all() from main.
#ifndef GAMBIT_MINITEST_HPP
#define GAMBIT_MINITEST_HPP

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace minitest {

struct TestCase {
    const char* name;
    void (*fn)();
};

// Function-local static: TEST() objects are constructed before main(), and this guarantees
// the vector already exists when the first one registers.
inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

// Failures of the test that is running now; reset before each test.
inline int& current_failures() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

// A failed check does not stop the test, so one run reports every problem at once.
inline void report_failure(const char* file, int line, const std::string& message) {
    ++current_failures();
    // endl flushes, so the message survives even if a later test crashes the process.
    std::cerr << "  " << file << ":" << line << ": " << message << std::endl;
}

template <typename A, typename B>
void check_equal(const A& actual, const B& expected, const char* actual_text,
                 const char* expected_text, const char* file, int line) {
    if (actual == expected) return;
    std::ostringstream out;
    out << "CHECK_EQ(" << actual_text << ", " << expected_text << ") failed: " << actual
        << " != " << expected;
    report_failure(file, line, out.str());
}

// The return value is the process exit code, which is how CTest sees pass or fail.
inline int run_all() {
    int failed = 0;
    for (const TestCase& test : registry()) {
        current_failures() = 0;
        std::cout << "[ RUN  ] " << test.name << std::endl;
        try {
            test.fn();
        } catch (const std::exception& e) {
            ++current_failures();
            std::cerr << "  uncaught exception: " << e.what() << std::endl;
        } catch (...) {
            ++current_failures();
            std::cerr << "  uncaught non-standard exception" << std::endl;
        }
        if (current_failures() == 0) {
            std::cout << "[  OK  ] " << test.name << std::endl;
        } else {
            std::cout << "[ FAIL ] " << test.name << std::endl;
            ++failed;
        }
    }
    std::cout << registry().size() << " tests, " << failed << " failed" << std::endl;
    return failed == 0 ? 0 : 1;
}

}  // namespace minitest

// Declares the test function, registers it before main(), then opens its body.
#define TEST(name)                                                                      \
    static void minitest_case_##name();                                                 \
    static const minitest::Registrar minitest_registrar_##name(#name, &minitest_case_##name); \
    static void minitest_case_##name()

#define CHECK(condition)                                                                \
    do {                                                                                \
        if (!(condition))                                                               \
            minitest::report_failure(__FILE__, __LINE__, "CHECK(" #condition ") failed"); \
    } while (0)

#define CHECK_EQ(actual, expected) \
    minitest::check_equal((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#endif  // GAMBIT_MINITEST_HPP
