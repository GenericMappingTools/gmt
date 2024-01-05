.. index:: ! fzblender
.. include:: ../module_supplements_purpose.rst_

*********
fzblender
*********

|fzblender_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt fzblender** [ |-D| ]
[ |-E|\ *sfilter* ]
[ |-F|\ *pfilter* ]
[ |-I|\ *FZid* ] 
[ |-Q|\ *q_min*/*q_max* ]
[ |-S|\ **b**\|\ **d**\|\ **e**\|\ **t**\|\ **u**\ [*weight*] ] 
[ |-T|\ *prefix* ]
[ |SYN_OPT-V| ]
[ |-Z|\ *acut*/*vcut*/*fcut*/*wcut* ]

|No-spaces|

Description
-----------

**fzblender** is a tool developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It reads an analysis file produced by
:doc:`fzanalyzer` and optionally filters those results along track.  Then, given the
specified signal codes we will produce an optimal FZ track that is a
weighted blend between the user's original digitized trace and one or more
of the model traces obtained by :doc:`fzanalyzer`.  The blend is based on
quality indices determined for the model traces: If the quality index is
high we favor this track, else we favor the digitized line; in between values
leads to a weighted blend.  We expect to read the analysis results from the
file *prefix*\ _analysis.txt produced by :doc:`fzanalyzer`; the blend results
will be written to file *prefix*\ _blend.txt.  Optionally, the intermediate
filtered analysis file can be written to *prefix*\ _filtered.txt if |-D| is given.

Optional Arguments
------------------

.. _-D:

**-D**
    Do not remove filtered output but save them to *prefix*\_filtered.txt. [By default we delete these intermediate files].

.. _-E:

**-E**\ *sfilter* 
    Apply a secondary filter after the primary required filter has completed.
    This is sometimes useful if you apply a robust filter first, which may result in
    short length-scale noise after removing gross outliers.  See |-F| for how to specify the filter.

.. _-F:

**-F**\ *pfilter*
    Sets the along-track primary filter.  Choose among convolution and non-convolution filters.
    Append the filter directive followed by the full (6-sigma) *width*. Available convolution filters are:

    - **b**: Boxcar: All weights are equal.
    - **c**: Cosine Arch: Weights follow a cosine arch curve.
    - **g**: Gaussian: Weights are given by a Gaussian function.

    Non-convolution filters are:

    - **m**: Median: Returns median value.
    - **p**: Maximum likelihood probability (a mode estimator): Return modal value.
      If more than one mode is found we return their average value.  Append **+l** or **+u** to
      the filter width if you rather want to return the lowermost or uppermost of the modal
      values.
    - **l**: Lower: Return the minimum of all values.
    - **L**: Lower: Return minimum of all positive values only.
    - **u**: Upper: Return maximum of all values.
    - **U**: Upper: Return maximum or all negative values only.

    In the case of **L**\|\| **U** it is possible that no data passes the initial sign test;
    in that case the filter will return 0.0.

.. _-I:

**-I**\ *FZid*
    By default, we will analyze the cross-profiles generated for all FZs.  However,
    you can use |-I| to specify a particular *FZid* (first *FZid* is 0).

.. _-Q:

**-Q**\ *q_min*/*q_max*
    Sets the range of quality indices that will be used in the blended result.
    The quality index *q(d)* ranges from *q_min*) (0 or bad) to *q_max* (1 or very good) and varies
    continuously with distance *d* along the FZ trace.  The quality weight assigned to
    the modeled FZ trace is *w_q(d)* = (*q(d)* - *q_min*)/(*q_max* - *q_min*)),
    unless *w_q(d)* > *q_max*) (*w_q(d)* = 1) or *w_q(d)* < *q_min*) (*w_q(d)* = 0).  You can use the |-Q|
    option to change this weight assignment.  The quality weight assigned to the digitized FZ trace is
    *w_q(d)* = 1 - mean{model quality weights} (see |-S|).  For the calculation of quality indices, see |-Z|.

.. _-S:

**-Sb**\|\ **d**\|\ **e**\|\ **t**\|\ **u**\ [*weight*]
    Specify the model and data traces you wish to blend and the relative custom weights
    of each [Defaults to 1 for all traces].  Repeat this option for each trace to consider.
    If you specify more than one model trace then the models are first averaged according to their quality
    indices and weights before blending with the digitized trace (if specified).  Hence, the quality index assigned
    to the digitized trace is *q_r* = 1 - mean(model quality indices).  The final blend is thus a weighted
    average that takes into account both the quality indices and the custom weights (if specified).
    Choose among these directives:
    
    - **b**: The trough location for the optimal trough/edge model blend model.  This is the best fit obtained to the data using
      a blend of "Atlantic"-, "Pacific"-, and "Compression"-style synthetic shapes.
    - **d**: This is the empirical picks of the trough locations along the trace.
    - **e**: This is the location of maximum slope for the optimal trough/edge model blend model.
    - **t**: This is the best fit using the "Atlantic"-style trough model only.
    - **u**: The user's original digitized trace.

    In addition to the blended FZ locations, we also output estimates of the FZ width and the traces
    of the 1-sigma boundaries on either side of the FZ.

.. _-T:

**-T**\ *prefix*
    Sets the file name prefix used for all output files [fztrack].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: ../../explain_-V.rst_
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
    amplitude, *A*, and trough width, *W*.  Here, we form the ratio (*A*/*a_cut*) over
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

.. include:: ../../explain_precision.rst_

Output Columns
--------------

**fzblender** reports 10 columns of data, which are *lon, lat, dist, shift, width,
qweight, lon_l, lat_l, lon_r, lat_r*, where *lon, lat* contain the blended track
along *dist*, with across-track *widt* and *shift* in origin.  The blend
obtained a quality weight of *qweight*, and the four last columns contains the
coordinates for the left/right bounds along the FZ.

Filtering
---------

Filtering always runs of of data near the FZ end points.  We utilize :doc:`filter1d </filter1d>` with its
**-E** option to extend the result to the end.  Because we are filtering data columns that may
contain a strong trend (e.g., longitude versus along-track distance) we first remove such
linear trends before filtering, then restore the trends before blending.  However, you should
be cautions in interpreting the blended results close to the ends of the FZs.  You can examine
the effect of filtering more directly by using the |-D| option to save the filtered profiles.

Blend Considerations
--------------------

Note that of the various directives in |-S|, the **e** is different in that it reflects
the FZ location estimate based on the theoretical prediction that the FZ crossing may be
associated with the steepest VGG slope.  As such it will be offset from the trough by 
several km (unless the blend is mostly "Atlantic") and combining it with the others
is unlikely to be productive.  It is best used by itself with filtering.

Examples
--------

To produce a weighted average of your digitized trace, the empirical trough locations,
and the trough model locations, giving the empirical locations a weight of 2 and the
model troughs a weight of 1, reading the file Pac_analysis.txt and selecting a median
filter of 70 km width followed by a 50-km Gaussian filter, try::

    gmt fzblender -Su1 -Sd2 -St1 -Fm70 -Eg50 -TPac

To produce a smooth trace of the maximum slope locations along track for the
same file, we try the same filters with the command::

    gmt fzblender -Se -Fm70 -Eg50 -TPac

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`,
:doc:`filter1d </filter1d>`,
:doc:`mlconverter </supplements/gsfml/mlconverter>`

References
----------

Wessel, P., Matthews, K. J., Müller, R. D., Mazzoni, A., Whittaker, J. M., Myhill, R., Chandler, M. T.,
2015, "Semiautomatic fracture zone tracking", *Geochem. Geophys. Geosyst.*, 16 (7), 2462–2472.
https://doi.org/10.1002/2015GC005853.
