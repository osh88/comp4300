# docker build -t comp4300_final_project . && docker run --rm -it -v .:/final_project --name comp4300_final_project comp4300_final_project

FROM debian:13-slim

RUN apt-get update
RUN apt-get install -y cmake liblzma-dev wget

WORKDIR /var
RUN wget "https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.2/LLVM-22.1.2-Linux-X64.tar.xz"
RUN apt-get install -y xz-utils

RUN tar -xvJf LLVM-22.1.2-Linux-X64.tar.xz
ENV CXX=/var/LLVM-22.1.2-Linux-X64/bin/clang++

WORKDIR /final_project
ENTRYPOINT /final_project/rebuild.sh
