FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cmake --build . -j

CMD ["bash"]
