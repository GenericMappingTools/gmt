.. index:: ! fzmapper
.. include:: ../module_supplements_purpose.rst_

********
fzmapper
********

|fzmapper_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt fzmapper** [ |-A| ]
[ |-F|\ *origfile* ]
[ |-G|\ *vgg_grid* ]
[ |-L|\ *labelint* ] 
[ |-O| ]
[ |-S| ]
[ |-T|\ *prefix* ]
[ |-W|\ *width*[**c**\|\ **i**\|\ **p**] ]
[ GMT_V_OPT ]

|No-spaces|

Description
-----------

**fzmapper** is a Bash script developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It make a Mercator map of cross-profiles from the
processed fracture zone (FZ) traces and cross-profiles as
produced by :doc:`grdtrack`.  Optionally, overlay analysis and blend results.

Optional Arguments
------------------

.. _-A:

**-A**
    In addition to the resampled FZ trace and the cross-profiles, overlay the
    result of :doc:`fzanalyzer` as color-coded points (red for data, green for trough,
    and blue for blend model) [No model results].

.. _-F:

**-F**\ *profile*
    This is the original digitized FZ traces that was given as input to:doc:`grdtrack`.

.. _-G:

**-G**\ *vgg_grid*
    Optionally, supply the name of the VGG grid to use as background [@earth_vgg_02m].

.. _-O:

**-O**
    Instead of making a stand-alone PDF plot, write a PostScript overlay to stdout,
    i.e., make the plot using the GMT **-O -K** options.

.. _-S:

**-S**
    Overlay the smoothed FZ trace produced by :doc:`fzblender` [no overlay]. 

.. _-T:

**-T**\ *prefix*
    Sets the file name prefix used for all input files as produced by
    :doc:`fzanalyzer`.  The default is *fztrack*.  The files are *prefix*_cross.txt and 
    *prefix*_par.txt as well as the resampled output from :doc:`grdtrack` which
    should be called *prefix*_resampled.txt.  When |-S| is set we also look for *prefix*_blend.txt
    as produced by :doc:`fzblender`, and with |-A| we also look for *prefix*_analysis.txt.

.. _-W:

**-W**\ *width*\ [**c**\|\ **i**\|\ **p**]
    Sets the *width* of the Mercator map.  Unless the measure unit is appended the
    unit is assumed to be whatever the GMT default :term:`PROJ_LENGTH_UNIT` is currently set to.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

Examples
--------

To look at the cross-profiles and the best-fit models and optimal FZ locations in
map view, with the prefix used previously as "traces", using a 9 inch wide Mercator,
and only label every other profile, try::

    fzmapper -Ttraces -W9i -L2 -Fguides.xy -S -V -A

where we use the original digitized FZ locations guides.xy, choosing to annotate every
other profile.  The final map will be named *prefix*_map.pdf.  For cross-section
profiles, see :doc:`fzprofiler`.

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzblender </supplements/gsfml/fzblender>`
:doc:`mlconverter </supplements/gsfml/mlconverter>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`,
:doc:`grdtrack </grdtrack>`,
