## Dockerfile for GMT Image nc5ng/gmt for 6.0.0 branch
##     (Currently master)
##
## Approach: Perform development build in situ
##
## Construct and delete the build environment.
## This reduces image size at expense of longer build.
## Also fetch DCW/GSSHG data first, this extends layer cache
## between subsequent builds (less to re-download).
##
## Note: Dockerhub automated builds do not cache intermediate layers
##
FROM ubuntu:16.04
LABEL maintainer="akshmakov@nc5ng.org"

## Part 1: Base Image and Run Dependencies

ARG BIN_DEPS="wget libnetcdf11 libgdal1i libfftw3-3 libpcre3 liblapack3 graphicsmagick liblas3"
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update					 &&\
    apt-get install -y					   \
    	    software-properties-common 			   \
	    python-software-properties 			   \
	    $BIN_DEPS					 &&\
    add-apt-repository universe        			 &&\
    apt-get purge -y					   \
            software-properties-common 			   \
	    python-software-properties 			 &&\
    apt-get autoremove -y 				 &&\
    rm -rf /var/lib/apt/lists/*          

## Part 2: GSHHG and DCW Base Data

ARG GSHHG_VERSION=2.3.7
ARG DCW_VERSION=1.1.4
ARG GMT_INSTALL_DIR=/opt/gmt

ENV GMT_DCW_HTTP=http://www.soest.hawaii.edu/pwessel/dcw/dcw-gmt-$DCW_VERSION.tar.gz \
    GMT_GSHHG_HTTP=http://www.soest.hawaii.edu/pwessel/gshhg/gshhg-gmt-$GSHHG_VERSION.tar.gz \
    GMT_DCW_FTP=ftp://ftp.soest.hawaii.edu/dcw/dcw-gmt-$DCW_VERSION.tar.gz \
    GMT_GSHHG_FTP=ftp://ftp.soest.hawaii.edu/gshhg/gshhg-gmt-$GSHHG_VERSION.tar.gz


RUN   mkdir -p $GMT_INSTALL_DIR				 &&\
      cd $GMT_INSTALL_DIR			 	 &&\
#      wget $GMT_DCW_FTP				 &&\
      wget $GMT_DCW_HTTP				 &&\
#      wget $GMT_GSHHG_FTP				 &&\
      wget $GMT_GSHHG_HTTP		 		 &&\
      tar -xzf dcw-gmt-$DCW_VERSION.tar.gz 		 &&\
      tar -xzf gshhg-gmt-$GSHHG_VERSION.tar.gz 		 &&\
      rm -f *.tar.gz                      		   
    
## Part 3: Copy and Build GMT

ARG GMT_CMAKE_ARGS=""	    
ARG BUILD_DEPS="subversion build-essential cmake libcurl4-gnutls-dev \
	    libnetcdf-dev libgdal1-dev libfftw3-dev libpcre3-dev \
	    liblapack-dev libblas-dev "

COPY . $GMT_INSTALL_DIR/gmt-src

RUN apt-get update							&&\
    apt-get install -y $BUILD_DEPS					&&\
    (									  \
    	cd /opt/gmt							&&\
    	mkdir build							&&\
	cd build							&&\
    	cmake -D CMAKE_INSTALL_PREFIX=$GMT_INSTALL_DIR			  \
	      -D DCW_ROOT=$GMT_INSTALL_DIR/dcw-gmt-$DCW_VERSION		  \
	      -D GSHHG_PATH=$GMT_INSTALL_DIR/gshhg-gmt-$GSHHG_VERSION 	  \
	      $GMT_CMAKE_ARGS						  \
	      $GMT_INSTALL_DIR/gmt-src					&&\
    	make  								&&\
    	make install							  \
    ) 		     	       						&&\
    rm -rf /opt/gmt/build						&&\
    apt-get purge  -y $BUILD_DEPS					&&\
    apt-get autoremove -y 						&&\
    rm -rf /var/lib/apt/lists/*						&&\
    export PATH=$PATH:/opt/gmt/bin					

WORKDIR /workspace

ENV PATH=$PATH:/opt/gmt/bin

ENTRYPOINT ["/opt/gmt/bin/gmt"]
