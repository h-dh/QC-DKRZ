FROM ubuntu:14.04
MAINTAINER https://github.com/h-dh/QA-DKRZ

# install system packages
RUN apt-get update -y
RUN apt-get install -y wget vim sudo
RUN apt-get install -y libuuid1 uuid-dev

# Add user hdh 
RUN useradd -d /home/hdh -m hdh -G dialout

# cd into home
#WORKDIR /home/hdh
WORKDIR /root

# Remaining tasks run as user hdh
#USER hdh

# prepare miniconda
RUN wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh;
RUN bash miniconda.sh -b -p $HOME/miniconda
ENV PATH="/root/miniconda/bin:$PATH"
RUN conda config --set always_yes yes --set changeps1 no
RUN conda update -q conda

# add additional conda channels
RUN conda config --add channels birdhouse

# install qa-dkrz
RUN conda install -c birdhouse qa-dkrz cdo

# add mount point for data
VOLUME /data

