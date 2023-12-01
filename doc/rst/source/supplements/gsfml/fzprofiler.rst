.. index:: ! fzprofiler
.. include:: ../module_supplements_purpose.rst_

**********
fzprofiler
**********

|fzprofiler_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**fzprofiler** [ |-H|\ *height*[**c**\|\ **i**\|\ **p**] ]
[ |-I|\ *profile* ] 
[ |-L|\ *inc* ]
[ |-M|\ *ncols* ]
[ |-T|\ *prefix* ]
[ GMT_V_OPT ]
[ |-W|\ *width*[**c**\|\ **i**\|\ **p**] ]

|No-spaces|

Description
-----------

**fzprofiler** is a script developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It plots the cross-profiles and the information produced by :doc:`fzanalyzer`.
All plots are combined into one plot using whatever custom page size is needed to fit the individual plots given
their specifications.

Optional Arguments
------------------

.. _-H:

**-H**\ *width*\ [**c**\|\ **i**\|\ **p**]
    Sets the plot height of an individual profile. Unless the measure unit is appended the
    unit is assumed to be whatever the GMT default :term:`PROJ_LENGTH_UNIT` is currently set to.

.. _-I:

**-I**\ *profile*
    By default we plot all the cross-profiles in one stack.  To select a single
    profile only, append the running number of the profile, where 0 is the first profile.

.. _-L:

**-L**\ *inc*
    Determines which profiles to plot.  By default we plot every profile (*inc* = 1).
    Use |-L| to plot every *inc* profile instead [1].

.. _-N:

**-N**\ *ncols*
    Spread profiles across *ncols* columns.  If *ncols* = 1
    then all profiles are stacked vertically in one long panel; if *ncols* = 2
    then we split the profiles evenly between two columns, etc.

.. _-T:

**-T**\ *prefix*
    Sets the file name *prefix* used for all input files as produced by
    :doc:`fzanalyzer`.  The default is fztrack. The files are *prefix*_cross.txt, *prefix*_analysis.txt,
    *prefix*_par.txt as well as the resampled output from :doc:`grdtrack` which
    should be called *prefix*_resampled.txt.

.. _-W:

**-W**\ *width*\ [**c**\|\ **i**\|\ **p**]
    Similarly sets the plot width of an individual profile.  For units, see |-H|.
 
.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Plot Features
-------------

**fzprofiler** packs much information into each plot by using different symbols and
colors.  The cross-profile VGG data are plotted as red circles connected by a faint
red dotted line.  The central corridor set with :doc:`fzanalyzer` **-D** is shown in light
blue.  Black line is the best symmetric
trough model; inverted triangle indicates the best location of the FZ (trough) and
its error bars reflect the half-width.  Blue line is the best asymmetric blend model;
circle shows the best location of the maximum VGG slope.  Finally, the inverted
red triangle and half-width error bar shows the best empirical trough location.  Dashed
orange line shows the crustal age variations.  Profile orientation is
indicated by the W-E or S-N letters, with vertical scale shown on the left (VGG) and
right (age).  Finally, each panel prints the best blend parameters.

Examples
--------

To look at the cross-profiles and the best-fit models and optimal FZ locations on
a per-profiles basis, with the prefix used previously as "traces", and placing the
profiles into two columns, each 6 inches wide with individual plots 2 inches tall,
try::

    fzprofiler -Ttraces -W6i -H2i -N2 -V

The final plot will be named *prefix*_cross.pdf.  To see the same profiles in map
view, use :doc:`fzmapper`.  To plot a synthetic profile, see :doc:`fzmodeler`.

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzblender </supplements/gsfml/fzblender>`
:doc:`mlconverter </supplements/gsfml/mlconverter>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`grdtrack </grdtrack>`,
