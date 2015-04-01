:tocdepth: 4

.. set default highlighting language for this document:

.. highlight:: c

**The Generic Mapping Tools**

**The MATLAB Interface**

**Pål (Paul) Wessel**

**SOEST, University of Hawai'i at Manoa**

**Walter H. F. Smith**

**Laboratory for Satellite Altimetry, NOAA/NESDIS/STAR**

**Remko Scharroo**

**EUMETSAT, Darmstadt, Germany**

**Joaquim F. Luis**

**Universidade do Algarve, Faro, Portugal**

**Florian Wobbe**

**Alfred Wegener Institute, Germany**

Introduction
============


Installing
==========

Windows
-------

Download the zip file and unzip it to a directory that is in the MATLAB path.
Also make sure that the GMT5.2 binaries are in the Windows path so they will be found by MATLAB.
If you want to (re)build the MEX file yourself, see the *compile_mex.bat* that comes along.

Unix (OSX and Linux)
--------------------

You are basically screwed and will have to temper your MATLAB installation so that the shared libs
that it ships don't get in the way.

Using
=====

The MATLAB wrapper was designed to work in a way the closest as possible to the command line version
and yet to provide all the facilities of the MATLAB IDE (the ML command line desktop). In this sense,
all **GMT** options are put in a single text string that is passed, plus the data itself when it applies,
to the ``gmt()`` command. But before that we must create a **GMT** session. We do that by running the
following command:

  ::

   gmt('create')

Next we are ready to run a standard **GMT** command. For example to reproduce the CookBook example
of an Hemisphere map using a Azimuthal projection

  ::

   gmt('pscoast -Rg -JA280/30/3.5i -Bg -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps')

but that is not particularly interesting as after all we could do the exact same thing on the a shell
command line. Things start to get interesting when we can send data *in* and *out* from MATLAB to
**GMT**. So, consider the following example

  ::

    t = rand(100,3) * 150;
    G = gmt('surface -R0/150/0/150 -I1', t);

Here we just created a random data *100x3* matrix and told **GMT** to grid it using it's program
*surface*. Note how the syntax follows closely the standard usage but we sent the data to be
interpolated (the *t* matrix) as the second argument to the ``gmt()`` function. And on return we
got the *G* variable that is a structure holding the grid and it's metadata. See the 
:ref:`grid struct <grid-struct>` for the details of its members.

Imagining that we want to plot that random data art, we can do it with a call to *grdimage*\ , like

  ::

   gmt('grdimage -JX8c -Ba -P -Cblue,red > crap_img.ps', G)
   gmt('destroy')

Note that we now sent the *G grid* as argument instead of the **-G**\ *gridname* that we would have
used in the command line. The second command means that we ended our work with **GMT** and call
the internall functions that will do the house keeping of freeing no longer needed memory.

So that's basically how it works. When numeric data has to be sent *in* to **GMT** we use
MATLAB variables holding the data in matrices or structures or cell arrays depending on the case. On
return we get the computed result stored in variables that we gave as output arguments.
Things only complicate a little more for the cases where we can have more than one *input* or
*output* arguments. The following examples illustrate some of those cases.



.. _grid-struct:

.. code-block:: c

  ProjectionRefPROJ4
  ProjectionRefWKT
  hdr
  range
  inc
  dim
  n_rows
  n_columns
  MinMax
  NoDataValue
  registration
  title
  remark
  command
  DataType
  LayerCount
  x
  y
  z
  x_units
  y_units
  z_units

Definition of the *grid structure* that holds a grid and its metadata.
