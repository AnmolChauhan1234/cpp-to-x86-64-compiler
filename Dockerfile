FROM ubuntu:22.04

# Install build essentials, cmake, and other deps
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    nasm \
    clang \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside container
WORKDIR /app

# Copy everything from your current dir into /app in container
COPY . /app

# Default command: start bash so you can interact
CMD ["/bin/bash"]
