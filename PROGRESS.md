# Progress

Handoff notes between phases. The full contract is in docs/SPEC.md.

## Phase checklist

- [x] Phase 1: skeleton, tooling, Git, GitHub
- [ ] Phase 2: C++ engine
- [ ] Phase 3: UI, tools, docs, release

## Status

Phase 1 is complete. Everything is pushed to main and verified locally, from a fresh clone, in Docker and in GitHub Actions. Next: Phase 2.

## Project facts

- Repo: https://github.com/omishapharswan/gambit-chess
- Local path: D:\omiii\gambit-chess
- OS: Windows
- Git identity is set repo-local to omishapharswan <omisha5555@gmail.com>. The global Git identity belongs to a different account and must not be changed.
- Toolchain: CMake, GCC 16.2.0 (WinLibs, installed with winget), Python 3.14.7, pytest 9.1.1.

## Conventions

- Namespace `gambit`. Header guards are `GAMBIT_<NAME>_HPP`. Comments explain WHY, not what.
- Tests use engine/tests/minitest.hpp (TEST, CHECK, CHECK_EQ). Each test file is its own executable linked against `gambit_core`.
- A new .cpp file must be added to the `gambit_core` list in engine/CMakeLists.txt.
- Git: branches `feat/<topic>`, merged into main with `--no-ff` and deleted. Conventional Commits, imperative, 72 characters max.
- Before every push: `git log -1 --format='%an <%ae>'` must show the omishapharswan identity, and `git rev-parse HEAD` must equal the SHA from `git ls-remote origin main`.

## Build and run commands

Windows (CMake with MinGW):

    cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    Push-Location build; ctest --output-on-failure; Pop-Location
    .\build\engine\gambit.exe

Make (Linux, macOS, or Windows with mingw32-make and `$env:CMAKE_GENERATOR = "MinGW Makefiles"`):

    make build
    make test
    make run

Docker:

    docker build -t gambit-chess .
    docker run -i gambit-chess

UI and its tests:

    cd ui
    pip install -r requirements.txt
    python -m gambit_ui
    python -m pytest tests

## Phase 1 record

Built:

- Build system: top-level and engine CMakeLists, Makefile.
- Tooling: Dockerfile (multi-stage, non-root runtime), .dockerignore, .gitignore, .clang-format, GitHub Actions workflow (engine on ubuntu, macos and windows; ui tests; docker build), MIT LICENSE file, ui/requirements.txt.
- Stubs: 11 engine headers, 11 engine sources, main.cpp that prints the version, minitest.hpp with a smoke test, Python UI package, UI tests, tools.
- Docs: SPEC.md, this file, a README skeleton, and one-line stubs for ARCHITECTURE, MATH, EXPLAINED, DESIGN and AI_USAGE.

Verified by running:

- Engine build with GCC: zero warnings.
- CTest: 5 of 5 tests passed.
- `gambit.exe` prints `Gambit 1.0.0`.
- UI: pytest 2 passed, `python -m gambit_ui` prints `Gambit UI 1.0.0`, all UI and tools modules import.
- Every push: author is omishapharswan and the local and remote SHAs matched.
- Fresh clone from GitHub into a temp folder: `mingw32-make test` passed 5 of 5, `mingw32-make run` printed `Gambit 1.0.0`, pytest passed 2, `python -m gambit_ui` printed `Gambit UI 1.0.0`.
- Docker 29.7.2: `docker build` exited 0 (ctest runs inside the build), the container printed `Gambit 1.0.0`, the runtime user is uid 10001 (gambit), and the image content size is 28.2 MB.
- GitHub Actions on commit 00a400c: all 5 jobs succeeded (engine on ubuntu, macos and windows; ui tests; docker build).

Deviations from the spec:

- The GitHub repo was created manually and pushed over HTTPS with the remote URL `https://omishapharswan@github.com/omishapharswan/gambit-chess.git`, so Git asks for that account. `gh` was not used.
- Local Windows builds use the MinGW Makefiles generator. The CI workflow uses each runner's default generator.
- The Makefile is written for GNU make. On Windows it is run as `mingw32-make` and needs `CMAKE_GENERATOR` set to `MinGW Makefiles`.
- tools/selfplay.py and tools/perft_check.py exit non-zero on purpose, so `make perft` cannot pass before the real check exists.
- The position, movegen, zobrist and search test files contain no tests yet. They pass because `run_all()` returns 0 when nothing is registered. Phase 2 must add real tests to them.

## Open items

- The Makefile was only run on Windows through mingw32-make. It is untested on Linux and macOS, because CI builds with CMake directly and does not call make.
- The Makefile targets `perft` and `ui` have not been run. `perft` fails on purpose until Phase 3 replaces the stub.
- The LICENSE copyright holder is the GitHub username omishapharswan. Replace it with a real name if wanted.

## Notes for Phase 2

- Only touch engine/ and docs/MATH.md. Do not touch ui/ or tools/.
- main.cpp currently prints the version. Replace it with the UCI loop (GAMBIT_VERSION is injected by CMake from the project version).
- Real performance numbers go in this file: startpos perft 5 time, search depth 6 time, bench nodes and nps.
- The Dockerfile copies only CMakeLists.txt and engine/. The engine must build without any file outside those.
- On Windows a new terminal can lose the compiler and CMake from PATH. WinLibs GCC is under %LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_*\mingw64\bin and CMake is in C:\Program Files\CMake\bin.
- Working style: the author uses Windows PowerShell and wants short paste-ready command blocks, one batch at a time. Each batch ends with commit and push plus the identity and SHA checks above.
