Docker User Guide
=================

Dockerfiles are provided  for development build and testing purposes.
Using this one can build and distribute GMT using containers that have the
environment with them. Additionally, contributors can test  development
locally without needing to install the full build toolchain on their
local machine

> **Note:** The Dockerfiles are not yet optimized for image size

Software Requirements
---------------------

- `docker`



Building the GMT Docker Image
-----------------------------

Building the docker image, and GMT with it, can be done with one command

    docker build .


for convenience however, it is usually a good idea to tag the image so it
can be referred to more easily

    docker build --tag gmt .


This should provide a suitable image to test or use `gmt`

    docker run --rm gmt defaults -Vd
    docker run --rm gmt pscoast -R0/10/0/10 -JM6i -Ba -Ggray -P -Vd > test.ps


an alias or shell script can be written to wrap the docker details to use the image
directly in shell scripts


    alias gmt='docker run --rm gmt'
    gmt defaults -Vd
    gmt pscoast -R0/10/0/10 -JM6i -Ba Ggray -P -Vd > test.ps








