FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        gdb \
        cmake \
        ninja-build \
        clang \
        clang-format \
        clang-tidy \
        cppcheck \
        valgrind \
        pkg-config \
        git \
        ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces/DA
