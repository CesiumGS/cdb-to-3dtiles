FROM ubuntu:20.04

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -yq install \
    build-essential \
    libgdal-dev \
    libpng-dev \
    libjpeg-dev \
    zlib1g-dev \
    cmake \
    make \
    libgl1-mesa-dev

WORKDIR /var/app

ENTRYPOINT ["/bin/bash", "-l", "-c"]