FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

SHELL ["/bin/bash", "-c"]

RUN apt-get update  && apt-get upgrade -y \
    build-essential \
    cmake \
    gcc-11-plugin-dev \
    vim \
    net-tools

    

