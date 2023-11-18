FROM ubuntu:20.04 AS builder

LABEL maintainer="raoufkh <khichane.araouf@gmail.com>"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get -y upgrade
RUN apt-get -y install make g++ libsctp-dev  libcurl4 libcurl4-openssl-dev
RUN apt-get -y install git python3 python3-pip
RUN pip3 install cmake

WORKDIR /ueransim
RUN git clone https://github.com/Kushan-Nilanga/UERANSIM \
    && cd UERANSIM

WORKDIR /ueransim/UERANSIM

RUN make

FROM ubuntu:20.04

WORKDIR /ueransim

RUN mkdir config build

COPY --from=builder /ueransim/UERANSIM/build/* ./build/
COPY --from=builder /ueransim/UERANSIM/config/* ./config/
