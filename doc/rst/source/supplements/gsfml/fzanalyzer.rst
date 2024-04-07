.. index:: ! fzanalyzer
.. include:: ../module_supplements_purpose.rst_

**********
fzanalyzer
**********

|fzanalyzer_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt fzanalyzer** *crossprofiles*
|-F|\ *fzlines*
[ |-A|\ *min*/*max*/*inc* ]
[ |-C|\ *min*/*max*/*inc* ]
[ |-D|\ *corrwidth* ]
[ |-I|\ *FZ*\ [/*profile*] ]
[ |-S|\ [**b**\|\ **c**]]
[ |-T|\ *prefix* ]
[ |SYN_OPT-V| ]
[ |-W|\ *min*/*max*/*inc* ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**fzanalyzer** is a tool developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It reads processed fracture zone (FZ) traces and cross-profiles as
produced by :doc:`grdtrack </grdtrack>`.  It then analyzes the trace of each FZ
by examining cross-sections orthogonal to the FZ trend and modeling these profiles
using a blend model of "Atlantic"-style symmetric troughs and "Pacific"-style asymmetric,
dipole-like anomalies, modulated with some peripheral bulges ("compression").
We also fit just the symmetric trough model and
examine the empirical data minimum and trough width.  Estimates are made
of the width of the data trough and 1-sigma uncertainties on the best FZ locations
given by the various models.  We also compute several statistical measures and return
all the model parameters as a function of distances along each FZ.


Required Arguments
------------------

*crossprofiles*
    This file is a table with cross-profiles as produced by :doc:`grdtrack </grdtrack>` **-C**
    from an approximate digitized trace (with *lon*, *lat*) of one or more FZs.
    This is an ASCII (or binary, see **-bi**) file that must contain 7 data columns:
    *lon, lat, dist, azimuth, vgg, age, fzdist*.

.. _-F:

**-F**\ *fzlines*
    Here, *fzlines* is a file with resampled track lines obtained by running :doc:`grdtrack </grdtrack>` **-D**.
    As for *crossprofiles* the file must contain the same 7 data columns *lon, lat, dist, azimuth, vgg, age, fzdist*.
    See Input Files for more details.

Optional Arguments
------------------

.. _-A:

**-A**\ *min*/*max*/*inc* 
    Specifies one or three parameters that control how the blending of the model signals will be done.
    Here, *min* is the minimum asymmetry value [0, i.e., "Atlantic" symmetric trough],
    *max* is the maximum asymmetry [1], i.e., "Pacific" dipole signal], and *inc* is the
    increment used for the search [0.05].  To specify just a single asymmetry value (no search),
    just provide the single desired asymmetry.

.. _-C:

**-C**\ *min*/*max*/*inc* 
    Specifies one or three parameters that control how the search for the "compression" model signal will be done.
    Here, *min* is the minimum compression value [0], *max* is the maximum compression [1, i.e., 
    "Mexican Hat" end-member], and *inc* is the increment used for the search [0.05].
    To specify just a single compression value (no search), just provide the single desired compression value..

.. _-D:

**-D**\ *corrwidth*
    Specifies a *corrwidth* (in km) that sets the width of the central corridor [25].  The purpose
    of this corridor is to constrain how far off center we may seek to relocate the location of the FZ trough.

.. _-I:

**-I**\ *FZ*\ [/*profile*]
    By default, we will analyze the cross-profiles generated for all FZs.  However,
    you can use |-I| to specify a particular FZ *id* id (first *id* is 0).
    Optionally, you can select that only one *profile* from that FZ be processed [Default is all].
    Note that the output files will still contain all profiles but derived quantities will be
    zero except for the chosen profiles.

.. _-S:


**-S**\ [**b**\|\ **c**]
    Output the parameters set by the command-line options in a format suitable for inclusion
    in a Bourne/bash shell script.  Alternatively, append **c** for csh/tcsh syntax.

.. _-T:

**-T**\ *prefix*
    Sets the file name prefix used for all output files [fztrack].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: ../../explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *min*/*max*/*inc* 
    Specifies three parameters that control how the modeling of the cross-FZ signal will be done.
    Here, *min* is the minimum FZ signal width (in km) for a nonlinear width search [1],
    *max* is the maximum width [50], and *inc* is the increment used for the width search [1].
    It is recommended to determine suitable limits of a particular region and tune |-W| accordingly.
    Selecting too wide a range might lead to spurious fits driven by data features not associated with
    the FZ trough.

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-do.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. include:: ../../explain_-q.rst_

.. include:: ../../explain_help.rst_

Input Files
-----------

The two input files are themselves generated by running :doc:`grdtrack </grdtrack>` first.  This step
requires a set of digitized FZ tracks (*lon*, *lat*) and three data grids: (a) a
VGG vertical gravity gradient file, (b) a 2 minute crustal age grid, and (c)
a grid with distance to the nearest FZ in km, listed in that order.  The critical file is the
VGG grid.  If you don't have or care about ages and distances you can make dummy grids that
are all NaNs.  You design your cross-profile layout and resampled FZ trackes using :doc:`grdtrack </grdtrack>`
options **-C** and **-D**..

Nearest Fracture Zone Distances
-------------------------------

You can use :doc:`grdmath` to create the nearest fracture zone distance grid (in km) required
to prepare the profiles. A typical command suitable for the Nazca plate area might be::

    gmt grdmath -R-120/-65/-50/5 -I5m -fg digitize.txt LDIST DEG2KM = dist2fz.nc

Since this is a very slow calculation for numerous FZs it is not necessary to use a very high resolution
in |-I| since the distances change smoothly and interpolation will be approximately correct.  Consider
making a global grid but do it in quadrants (or smaller region chunks) and run concurrently on a multi-core
computer. For example, to make a global grid from quadrants, one may run::

    gmt grdmath -R0/180/0/90 -I2m -fg -V3 global_FZ.txt LDIST DEG2KM = WN.nc
    gmt grdmath -R180/360/0/90 -I2m -fg -V3 FZ_KM.txt LDIST DEG2KM = EN.nc
    gmt grdmath -R0/180/-90/0 -I2m -fg -V3 FZ_KM.txt LDIST DEG2KM = WS.nc
    gmt grdmath -R180/360/-90/0 -I2m -fg -V3 FZ_KM.txt LDIST DEG2KM = ES.nc

then blend these together into a global grid with::

    gmt grdblend -Rg -I2 -fg EN.nc WN.nc ES.nc WS.nc -Gdist2FZ.nc -V
    rm -f WN.nc EN.nc WS.nc ES.nc

To make NaN grids for ages and/or distances for the Nazca area, use::

    gmt grdmath -R-120/-65/-50/5 -I5m -fg 0 0 NaN = ages.nc

Output Files
------------

**fzanalyzer** can produce up to three output files; these are described below:

    #. File *prefix*\ _analysis.txt contains the results of the analysis for each cross
       profile.  There 61 output columns containing the fitted or observed values (see
       Determined Parameters).  This file is used by :doc:`fzblender` to produce a smooth
       and optimal fit to the fracture zone.
    #. File *prefix*\ _cross.txt contains both observed and predicted best-fitting models
       for each cross profile.  It can be used for plotting and visual analysis of the
       results on a profile-by-profile basis.
    #. The *prefix*\ _par.[c]sh is either a Bourne (|-S|) or cshell (|-S|\ **c**) script
       that contains all parameters specified by the command line as shell variables. You
       can include this script in custom mapping or analysis scripts and use the variables
       as you see fit.
    #. Finally, while not an output file from **fzanalyzer**, you should use the name
       *prefix*\ _resampled.txt for the output of :doc:`grdtrack </grdtrack>` **-D** as that is what the
       scripts for plotting expect.

Examples
--------

To analyze digitized FZs we use the Sandwell/Smith VGG (1 minute vertical gravity gradient
@earth_vgg_01m), a 1-minute age grid (@earth_ages_01m), and a nearest-FZ distance grid
(in km). Given the potential FZ locations in the multi-segment file fz_digitized.txt, we
specify a 40 km cross-profile length, with profiles spaced every 5 km, and use an
along-cross-profile sampling interval of 2 km, by running::

    gmt grdtrack fz_digitized.txt -C40k/2k/5k -G@earth_vgg_01m -G@earth_ages_01m -Gdist.nc -Dtraces_resampled.txt -fg --FORMAT_GEO_OUT=ddd.xxxx --FORMAT_FLOAT_OUT=%.1f > xprofiles.txt

These two data tables can now be used with **fzanalyzer** to analyze the traces.  Here,
we specify a 20 km central corridor and accept default values for most settings::

    gmt fzanalyzer xprofiles.txt -D20 -Ftraces_resampled.txt -Ttraces -S --FORMAT_GEO_OUT=ddd.xxxx --FORMAT_FLOAT_OUT=%.1f

You can then make plots of these cross-profiles with best-fitting curves and parameters
indicated by using::

    fzprofiler traces -W6i -H2i -N2

which will plot all cross-profiles in separate 6x2 inch cross-sections stacked in one vertical panel.
You can show this information in map view via::

    fzmapper traces -W9i -L1 -Ffz_digitized.txt

See the :doc:`fzprofiler` and :doc:`fzmapper` documentation for further details.

Determined Parameters
---------------------

Here are the header codes for each of the 61 output columns and what they represent:

    - **XR**: Longitude of raw digitized trace.
    - **YR**: Latitude of raw digitized trace.
    - **DR**: Distance at digitized points along raw digitized trace.
    - **AR**: Azimuth at digitized points along raw digitized trace.
    - **ZR**: Data value at digitizing locations
    - **TL**: Crustal age estimate at left side of FZ (negative distances).
    - **TR**: Crustal age estimate at right side of FZ (positive distances).
    - **SD**: Shift of data minimum (in km) from raw line origin.
    - **ST**: Shift of trough model minimum (in km) from raw line origin.
    - **SB**: Shift of blend model minimum (in km) from raw line origin.
    - **SE**: Shift of blend model maximum slope (in km) from raw line origin.
    - **BL**: Best blend value [0-1].
    - **OR**: Orientation of blend model profile (-1 means left side is old, +1 means left side is young).
    - **WD**: Width of data trough.
    - **WT**: Width of model trough (for trough model).
    - **WB**: Width of model trough (for blend model).
    - **AD**: Peak-to-trough amplitude from data.
    - **AT**: Peak-to-trough amplitude from model (for trough model).
    - **AB**: Peak-to-trough amplitude from model (blend).
    - **UT**: Compression indicator (for trough model).
    - **UB**: Compression indicator (for blend model).
    - **VT**: Variance reduction (%) from model (for trough model).
    - **VB**: Variance reduction (%) from model (for blend model).
    - **FT**: F-statistic (for trough model).
    - **FB**: F-statistic (for blend model).
    - **XDL**: Longitude of data minimum left bounds.
    - **XD0**: Longitude of data minimum.
    - **XDR**: Longitude of data minimum right bounds.
    - **YDL**: Latitude of data minimum left bounds.
    - **YD0**: Latitude of data minimum.
    - **YDR**: Latitude of data minimum right bounds.
    - **ZDL**: Value of data minimum left bounds.
    - **ZD0**: Value of data minimum.
    - **ZDR**: Value of data minimum right bounds.
    - **XTL**: Longitude of minimum (for trough model) left bounds.
    - **XT0**: Longitude of minimum (for trough model).
    - **XTR**: Longitude of minimum (for trough model) right bounds.
    - **YTL**: Latitude of minimum (for trough model) left bounds.
    - **YT0**: Latitude of minimum (for trough model).
    - **YTR**: Latitude of minimum (for trough model) right bounds.
    - **ZTL**: Model prediction (for trough model) at left bounds.
    - **ZT0**: Model prediction minimum (for trough model).
    - **ZTR**: Model prediction (for trough model) at right bounds.
    - **XBL**: Longitude of minimum (for blend model) left bounds.
    - **XB0**: Longitude of minimum (for blend model).
    - **XBR**: Longitude of minimum (for blend model) right bounds.
    - **YBL**: Latitude of minimum (for blend model) left bounds.
    - **YB0**: Latitude of minimum (for blend model).
    - **YBR**: Latitude of minimum (for blend model) right bounds.
    - **ZBL**: Model prediction (for blend model) at left bounds.
    - **ZB0**: Model prediction minimum (for blend model).
    - **ZBR**: Model prediction (for blend model) at right bounds.
    - **XEL**: Longitude of maximum slope (for blend model) left bounds.
    - **XE0**: Longitude of maximum slope (for blend model).
    - **XER**: Longitude of maximum slope (for blend model) right bounds.
    - **YEL**: Latitude of maximum slope (for blend model) left bounds.
    - **YE0**: Latitude of maximum slope (for blend model).
    - **YER**: Latitude of maximum slope (for blend model) right bounds.
    - **ZEL**: Model prediction at maximum slope (for blend model) at left bounds.
    - **ZE0**: Model prediction at maximum slope (for blend model).
    - **ZER**: Model prediction at maximum slope (for blend model) at right bounds.

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzblender </supplements/gsfml/fzblender>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`,
:doc:`grdmath </grdmath>`,
:doc:`grdtrack </grdtrack>`,
:doc:`mlconverter </supplements/gsfml/mlconverter>`

References
----------

Wessel, P., Matthews, K. J., Müller, R. D., Mazzoni, A., Whittaker, J. M., Myhill, R., Chandler, M. T.,
2015, "Semiautomatic fracture zone tracking", *Geochem. Geophys. Geosyst.*, 16 (7), 2462–2472.
https://doi.org/10.1002/2015GC005853.
