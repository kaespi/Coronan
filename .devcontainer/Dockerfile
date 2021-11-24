FROM ubuntu:focal

LABEL org.opencontainers.image.title="Co(ro)nan devcontainer"
LABEL maintainer="Kaspar Giger <kaspar.giger@gmail.com>"
LABEL org.opencontainers.image.licenses="MIT License"

USER root

# install Qt
RUN apt-get update \
 && apt-get -y install --fix-missing \
 && apt-get -qq install -y --no-install-recommends software-properties-common \
 && add-apt-repository ppa:beineri/opt-qt-5.15.2-focal \
 && apt-get update \
 && apt-get -qq install -y --no-install-recommends qt515base qt515charts-no-lgpl qt515tools \
 && apt-get -qq install -y --no-install-recommends libxcb-xinerama0-dev \
 && apt-get -qq install -y --no-install-recommends '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev \
 && apt-get -qq install -y --no-install-recommends perl \
 && apt-get clean && rm -rf /var/lib/apt/lists/*

# install Cmake
RUN apt-get update \
 && apt-get install -y --no-install-recommends wget \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
RUN mkdir /opt/cmake
RUN wget --no-check-certificate https://github.com/Kitware/CMake/releases/download/v3.20.3/cmake-3.20.3-linux-x86_64.sh \
 && sh ./cmake-3.20.3-linux-x86_64.sh --skip-license --prefix=/opt/cmake
# add symlinks to the cmake binaries in /usr/local/bin, e.g. /usr/local/bin/cmake -> /opt/cmake/bin/cmake
RUN for i in /opt/cmake/bin/*;do ln -s $i `echo $i | sed 's/opt/usr/;s/cmake/local/;'`;done

# install development tools/packages:
# lcov and doxygen, GCC's C++ compiler, Ninja build system,
# Python stuff (incl. pip3), Git gdb, a command line editor, 
# bash configuration, SSL stuff (for conan), Make utils
RUN apt-get update \
 && apt-get -qq install -y --no-install-recommends \
    bash-completion \
    ca-certificates \
    clang-tidy-11 \
    curl \
    doxygen \
    g++ \
    gdb \
    git \
    gnupg \
    graphviz \
    lcov \
    libssl-dev \
    make \
    nano \
    ninja-build \
    openssl \
    python3-pip \
    ssh \
    sudo \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# enable HTTPS network traffic
EXPOSE 443

# add user coronan and add offer him sudo rights
RUN useradd -ms /bin/bash coronan \
    && adduser coronan sudo \
    && printf "coronan ALL= NOPASSWD: ALL\\n" >> /etc/sudoers

USER coronan

# prepare $PATH to include the folder where the conan executable later on will be found
ENV PATH=$PATH:/home/coronan/.local/bin
RUN echo 'PATH=$PATH:${HOME}/.local/bin' >> ~/.bashrc

# conan
RUN pip3 install conan==1.37.1 conan-package-tools \
    && conan profile new default --detect

# somehow conan has/had trouble with their certificates. This one here
# is needed to make sure that conan is using the right (and up-to-date)
# public certificate
RUN conan config install https://github.com/conan-io/conanclientcert.git

# cmake-format
RUN pip3 install cmake-format==0.6.13

# gcovr
RUN pip3 install gcovr==4.2

# pre-commit
RUN pip3 install pre-commit==2.13.0
RUN echo 'export PIP_USER=false' >> ~/.bashrc

# beautify the bash prompt
# (credits to https://dominikberner.ch/using-devcontainers-with-cpp)
RUN curl -L https://raw.github.com/git/git/master/contrib/completion/git-prompt.sh > ~/.bash_git && echo "source ~/.bash_git" >> ~/.bashrc
RUN sed -Ei 's/(PS1=.*)(\\\[\\033\[00m\\\]\\\$.*)/\1\\[\\033[01;33m\\]$(__git_ps1)\2/p' ~/.bashrc
# I don't need to know in the bash prompt who I am and on which machine I'm
# working (it's a container anyway and I don't really care)
RUN sed -Ei 's/(.*PS1=.*)\\u.*\\h.*:\s*/\1/' ~/.bashrc

# no need to be notified about "To run a command as administrator (user "root"), use "sudo <command>"."
# by the bash each time a new shell is opened
RUN touch ~/.sudo_as_admin_successful
