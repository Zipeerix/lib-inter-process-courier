FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    wget \
    python3 \
    python3-pip \
    python3-venv \
    pkg-config \
    ninja-build \
    ccache \
    gcc-14 \
    g++-14 \
    sudo \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100

RUN groupadd -g 109 jenkins && \
    useradd -u 109 -g jenkins -m -s /bin/bash jenkins

RUN mkdir /var/lib/jenkins

RUN chown -R jenkins:jenkins /var/lib/jenkins

USER jenkins
ENV HOME=/var/lib/jenkins
ENV CONAN_HOME=/var/lib/jenkins/.conan2
ENV PATH="/var/lib/jenkins/.local/bin:${PATH}"

WORKDIR /var/lib/jenkins

RUN pip3 install --break-system-packages conan
RUN conan profile detect --force