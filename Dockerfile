FROM ubuntu:rolling

RUN apt update; apt upgrade -y
RUN apt install -y build-essential g++-14 cmake ninja-build python3

WORKDIR /fastnx
COPY . .

RUN mkdir build; cd build; cmake .. -DCMAKE_BUILD_TYPE=Debug -Wno-dev -GNinja; ninja