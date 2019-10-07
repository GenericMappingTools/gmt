#############
Julia Wrapper
#############

.. set default highlighting language for this document:

.. highlight:: c

**The Generic Mapping Tools**

**The Julia Interface**

**Joaquim F. Luis**

**Universidade do Algarve, Faro, Portugal**

**Pål (Paul) Wessel**

**SOEST, University of Hawai'i at Manoa**


The `Julia <http://julialang.org>`_ wrapper is a companion to the MATLAB wrapper that works in a very similar way to it. However, besides the so called `Monolithic <https://www.generic-mapping-tools.org/GMT.jl/latest/monolitic/>`_ mode it also provides a more modern and verbose interface to the GMT modules. For example, instead of using the GMT classic syntax to do a line plot:

  ::

    gmt psxy filename -R0/10/0/5 -JX12 -W1p -Ba -P > psfile.ps


one can simply do:

  ::

    plot("filename", show=true)

Details and installing instructions at `GMT.jl <https://github.com/GenericMappingTools/GMT.jl>`_
