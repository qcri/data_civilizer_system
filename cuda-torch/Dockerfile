
FROM docker.io/ubuntu:18.04

# Note: Package 'gnupg' was required for the next step...
#
# Size: 84.6 MB
# Size: 15.2 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        gnupg \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Size: 1.21 kB
RUN NVIDIA_GPGKEY_SUM=d1be581509378368edeec8c1eb2958702feedf3bc3d17011adbf24efacce4ab5 && \
    NVIDIA_GPGKEY_FPR=ae09fe4bbd223a84b2ccfce3f60f4b3d7fa2af80 && \
    apt-key adv --fetch-keys http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1604/x86_64/7fa2af80.pub && \
    apt-key adv --export --no-emit-version -a $NVIDIA_GPGKEY_FPR | sed -n '/^--/,/^--/ p' > cudasign.pub && \
    echo "$NVIDIA_GPGKEY_SUM cudasign.pub" | sha256sum -c --strict - && \
    rm cudasign.pub && \
    echo "deb http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1604/x86_64 /" > /etc/apt/sources.list.d/cuda.list

ENV CUDA_VERSION=8.0.61

LABEL com.nvidia.cuda.version=8.0.61

ENV CUDA_PKG_VERSION=8-0=8.0.61-1

# Size: 690 MB
# Size: 661 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        cuda-nvrtc-$CUDA_PKG_VERSION \
        cuda-nvgraph-$CUDA_PKG_VERSION \
        cuda-cusolver-$CUDA_PKG_VERSION \
        cuda-cublas-8-0=8.0.61.2-1 \
        cuda-cufft-$CUDA_PKG_VERSION \
        cuda-curand-$CUDA_PKG_VERSION \
        cuda-cusparse-$CUDA_PKG_VERSION \
        cuda-npp-$CUDA_PKG_VERSION \
        cuda-cudart-$CUDA_PKG_VERSION \
    && \
    ln -s cuda-8.0 /usr/local/cuda && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Size: 22.9 kB
RUN echo "/usr/local/cuda/lib64" >> /etc/ld.so.conf.d/cuda.conf && \
    ldconfig

# Size: 46 B
RUN echo "/usr/local/nvidia/lib" >> /etc/ld.so.conf.d/nvidia.conf && \
    echo "/usr/local/nvidia/lib64" >> /etc/ld.so.conf.d/nvidia.conf

ENV PATH=/usr/local/nvidia/bin:/usr/local/cuda/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

ENV LD_LIBRARY_PATH=/usr/local/nvidia/lib:/usr/local/nvidia/lib64

# Size: 943 MB
# Size: 914 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        cuda-core-$CUDA_PKG_VERSION \
        cuda-misc-headers-$CUDA_PKG_VERSION \
        cuda-command-line-tools-$CUDA_PKG_VERSION \
        cuda-nvrtc-dev-$CUDA_PKG_VERSION \
        cuda-nvml-dev-$CUDA_PKG_VERSION \
        cuda-nvgraph-dev-$CUDA_PKG_VERSION \
        cuda-cusolver-dev-$CUDA_PKG_VERSION \
        cuda-cublas-dev-8-0=8.0.61.2-1 \
        cuda-cufft-dev-$CUDA_PKG_VERSION \
        cuda-curand-dev-$CUDA_PKG_VERSION \
        cuda-cusparse-dev-$CUDA_PKG_VERSION \
        cuda-npp-dev-$CUDA_PKG_VERSION \
        cuda-cudart-dev-$CUDA_PKG_VERSION \
        cuda-driver-dev-$CUDA_PKG_VERSION && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV LIBRARY_PATH=/usr/local/cuda/lib64/stubs:

# Size: 92 B
RUN echo "deb http://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1604/x86_64 /" > /etc/apt/sources.list.d/nvidia-ml.list

ENV CUDNN_VERSION=5.1.10

LABEL com.nvidia.cudnn.version=5.1.10

# Size: 185 MB
# Size: 156 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libcudnn5=$CUDNN_VERSION-1+cuda8.0 \
        libcudnn5-dev=$CUDNN_VERSION-1+cuda8.0 \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Note: The 'install-deps' script in the next step will detect and install most of
#       the required dependency packages for torch.  However, 'git' is required
#       before running that script, 'software-properties-common' installs the root
#       certificates necessary for the git clone to succeed, and sudo is used by
#       the 'install-deps' script but not installed by it.
# Size: 952 MB
# Size: 779 MB - added --no-install-recommeds
# Size: 83.5 MB - removed all non-essential packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git \
        software-properties-common \
        sudo \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
#        nano \
#        wget \
#        build-essential \
#        cmake \
#        curl \
#        gfortran \
#        libatlas-base-dev \
#        libavcodec-dev \
#        libavformat-dev \
#        libboost-all-dev \
#        libgtk2.0-dev \
#        libjpeg-dev \
#        liblapack-dev \
#        libswscale-dev \
#        pkg-config \
#        python-dev \
#        python-pip \
#        graphicsmagick \
#        libgraphicsmagick1-dev \
#        python-numpy zip \

# Note: Package 'python-software-properties' was trying to be installed by
#       'install-deps', but has been deprecated and is replaced by
#       'software-properties-common', which was installed in the previous step.
# 
# Size: 370 MB
# Size: 299 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
# Size: 417 MB - adding --no-install-recommends above increased the number of
#                packages installed during 'install-deps' but resulted in a
#                net reduction overall
# Size: 714 MB - removing almost all packages above further increased detected
#                dependencies, but still resulted in further net savings
RUN git clone https://github.com/torch/distro.git /root/torch --recursive && \
    cd /root/torch && \
    sed -i '/python-software-properties/ s/^/#/' install-deps && \
    bash install-deps && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Note: The default version of gcc was 7.2, but './install.sh' complained that
#       gcc newer than 5 was not supported.  Links to resolve this issue
#       suggested installing gcc 4.9, but that version always returned
#       package not found.  Version 4.8 installed successfully and compiled.
#
# Size: 93.1 MB
# Size: 62.6 MB - apt-get clean; rm -rf /var/lib/apt/lists/*
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        gcc-4.8 \
        g++-4.8 \
    && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Size: 3.78 GB
# Size: 1.25 GB - adding modified clean.sh
RUN cd /root/torch && \
    ./install.sh && \
    sed -r 's/-o -name install |exe\/|jit-//g' ./clean.sh | bash

# Size: 258 B
RUN ln -s /root/torch/install/bin/* /usr/local/bin

# Note: Luarocks packages added significantly to the container size, so only
#       requierd packages showed be installed.  Since additional luarocks
#       packages are installed by the data-civilizer-api Dockerfile, all
#       necessary luarocks packages should be installed at that time.
#
# Size: 1.27 GB
#RUN luarocks install cutorch && \
#    luarocks install cunn && \
#    luarocks install cudnn

WORKDIR /root
