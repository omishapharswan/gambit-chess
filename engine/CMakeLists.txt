set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Extensions off keeps the code on standard C++17 instead of GNU dialects.
set(CMAKE_CXX_EXTENSIONS OFF)

option(GAMBIT_SANITIZE "Build with AddressSanitizer and UBSan (GCC/Clang only)" OFF)

# One interface target carries the warning and sanitizer flags, so the library,
# the binary and every test are built with identical settings.
add_library(gambit_options INTERFACE)
if(MSVC)
  target_compile_options(gambit_options INTERFACE /W4 /permissive-)
  if(GAMBIT_SANITIZE)
    message(WARNING "GAMBIT_SANITIZE needs GCC or Clang; ignoring it on MSVC")
  endif()
else()
  target_compile_options(gambit_options INTERFACE -Wall -Wextra -Wpedantic)
  if(GAMBIT_SANITIZE)
    target_compile_options(gambit_options INTERFACE
      -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(gambit_options INTERFACE -fsanitize=address,undefined)
  endif()
endif()

# Everything except main.cpp lives in a library so tests link the real code.
add_library(gambit_core STATIC
  src/types.cpp
  src/bitboard.cpp
  src/attacks.cpp
  src/zobrist.cpp
  src/move.cpp
  src/position.cpp
  src/movegen.cpp
  src/eval.cpp
  src/tt.cpp
  src/search.cpp
  src/uci.cpp)
target_include_directories(gambit_core PUBLIC include)
target_link_libraries(gambit_core PUBLIC gambit_options)

add_executable(gambit src/main.cpp)
target_link_libraries(gambit PRIVATE gambit_core)
target_compile_definitions(gambit PRIVATE GAMBIT_VERSION="${PROJECT_VERSION}")

# Each test file is its own executable so one crash cannot hide the others.
foreach(name bitboard position movegen_perft zobrist search)
  add_executable(test_${name} tests/test_${name}.cpp)
  target_link_libraries(test_${name} PRIVATE gambit_core)
  add_test(NAME ${name} COMMAND test_${name})
  set_tests_properties(${name} PROPERTIES TIMEOUT 300)
endforeach()
