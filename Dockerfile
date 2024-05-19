FROM debian:latest

RUN apt update
RUN apt install -y -V ca-certificates lsb-release wget
RUN wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
RUN apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
RUN apt update

SHELL ["/bin/bash", "-c"]
RUN apt-get update && apt install -y -V libarrow-dev
RUN apt-get install -y -V libarrow-flight-dev
RUN apt-get install -y -V cmake
RUN apt-get install -y -V libgflags-dev
RUN apt-get install -y -V python3
RUN apt-get install -y -V python3-pip
RUN apt-get install -y -V python3.11-venv
RUN python3 -m venv /venv 
RUN source /venv/bin/activate && python -m pip install pyarrow

RUN mkdir /src
COPY main.cpp /src
COPY client.py /src
COPY CMakeLists.txt /src
WORKDIR /src

RUN echo "Hello World"