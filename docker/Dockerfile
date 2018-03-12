FROM ubuntu:16.04

RUN cd root \
    && mkdir coflowSim_online

RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list

RUN apt-get update \  
    && apt-get install -y cmake g++ zlib1g-dev \
    # added lines
    && rm -rf /var/lib/apt/lists/*