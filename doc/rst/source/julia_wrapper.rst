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
command: (but refer to Julia's `package manager <http://docs.julialang.org/en/release-0.4/stdlib/pkg/?highlight=init#package-manager-functions>`_

    Pkg.clone("git://github.com/joa-quim/GMT.jl.git")

Now you are ready to start using the **GMT** wrapper and the only condition for it to work is that the GMT5.2 shared libs
are listed in your path. On Windows, the only version tested so far, the **GMT** installer takes care of that but be
careful that no other previous version is found first.

Using
=====

The Julia wrapper was designed to work in a way the closest as possible to the command line version.
In this sense, all **GMT** options are put in a single text string that is passed, plus the data itself when it applies,
to the ``gmt()`` command. For example to reproduce the CookBook example of an Hemisphere map using a
Azimuthal projection

  ::
    
    using GMT
    gmt("pscoast -Rg -JA280/30/3.5i -Bg -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps")

Note the ``using GMT`` command. We need to do that to load the ``GMT.jl`` wrapper and the first time it will
take a little longer because it need to compile the module's code but following commands will run at same
speed as the command line calls to **GMT**.

But that is not particularly interesting as after all we could do the exact same thing on the a shell
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
   gmt("grdimage -JX8c -Ba -P -C -G > crap_img.ps", cpt, G)

Now we had to explicitly write the **-C** & **-G** (well, actually we could have omitted the **-G** because
it's a mandatory input but that would make the things more confusing). Note also the order of the input data variables.
It is crucial that they are used in the **exact** same order as the options in the command string.

To illustrate another aspect on the importance of the order of input data let us see how to plot a sinus curve
made of colored filled circles.

  ::

   x = linspace(-pi, pi)';            # The *xx* var
   seno = sin(x);                     # *yy*
   xyz  = [x seno seno];              # Duplicate *yy* so that it can be colored
   cpt  = gmt("makecpt -T-1/1/0.1");  # Create a CPT
   gmt("psxy -R-3.2/3.2/-1.1/1.1 -JX12c -Sc0.1c -C -P -Ba > seno.ps", cpt, xyz)

The poin here is that we had to give *cpt, xyz* and not *xyz, cpt* (which would error) because input data
associated with an option letter **always comes first** and has to respect the corresponding options order
in command string.

To plot text strings we send in the input data wrapped in a cell array. Example:

  ::

   lines = Any["5 6 Some label", "6 7 Another label"];
   gmt("pstext -R0/10/0/10 -JM6i -Bafg -F+f18p -P > text.ps", lines)

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
Gallery section of the **GMT** documentation, has many (not so trivial) examples on usage of the Julia GMT5.2 API.



.. _grid-type:

.. code-block:: c

  ProjectionRefPROJ4     # Projection string in PROJ4 syntax (Optional)
  ProjectionRefWKT       # Projection string in WKT syntax (Optional)
  range                  # 1x6 vector with [x_min x_max y_min y_max z_min z_max]
  inc                    # 1x2 vector with [x_inc y_inc]
  n_rows                 # Number of rows in grid
  n_columns              # Number of columns in grid
  n_bands                # Not-yet used (always == 1)
  registration           # Registration type: 0 -> Grid registration; 1 -> Pixel registration
  NoDataValue            # The value of nodata
  title                  # Title (Optional)
  remark                 # Remark (Optional)
  command                # Command used to create the grid (Optional) 
  DataType               # 'float' or 'double'
  x                      # [1 x n_columns] vector with XX coordinates
  y                      # [1 x n_rows]    vector with YY coordinates
  z                      # [n_rows x n_columns] grid array
  x_units                # Units of XX axis (Optional)
  y_units                # Units of YY axis (Optional)
  z_units                # Units of ZZ axis (Optional)

Definition of the *grid type* that holds a grid and its metadata.
