#############################
Common Options (Classic Mode)
#############################

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - STANDARDIZED COMMAND LINE OPTIONS
     -
   * - **-B**\ *information*
     - :ref:`Specify map frame and axes parameters <-B_full>`
   * - **-J**\ *parameters*
     - :ref:`Select map projection <-J_full>`
   * - **-K**
     - :ref:`Append more PS later <-K_full>`
   * - **-O**
     - :ref:`This is an overlay plot <-O_full>`
   * - **-P**
     - :ref:`Select Portrait orientation <-P_full>`
   * - **-R**\ *west/east/south/north*\ [*/zmin/zmax*][**+r**][**+u**\ *unit*]
     - :ref:`Specify region of interest <-R_full>`
   * - **-U**\ [*label*][**+c**][**+j**\ *just*][**+o**\ *dx*/*dy*]
     - :ref:`Plot time-stamp on plot <-U_full>`
   * - **-V**\ [*verbosity*]
     - :ref:`Run in verbose mode <-V_full>`
   * - **-X**\ [**a**\|\ **c**\|\ **f**\|\ **r**]\ *xshift*
     - :ref:`Shift plot origin in x-direction <-XY_full>`
   * - **-Y**\ [**a**\|\ **c**\|\ **f**\|\ **r**]\ *yshift*
     - :ref:`Shift plot origin in y-direction <-XY_full>`
   * - **-a**\ [*col*\ =]\ *name*\ [,\ *...*]
     - :ref:`Associates aspatial data with columns <-aspatial_full>`
   * - **-bi**\ [*ncols*][*type*][**w**][**+l**\|\ **b**]
     - :ref:`Select binary input <-bi_full>`
   * - **-bo**\ [*ncols*][*type*][**w**][**+l**\|\ **b**]
     - :ref:`Select binary output <-bo_full>`
   * - **-d**\ [**i**\|\ **o**]\ *nodata*
     - :ref:`Replace columns with nodata with NaN <-d_full>`
   * - **-e**\ [**~**]\ *"pattern"* \| **-e**\ [**~**]/\ *regexp*/[**i**]
     - :ref:`Filter data records that match the given pattern <-e_full>`
   * - **-f**\ [**i**\|\ **o**]\ *colinfo*
     - :ref:`Set formatting of ASCII input or output <-f_full>`
   * - **-g**\ [**a**]\ **x**\|\ **y**\|\ **d**\|\ **X**\|\ **Y**\|\ **D**\|[*col*]\ **z**\ *gap*\ [**+n**\|\ **p**]
     - :ref:`Segment data by detecting gaps <-g_full>`
   * - **-h**\ [**i**\|\ **o**][*n*][**+c**][**+d**][**+m**\ *segheader*][**+r**\ *remark*][**+t**\ *title*]
     - :ref:`ASCII tables have header record[s] <-h_full>`
   * - **-i**\ *cols*\ [**+l**][**+d**\ *divide*][**+s**\ *scale*][**+o**\ *offset*][,\ *...*][,\ **t**\ [*word*]]
     - :ref:`Selection of input columns <-icols_full>`
   * - **-je**\|\ **f**\|\ **g**
     - :ref:`Mode of spherical distance calculation <-distcalc_full>`
   * - **-n**\ [**b**\|\ **c**\|\ **l**\|\ **n**][**+a**][**+b**\ *BC*][**+c**][**+t**\ *threshold*]
     - :ref:`Set grid interpolation mode <-n_full>`
   * - **-o**\ *cols*\ [,...][,\ **t**\ [*word*]]
     - :ref:`Selection of output columns <-ocols_full>`
   * - **-p**\ [**x**\|\ **y**\|\ **z**]\ *azim*\ [/*elev*\ [/*zlevel*]][**+w**\ *lon0*/*lat0*\ [/*z0*]][**+v**\ *x0*/*y0*]
     - :ref:`Control 3-D perspective view <perspective_full>`
   * - **-q**\ [**i**\|\ **o**][~]\ *rows*\ [**+c**\ *col*][**+a**\|\ **f**\|\ **s**]
     - :ref:`Selection of input or output rows <-q_full>`
   * - **-r**\ [**g**\|\ **p**]
     - :ref:`Sets grid registration <nodereg_full>`
   * - **-s**\ [*cols*][**+a**\|\ **r**]
     - :ref:`Control treatment of NaN records <-s_full>`
   * - **-t**\ *transparency*
     - :ref:`Set layer transparency <-t_full>`
   * - **-wy**\|\ **a**\|\ **w**\|\ **d**\|\ **h**\|\ **m**\|\ **s**\|\ **c**\ *period*\ [/*phase*][**+c**\ *col*]
     - :ref:`Convert selected coordinate to repeating cycles <-w_full>`
   * - **-x**\ [[-]\ *n*]
     - :ref:`Set number of cores in multi-threaded modules <core_full>`
   * - **-:**\ [**i**\|\ **o**]
     - :ref:`Expect y/x input rather than x/y <colon_full>`
