# Gambit: Shared Specification

This is the contract all three phases follow. Phase 1 builds the skeleton and tooling, Phase 2 the C++ engine, Phase 3 the UI, tools, docs and release.

## 1. Folder structure

    gambit-chess/
      README.md
      LICENSE                      (MIT)
      CMakeLists.txt               (top level, adds engine/)
      Makefile                     (thin wrapper: build test perft run ui docker clean help)
      Dockerfile                   (multi-stage; builds and tests engine; slim non-root runtime)
      .dockerignore  .gitignore  .clang-format
      .github/workflows/ci.yml
      PROGRESS.md                  (handoff notes between phases)
      docs/  SPEC.md ARCHITECTURE.md MATH.md EXPLAINED.md DESIGN.md AI_USAGE.md images/
      engine/
        CMakeLists.txt
        include/gambit/  types.hpp bitboard.hpp attacks.hpp zobrist.hpp move.hpp
                         position.hpp movegen.hpp eval.hpp tt.hpp search.hpp uci.hpp
        src/             one .cpp per header, plus main.cpp
        tests/           minitest.hpp test_bitboard.cpp test_position.cpp
                         test_movegen_perft.cpp test_zobrist.cpp test_search.cpp
      ui/
        requirements.txt
        gambit_ui/  __init__.py __main__.py app.py theme.py engine_client.py
                    game_state.py board_view.py panels.py pieces.py
                    assets/pieces/  assets/fonts/  (both optional)
        tests/  test_engine_client.py test_game_state.py
      tools/  selfplay.py elo.py perft_check.py

## 2. Engine rules (C++17, namespace gambit, zero third-party libraries)

- Bitboard = uint64_t. Squares 0..63 with A1=0 and H8=63.
- Move = 16-bit packed (from, to, flags). Convert to and from UCI strings.
- Position: 12 piece bitboards plus occupancy, side to move, castling rights, en-passant square, halfmove and fullmove counters, incremental Zobrist key, history stack for make/unmake and repetition detection. FEN parse and write.
- Move generation: fully legal moves (castling, en passant, promotions). Sliding pieces via magic bitboards (hyperbola quintessence is acceptable if simpler). Correctness before speed.
- Draws: threefold repetition, fifty-move rule, insufficient material.
- Evaluation: centipawns from the side-to-move view; material plus tapered middlegame/endgame piece-square tables, bishop pair, basic pawn structure.
- Search: iterative deepening, negamax with alpha-beta, quiescence search, transposition table, move ordering (TT move, MVV-LVA, killer moves, history heuristic), check extension, mate-distance scoring, time management (movetime, wtime/btime/winc/binc, depth). Optional stretch: null-move pruning and late move reductions.
- Transposition table: power-of-two array of entries (key, depth, score, bound, best move), size set by the Hash option in MB, replacement scheme documented.
- Build: Release by default, -Wall -Wextra -Wpedantic with zero warnings, option GAMBIT_SANITIZE=ON for ASan/UBSan.
- Tests: tiny self-written test framework (engine/tests/minitest.hpp), each test file is its own executable registered with CTest. No downloads.

## 3. Perft oracle (the move generator must match exactly)

- `startpos`: depth 1-5: 20, 400, 8902, 197281, 4865609
- `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`: depth 1-4: 48, 2039, 97862, 4085603
- `8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1`: depth 1-5: 14, 191, 2812, 43238, 674624
- `r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1`: depth 1-4: 6, 264, 9467, 422333
- `rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8`: depth 1-4: 44, 1486, 62379, 2103487
- `r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10`: depth 1-4: 46, 2079, 89890, 3894594

If a number will not match after the generator is believed correct, cross-check it against the Chess Programming Wiki "Perft Results" page before assuming a bug.

## 4. Engine <-> UI contract (UCI over stdin/stdout, flush after every line)

Standard commands: `uci`, `isready`, `ucinewgame`, `position [startpos|fen ...] [moves ...]`, `go [depth N | movetime MS | wtime/btime/winc/binc | infinite]`, `stop`, `quit`, `setoption name Hash value N`.

During search print: `info depth D score cp X` (or `score mate N`) `nodes N nps N time MS pv <moves>`. Finish with: `bestmove <move>`.

Extensions (the UI contains NO chess rules, it asks the engine):

- `perft N`: per-root-move counts, then `nodes N`
- `legalmoves`: one line, `legalmoves e2e4:e4 g1f3:Nf3 e7e8q:e8=Q ...` (uci:san)
- `status`: one line, `status <ongoing|checkmate|stalemate|draw_repetition|draw_fifty|draw_material> <w|b> <incheck 0|1>`
- `d`: one line, `fen <current FEN>`
- `bench`: fixed positions at fixed depth, prints total nodes and nps

Bad input must never crash the engine.

Engine binary lookup order for tools and UI: env `GAMBIT_ENGINE`, then `build/engine/gambit(.exe)`, then `build/engine/Release/gambit.exe`, then PATH.

## 5. UI spec (Python 3.10+, pygame)

- Window 1100x720, minimum 1000x680. Board on the left with a slim vertical eval bar beside it. Right panel (about 340 px): wordmark "GAMBIT" in a serif face, engine readout (depth, nodes, nps, score, principal variation), two-column scrollable move list in SAN, then a control row.
- Features: click-to-move and drag-and-drop; legal-move dots; capture rings; last-move highlight; king-in-check highlight; promotion picker; flip board; new game; undo (takes back a full move pair); play as White or Black; 5 difficulty levels (Beginner depth 1, Casual depth 2, Club depth 4, Expert 1 s, Master 3 s); game-over banner; shortcuts N / U / F / Esc.
- The engine thinks on a background thread. The UI never freezes and the engine subprocess is shut down cleanly on exit.

## 6. Design system ("wooden study", must NOT look AI-generated)

| Role | Hex |
|---|---|
| background | #1E1A16 |
| panel | #2A241E |
| panel border (1px) | #3A3229 |
| light square | #E8D5B0 |
| dark square | #A67C52 |
| brass accent | #D9A441 |
| text | #F1E8D8 |
| muted text | #A89B86 |
| last-move | #C9B458 |
| danger | #B5483A |
| eval bar light | #E8D5B0 |
| eval bar dark | #14110E |

- Spacing scale 4/8/12/16/24/32. Corner radius 2px max. Flat colors only.
- Forbidden: gradients, glow, glassmorphism, drop shadows, emoji icons, pure #000 or #FFF, centered "hero" layouts, generic rounded cards.
- Typography: serif for the wordmark and headings (Georgia/Cambria fallback list), clean sans for UI and numbers (Segoe UI/Helvetica/Arial fallback list), small uppercase letter-spaced labels at 11 px, right-aligned numbers. Hover = color shift, not animation.
- Pieces: load assets/pieces/{w|b}{K,Q,R,B,N,P}.png if present, else use a built-in vector renderer drawn in palette colors. Do not download assets from unknown sources; log any third-party asset license in assets/LICENSES.md.

## 7. Tools

- `selfplay.py`: plays N games between two engine settings from a small opening set, reports W/D/L, Elo difference and a 95% error margin.
- `elo.py`: score -> Elo (logistic model) and confidence interval functions.
- `perft_check.py`: runs the perft oracle against the built binary, exits non-zero on any mismatch.

## 8. Git rules

- main always builds and passes tests.
- Short-lived branches feat/<topic>, merged into main with --no-ff, then deleted.
- Conventional Commits (feat:, fix:, test:, docs:, build:, ci:, chore:), imperative, 72 chars max, one logical change per commit.
- Never commit build outputs, venvs, __pycache__, or secrets. Never force-push main. Never store tokens in files.
