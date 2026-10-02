# Gambit

A chess engine written from scratch in C++17, with a playable pygame desktop app.

Status: project skeleton. The engine and the app are not implemented yet; the plan is in [docs/SPEC.md](docs/SPEC.md).

## Quick start

### Windows (CMake and MinGW)

    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    cd build
    ctest --output-on-failure

### Make (Linux, macOS)

    make build
    make test

### Docker

    docker build -t gambit-chess .
    docker run -i gambit-chess

## Sections added with the engine and the app

Features, architecture diagram, perft table, benchmark numbers, Elo result and folder tour.

## License

MIT, see [LICENSE](LICENSE).
