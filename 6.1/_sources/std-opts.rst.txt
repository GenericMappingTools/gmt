##############
Common Options
##############

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - STANDARDIZED COMMAND LINE OPTIONS
     -
   * - **-B**\ *information*
     - Specify map frame and axes parameters :ref:`(...) <-B_full>`
   * - **-J**\ *parameters*
     - Select map projection :ref:`(...) <-J_full>`
   * - **-R**\ *west/east/south/north*\ [*/zmin/zmax*][**+r**][**+u**\ *unit*]
     - Specify region of interest :ref:`(...) <-R_full>`
   * - **-U**\ [*label*][**+c**][**+j**\ *just*][**+o**\ *dx*/*dy*]
     - Plot time-stamp on plot :ref:`(...) <-U_full>`
   * - **-V**\ [*verbosity*]
     - Run in verbose mode :ref:`(...) <-V_full>`
   * - **-X**\ [**a**\|\ **c**\|\ **f**\|\ **r**]\ *xshift*
     - Shift plot origin in *x*-direction :ref:`(...) <-XY_full>`
   * - **-Y**\ [**a**\|\ **c**\|\ **f**\|\ **r**]\ *yshift*
     - Shift plot origin in *y*-direction :ref:`(...) <-XY_full>`
   * - **-a**\ [*col*\ =]\ *name*\ [,\ *...*]
     - Associates aspatial data with columns :ref:`(...) <-aspatial_full>`
   * - **-bi**\ [*ncols*][*type*][**w**][**+l**\|\ **b**]
     - Select binary input :ref:`(...) <-bi_full>`
   * - **-bo**\ [*ncols*][*type*][**w**][**+l**\|\ **b**]
     - Select binary output :ref:`(...) <-bo_full>`
   * - **-c**\ [*row*\ ,\ *col*\|\ *index*]
     - Advance plot focus to selected (or next) subplot panel :ref:`(...) <-c_full>`
   * - **-d**\ [**i**\|\ **o**]\ *nodata*
     - Replace columns with *nodata* with NaN :ref:`(...) <-d_full>`
   * - **-e**\ [**~**]\ *"pattern"* \| **-e**\ [**~**]/\ *regexp*/[**i**]
     - Filter data records that match the given pattern :ref:`(...) <-e_full>`
   * - **-f**\ [**i**\|\ **o**]\ *colinfo*
     - Set formatting of ASCII input or output :ref:`(...) <-f_full>`
   * - **-g**\ [**a**]\ **x**\|\ **y**\|\ **d**\|\ **X**\|\ **Y**\|\ **D**\|[*col*]\ **z**\ *gap*\ [**+n**\|\ **p**]
     - Segment data by detecting gaps :ref:`(...) <-g_full>`
   * - **-h**\ [**i**\|\ **o**][*n*][**+c**][**+d**][**+m**\ *segheader*][**+r**\ *remark*][**+t**\ *title*]
     - ASCII [*I*\|\ *O*] tables have header record[s] :ref:`(...) <-h_full>`
   * - **-i**\ *cols*\ [**+l**][**+s**\ *scale*][**+o**\ *offset*][,\ *...*][,\ **t**\ [*word*]]
     - Selection of input columns :ref:`(...) <-icols_full>`
   * - **-je**\|\ **f**\|\ **g**
     - Mode of spherical distance calculation :ref:`(...) <-distcalc_full>`
   * - **-l**\ [*label*]\ [*modifiers*]
     - Add an item to the automatic plot legend :ref:`(...) <-l_full>`
   * - **-n**\ [**b**\|\ **c**\|\ **l**\|\ **n**][**+a**][**+b**\ *BC*][**+c**][**+t**\ *threshold*]
     - Set grid interpolation mode :ref:`(...) <-n_full>`
   * - **-o**\ *cols*\ [,...][,\ **t**\ [*word*]]
     - Selection of output columns :ref:`(...) <-ocols_full>`
   * - **-p**\ [**x**\|\ **y**\|\ **z**]\ *azim*\ [/*elev*\ [/*zlevel*]][**+w**\ *lon0*/*lat0*\ [/*z0*]][**+v**\ *x0*/*y0*]
     - Control 3-D perspective view :ref:`(...) <perspective_full>`
   * - **-q**\ [**i**\|\ **o**][~]\ *rows*\ [**+c**\ *col*][**+a**\|\ **f**\|\ **s**]
     - Selection of input or output rows :ref:`(...) <-q_full>`
   * - **-r**\ [**g**\|\ **p**]
     - Sets grid registration :ref:`(...) <nodereg_full>`
   * - **-s**\ [*cols*][**+a**\|\ **r**]
     - Control treatment of NaN records :ref:`(...) <-s_full>`
   * - **-t**\ *transparency*
     - Set layer transparency :ref:`(...) <-t_full>`
   * - **-x**\ [[-]\ *n*]
     - Set number of cores in multi-threaded modules :ref:`(...) <core_full>`
   * - **-:**\ [**i**\|\ **o**]
     - Expect *y*/*x* input rather than *x*/*y* :ref:`(...) <colon_full>`
