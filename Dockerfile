FROM ubuntu:noble

ENV TZ="Europe/Berlin"
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt -y update
RUN apt -y install git clang-format vim g++ cmake
RUN apt -y install libsdl2-dev libsdl2-image-dev libsdl2-gfx-dev

RUN apt -y update
RUN apt -y install valgrind

RUN useradd -ms /bin/bash dino_runner