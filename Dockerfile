FROM centos:6
MAINTAINER https://github.com/h-dh/QA-DKRZ

# install system packages
RUN yum update -y
RUN yum install -y wget tar bzip2
RUN yum install -y libuuid libuuid-devel

# Add user hdh 
RUN useradd -d /home/hdh -m hdh

# cd into home
WORKDIR /home/hdh

# Remaining tasks run as user hdh
USER hdh

# prepare miniconda
RUN wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh;
RUN bash miniconda.sh -b -p $HOME/miniconda
ENV PATH="/home/hdh/miniconda/bin:$PATH"
#RUN conda config --set always_yes yes --set changeps1 no
RUN conda update -y -q conda

# add additional conda channels
RUN conda config --add channels birdhouse

# install qa-dkrz
RUN conda install -y -c birdhouse qa-dkrz cdo

# add mount point for data
VOLUME /data

CMD ["/bin/bash"]

