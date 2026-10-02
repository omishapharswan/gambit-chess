# Thin wrapper around CMake, Python and Docker: every workflow is one short command.
BUILD_DIR ?= build
CONFIG    ?= Release
PYTHON    ?= python
# Recursive "=" so the lookup runs after the build, when the binary exists.
ENGINE     = $(firstword $(wildcard $(BUILD_DIR)/engine/gambit $(BUILD_DIR)/engine/gambit.exe $(BUILD_DIR)/engine/$(CONFIG)/gambit.exe))

.PHONY: build test perft run ui docker clean help

help:
	@echo build  - configure and compile the engine
	@echo test   - run all C++ tests through CTest
	@echo perft  - check move generation against the perft oracle
	@echo run    - start the engine in UCI mode
	@echo ui     - start the pygame desktop app
	@echo docker - build the Docker image
	@echo clean  - delete the build directory

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG)
	cmake --build $(BUILD_DIR) --config $(CONFIG)

test: build
	cd $(BUILD_DIR) && ctest -C $(CONFIG) --output-on-failure

perft: build
	$(PYTHON) tools/perft_check.py

run: build
	$(ENGINE)

ui: build
	cd ui && $(PYTHON) -m gambit_ui

docker:
	docker build -t gambit-chess .

clean:
	cmake -E remove_directory $(BUILD_DIR)
