# syntax=docker/dockerfile:1

# Stage 1: build and test. The toolchain stays here and never reaches the final image.
# Both stages use the same base so the runtime has the exact libstdc++ the binary was built against.
FROM debian:bookworm-slim AS build
RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ cmake make \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY engine ./engine
# Running ctest inside the build means a broken engine can never produce an image.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)" \
 && cd build && ctest --output-on-failure

# Stage 2: slim runtime with only the binary, running as an unprivileged user.
FROM debian:bookworm-slim AS runtime
RUN useradd --system --no-create-home --uid 10001 gambit
COPY --from=build /src/build/engine/gambit /usr/local/bin/gambit
USER gambit
# The engine speaks UCI on stdin/stdout, so run it with: docker run -i gambit-chess
ENTRYPOINT ["gambit"]
