FROM --platform=linux/amd64 ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive  
ENV UDEV=1

# Install the packages required by ModusToolbox
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    autoconf \
    build-essential \
    lcov \
    libtool \
    udev \
    binutils \
    make \
    git \
    cmake \
    perl \
    python3 \
    libncurses5 \
    libusb-1.0-0 \
    libxcb-xinerama0 \
    libglib2.0-0 \
    libgl1-mesa-dev \
    sudo \
    python3-pip \
    udev \
    vim

# Environment variables
ARG MTB_VERSION=3.0
ENV HOME=/home
ENV MTB_TOOLS_DIR=${HOME}/ModusToolbox/tools_${MTB_VERSION}

# ModusToolbox is located locally in the repository during build
# as it is not available through wget or a package manager
COPY ModusToolbox/ ${HOME}/ModusToolbox

# Post install scripts required by Modustoolbox
RUN cd ${MTB_TOOLS_DIR}/openocd/udev_rules \
    && sh install_rules.sh 
RUN cd ${MTB_TOOLS_DIR}/driver_media \
    && sh install_rules.sh
RUN cd ${MTB_TOOLS_DIR}/fw-loader/udev_rules \
    && sh install_rules.sh
RUN cd ${MTB_TOOLS_DIR}/modus-shell \
    && sh postinstall
RUN cd ${MTB_TOOLS_DIR} \
    && bash idc_registration-3.0.0.bash

# Add MTB gcc and project-creator tool to path
ENV PATH "${MTB_TOOLS_DIR}/gcc/bin:$PATH:${MTB_TOOLS_DIR}/project-creator"

# Set environment variable required by ModusToolbox application makefiles
ENV CY_TOOLS_PATHS=${MTB_TOOLS_DIR}

CMD ["/bin/bash"]
