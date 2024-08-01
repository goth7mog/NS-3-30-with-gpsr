FROM ubuntu:16.04

RUN apt-get update



RUN apt-get install -y git
RUN apt-get install -y mercurial
RUN apt-get install -y gcc
RUN apt-get install -y g++
RUN apt-get install -y vim
RUN apt-get install -y python
RUN apt-get install -y python-dev
RUN apt-get install -y python-setuptools
RUN apt-get install -y qt5-default
RUN apt-get install -y python-pygraphviz
RUN apt-get install -y python-kiwi
RUN apt-get install -y python-pygoocanvas
RUN apt-get install -y libgoocanvas-dev
RUN apt-get install -y ipython
RUN apt-get install -y autoconf
RUN apt-get install -y cvs
RUN apt-get install -y bzr
RUN apt-get install -y unrar
RUN apt-get install -y gdb
RUN apt-get install -y valgrind
RUN apt-get install -y uncrustify
RUN apt-get install -y flex
RUN apt-get install -y bison
RUN apt-get install -y libfl-dev
RUN apt-get install -y tcpdump
RUN apt-get install -y gsl-bin
RUN apt-get install -y libgsl2
RUN apt-get install -y libgsl-dev
RUN apt-get install -y sqlite
RUN apt-get install -y sqlite3
RUN apt-get install -y libsqlite3-dev
RUN apt-get install -y libxml2
RUN apt-get install -y libxml2-dev
RUN apt-get install -y cmake
RUN apt-get install -y libc6-dev
# RUN apt-get install -y libc6-dev-i386
RUN apt-get install -y libclang-dev
RUN apt-get install -y llvm-dev
RUN apt-get install -y automake
RUN apt-get install -y libgtk2.0-0
RUN apt-get install -y libgtk2.0-dev
RUN apt-get install -y vtun
RUN apt-get install -y lxc
RUN apt-get install -y libboost-signals-dev
RUN apt-get install -y libboost-filesystem-dev

# RUN apt-get install nano



# RUN mkdir -p /usr/ns3
WORKDIR /usr 

RUN wget https://www.nsnam.org/release/ns-allinone-3.30.tar.bz2  && \
    tar -jxvf ns-allinone-3.30.tar.bz2

# RUN cd ns-allinone-3.30/ns-3.30/src && git clone https://gitlab.ibr.cs.tu-bs.de/tschuber/ns-3-leo.git && mv ns-3-leo leo


RUN cd ns-allinone-3.30/ns-3.30/src && mkdir leo
COPY ns3-leo-module ns-allinone-3.30/ns-3.30/src/leo

RUN cd ns-allinone-3.30/ns-3.30/src && mkdir gpsr
COPY gpsr ns-allinone-3.30/ns-3.30/src/gpsr

RUN cd ns-allinone-3.30/ns-3.30/src && mkdir location-service
COPY location-service ns-allinone-3.30/ns-3.30/src/location-service



RUN cd ns-allinone-3.30/ns-3.30 && ./waf configure --enable-examples --enable-tests

RUN cd ns-allinone-3.30/ns-3.30 && ./waf

RUN cd ns-allinone-3.30/ns-3.30/contrib && mkdir -p leo/data

COPY ns3-leo-module/data ns-allinone-3.30/ns-3.30/contrib/leo/data





RUN apt-get clean && \
    rm -rf /var/lib/apt && \
    rm ns-allinone-3.30.tar.bz2