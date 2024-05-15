#
# The line below states we will base our new image on the Latest Official Ubuntu 
FROM ubuntu:latest
#
# Identify the maintainer of an image
LABEL maintainer="shaomeng@ucar.edu"
#
# Update the image to the latest packages
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y git vim cmake-curses-gui build-essential && \
    groupadd -r Odie && useradd -r -g Odie Odie && \
    mkdir -p /home/Odie && cd /home && chown Odie:Odie Odie
#
# Switch user
USER Odie
#
# Clone the repo, and build it
RUN cd /home/Odie && \
    git clone --depth 1 https://github.com/NCAR/SPERR.git SPERR-src && \
    cd SPERR-src && mkdir build && cd build && \
    cmake .. && make -j 6
#
# Update PATH enviornment variable
ENV PATH="$PATH:/home/Odie/SPERR-src/build/bin"
