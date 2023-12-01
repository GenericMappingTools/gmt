.. index:: ! fzinformer
.. include:: ../module_supplements_purpose.rst_

**********
fzinformer
**********

|fzinformer_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**zinformer** [ |-D| ]
[ |-F|\ *max* ]
[ |-I|\ *profile* ] 
[ |-N|\ *max* ] 
[ |-S|\ *max* ]
[ |-T|\ *prefix* ]
[ |-W|\ *max* ]
[ GMT_V_OPT ]

|No-spaces|

Description
-----------

**fzinformer** is a script developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It make plots of statistical information obtained
by :doc:1fzanalyzer1 as a function of position along a fracture zone (FZ).

Optional Arguments
------------------

.. _-D:

**-D**
    Use the filtered output from :doc:`fzblender` instead of the raw analysis file in making
    the plot.  This requires that you ran :doc:`fzblender` with the **-D** option.

.. _-F:

**-F**\ *max*
    Sets the maximum *F*-statistic amplitude for the plot [10000].  A logarithmic
    scale is used for this panel; all others are linear.

.. _-I:

**-I**\ *profile*
    By default we plot all the cross-profiles in one stack.  To select a single
    profile only, append the running number of the profile, where 0 is the first profile.

.. _-N:

**-N**\ *max*
    Sets the maximum range of VGG amplitudes (in Eotvos) for the plot [200].

.. _-S:

**-S**\ *max*
    Sets the maximum (±) half-range of FZ offsets (in km) [25].

.. _-T:

**-T**\ *prefix*
    Sets the file name prefix used when running :doc:`fzanalyzer` and :doc:`fzblender`
    [The default is *fztrack*].  The files used here are *prefix*_analysis.txt
    (or *prefix*_filtered.txt if |-D| is used) and *prefix*_blend.txt.

.. _-W:

**-W**\ *max*
    Sets the maximum range of FZ widths (in km) [50].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ *acut*/*vcut*/*fcut*/*wcut*
    We will attempt to assign a single quality index *Q* that summarize how good we
    believe a model fit to be.  This assignment relies of four threshold values
    that need to be determined empirically.  Here, *a_cut* is the minimum peak-to-trough amplitude
    (in Eotvos) of a model for the crossing profile [25], *v_cut* is the minimum
    variance reduction offered by the model (in %) [50], *f_cu* is
    the minimum F statistic computed for the model [50], and *w_cut* is a typical
    FZ trough width (in km) [15].  Currently, the first three quantities
    are used to arrive at a 5-level quality index (0-1) for fitted models, as follows: (1) Very Good: Requires
    model parameters to exceed all three thresholds; (0.75) Good: Requires amplitude and
    variance reduction to exceed thresholds; (0.5) Fair: Requires the variance reduction only
    to exceed its threshold; (0.25) Poor: Requires the amplitude only to exceed its threshold;
    and (0) Bad: None of the criteria were met.  We compute separate quality indices for the
    trough and blend models.  For the empirical trough model we only have estimates or peak-to-trough
    amplitude, IT(A), and trough width, IT(W).  Here, we form the ratio (*A*/*a_cut*) over
    (*W*/*w_cut*), take :math:`\tan^{-1}` of this ratio and scale the result to yield the range 0-1 rounded
    to the nearest multiple of 0.25.

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-do.rst_

.. include:: ../../explain_-icols.rst_

.. include:: ../../explain_-ocols.rst_

.. include:: ../../explain_-q.rst_

.. include:: ../../explain_help.rst_

Plot Features
-------------

**fzinformer** packs much information into each plot by using different symbols and
colors.  Empirical information obtained from raw data are shown in red.  Information
derived from a forced trough FZ model are shown in green, while the information derived
from the optimal blend model are shown in blue.  We present 7 panels for each FZ. Panel
1 (top) shows how the *F*-statistic parameter varies with distance for the trough (green)
and blend (blue) models. Panel 2 shows the reduction in variance for the same two models.
Panel 3 shows the maximum amplitude for the two models and the empirical data (red).
Panel 4 shows the width of the FZ signal for all three data.  Panel 5 presents the offset
(in km) between the digitized trace and the optimal FZ locations (one curve for each type).
Panel 6 shows which side (left is -1, right = +1) is the young side assuming a Pacific
edge-anomaly model (it will tend to jump back and forth where the signal is close to
symmetric and should only be used when we have clearly asymmetric signals). Finally, panel 7 shows
the compression parameter **C for the blend and trough models, as well as the blend parameter *A*
(black line) for the optimal blend model.

Examples
--------

To look at the statistics for the 5th (0 is first) FZ analyzed as part of a larger group called traces,
accepting default values except we override the maximum amplitude by using 100,
try::

    fzinformer -Ttraces -N100 -I5

The statistical plot will be named *prefix*_stat.pdf.

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzblender </supplements/gsfml/fzblender>`
:doc:`mlconverter </supplements/gsfml/mlconverter>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`,
:doc:`grdtrack </grdtrack>`,
