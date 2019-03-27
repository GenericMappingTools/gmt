:tocdepth: 4

.. set default highlighting language for this document:

.. highlight:: c

**The Generic Mapping Tools**

**The Julia Interface**

**Joaquim F. Luis**

**Universidade do Algarve, Faro, Portugal**

**Pål (Paul) Wessel**

**SOEST, University of Hawai'i at Manoa**

Introduction
============

The `Julia <http://julialang.org>`_ wrapper is a companion to the MATLAB wrapper that works in very similar way,
making use of the relative similarity of the two languages. 

Installing
==========

Contrary to the rest of all **GMT** products the Julia wrapper has to live in a Github repository. At the time of
this writing the wrapper was not yet registered within the Julia package manager, so to install it one has to
access the Github address directly. After installing Julia and from within its shell (*aka* the REPL) issue this
command: (but refer to Julia's `package manager <http://docs.julialang.org/en/release-0.4/stdlib/pkg/?highlight=init#package-manager-functions>`_)

  ::

    Pkg.add("GMT")

Now you are ready to start using the **GMT** wrapper and the only condition for it to work is that the GMT shared libs
are listed in your path. On Windows the **GMT** installer takes care of that but be careful that no other previous version
is found first.

On UNIX things are more complicated (*surprise*). On OSX, the only other OS tested so far by us, we got a working version
by running, in the Julia *REPL*

  ::

    push!(Libdl.DL_LOAD_PATH, "/Users/j/programs/gmt5/lib")

this adds, for one particular user case, the *gmt/lib* directory to the list of system locations searched for valid libraries.
There might be more problems finding other gmt dependencies but with Homebrew builds it works because those dependencies
are located at /usr/local/lib and the system finds them with no other help. To make that line permanent, add it to your
~/.juliarc.jl file (if don't have one yet, create it).

Using
=====

The Julia wrapper was designed to work in a way the closest as possible to the command line version.
In this sense, all **GMT** options are put in a single text string that is passed, plus the data itself when it applies,
to the ``gmt()`` command. For example to reproduce the CookBook example of an Hemisphere map using a
Azimuthal projection

  ::
    
    using GMT
    gmt("coast -Rg -JA280/30/3.5i -Bg -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps")

Note the ``using GMT`` command. We need to do that to load the ``GMT.jl`` wrapper and the first time it will
take a little longer because it will need to *JIT* compile the module's code. Following commands, however, will run at same
speed as the command line calls to **GMT**.

However, the above example is not particularly interesting as after all we could do the exact same thing on the a shell
command line. Things start to get interesting when we can send data *in* and *out* from Julia to
**GMT**. So, consider the following example

  ::

    t = rand(100,3) * 150;
    G = gmt("surface -R0/150/0/150 -I1", t);

Here we just created a random data *100x3* matrix and told **GMT** to grid it using it's program
*surface*. Note how the syntax follows closely the standard usage but we sent the data to be
interpolated (the *t* matrix) as the second argument to the ``gmt()`` function. And on return we
got the *G* variable that is a structure (actually a Julia's ``type``) holding the grid and it's metadata. See the 
:ref:`grid type <grid-type>` for the details of its members.

Imagining that we want to plot that random data art, we can do it with a call to *grdimage*\ , like

  ::

   gmt("grdimage -JX8c -Ba -P -Cblue,red > crap_img.ps", G)

Note that we now sent the *G grid* as argument instead of the **-G**\ *gridname* that we would have
used in the command line. But for readability we could well had left the **-G** option in command string. E.g:

  ::

   gmt("grdimage -JX8c -Ba -P -Cblue,red -G > crap_img.ps", G)

While for this particular case it makes no difference to use or not the **-G**, because there is **only**
one input, the same does not hold true when we have more than one. For example, we can run the same example
but compute the CPT separately.

  ::

   cpt = gmt("grd2cpt -Cblue,red", G);
   gmt("grdimage -JX8c -Ba -P -C -G > crap_img.ps", G, cpt)

Now we had to explicitly write the **-C** & **-G** (well, actually we could have omitted the **-G** because
it's a mandatory input but that would make the things more confusing). Note also the order of the input data variables.
It is crucial that they are used in the **exact** same order as the options in the command string.

To illustrate another aspect on the importance of the order of input data let us see how to plot a sinus curve
made of colored filled circles.

.. code-block:: none

   x = linspace(-pi, pi);             # The *xx* var
   seno = sin(x);                     # *yy*
   xyz  = [x seno seno];              # Duplicate *yy* so that it can be colored
   cpt  = gmt("makecpt -T-1/1/0.1");  # Create a CPT
   gmt("plot -R-3.2/3.2/-1.1/1.1 -JX12c -Sc0.1c -C -P -Ba > seno.ps", xyz, cpt)

The poin here is that we had to give *cpt, xyz* and not *xyz, cpt* (which would error) because input data
associated with an option letter **always comes first** and has to respect the corresponding options order
in command string.

To plot text strings we send in the input data wrapped in a cell array. Example:

  ::

   lines = Any["5 6 Some label", "6 7 Another label"];
   gmt("text -R0/10/0/10 -JM6i -Bafg -F+f18p -P > text.ps", lines)

and we get back text info in cell arrays as well. Using the *G* grid computed above we can run *gmtinfo* on it

  ::

    info = gmt("gmtinfo", G)

At the end of an **GMT** session work we call the internal functions that will do the house keeping of
freeing no longer needed memory. We do that with this command:

  ::

   gmt("destroy")


So that's basically how it works. When numeric data has to be sent *in* to **GMT** we use
Julia variables holding the data in matrices or structures or cell arrays depending on the case. On
return we get the computed result stored in variables that we gave as output arguments.
Things only complicate a little more for the cases where we can have more than one *input* or
*output* arguments. The file *gallery.jl* in *test* directory, that reproduces the examples in the
Gallery section of the **GMT** documentation, has many (not so trivial) examples on usage of the Julia/GMT API.

To run the examples in *gallery.jl* we have to load the file first, which is located in your .julia directory.
For me it lives in *C:/j/.julia/v0.4/GMT/test/gallery.jl* and we have to edit it to set the path to the **GMT**
root dir so that the data file used in examples can be found. After that, run

  ::

    include("C:/j/.julia/v0.4/GMT/test/gallery.jl")

now the examples are wrapped in functions named *ex01*, *ex02*, ... *ex45* (not all are yet ported/working) and we
just call them with

  ::

    ex01()

.. _grid-type:

.. code-block:: none

    type GMTgrid                  # The type holding a local header and data of a GMT grid
       proj4::String              # Projection string in PROJ4 syntax (Optional)
       wkt::String                # Projection string in WKT syntax (Optional)
       range::Array{Float64,1}    # 1x6 vector with [x_min x_max y_min y_max z_min z_max]
       inc::Array{Float64,1}      # 1x2 vector with [x_inc y_inc]
       registration::Int          # Registration type: 0 -> Grid registration; 1 -> Pixel registration
       nodata::Float64            # The value of nodata
       title::String              # Title (Optional)
       comment::String            # Remark (Optional)
       command::String            # Command used to create the grid (Optional)
       datatype::String           # 'float' or 'double'
       x::Array{Float64,1}        # [1 x n_columns] vector with XX coordinates
       y::Array{Float64,1}        # [1 x n_rows]    vector with YY coordinates
       z::Array{Float32,2}        # [n_rows x n_columns] grid array
       x_units::String            # Units of XX axis (Optional)
       y_units::String            # Units of YY axis (Optional)
       z_units::String            # Units of ZZ axis (Optional)
       layout::String             # A three character string describing the grid memory layout
    end

Definition of the *grid type* that holds a grid and its metadata.


.. _img-type:

.. code-block:: none

    type GMTimage                 # The type holding a local header and data of a GMT image
       proj4::String              # Projection string in PROJ4 syntax (Optional)
       wkt::String                # Projection string in WKT syntax (Optional)
       range::Array{Float64,1}    # 1x6 vector with [x_min x_max y_min y_max z_min z_max]
       inc::Array{Float64,1}      # 1x2 vector with [x_inc y_inc]
       registration::Int          # Registration type: 0 -> Grid registration; 1 -> Pixel registration
       nodata::Float64            # The value of nodata
       title::String              # Title (Optional)
       comment::String            # Remark (Optional)
       command::String            # Command used to create the image (Optional)
       datatype::String           # 'uint8' or 'int8' (needs checking)
       x::Array{Float64,1}        # [1 x n_columns] vector with XX coordinates
       y::Array{Float64,1}        # [1 x n_rows]    vector with YY coordinates
       image::Array{UInt8,3}      # [n_rows x n_columns x n_bands] image array
       x_units::String            # Units of XX axis (Optional)
       y_units::String            # Units of YY axis (Optional)
       z_units::String            # Units of ZZ axis (Optional) ==> MAKES NO SENSE
       colormap::Array{Clong,1}   # 
       alpha::Array{UInt8,2}      # A [n_rows x n_columns] alpha array
       layout::String             # A four character string describing the image memory layout
    end

Definition of the *image type* that holds an image and its metadata.

.. _dataset-type:

.. code-block:: c

    type GMTdataset
        header::String
        data::Array{Float64,2}
        text::Array{Any,1}
        comment::Array{Any,1}
        proj4::String
        wkt::String
    end

Definition of the *daset type*.

.. _cpt-type:

.. code-block:: none

    type GMTcpt
        colormap::Array{Float64,2}
        alpha::Array{Float64,1}
        range::Array{Float64,2}
        minmax::Array{Float64,1}
        bfn::Array{Float64,2}
        depth::Cint
        hinge::Cdouble
        cpt::Array{Float64,2}
        model::String
        comment::Array{Any,1}   # Cell array with any comments
    end

Definition of the *cpt type* that holds a CPT palette.

.. _ps-type:

.. code-block:: none

    type GMTps
        postscript::String      # Actual PS plot (text string)
        length::Int             # Byte length of postscript
        mode::Int               # 1 = Has header, 2 = Has trailer, 3 = Has both
        comment::Array{Any,1}   # Cell array with any comments
    end

Definition of the *PotScript type*.
