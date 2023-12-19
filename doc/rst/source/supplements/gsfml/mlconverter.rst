.. index:: ! mlconverter
.. include:: ../module_supplements_purpose.rst_

***********
mlconverter
***********

|mlconverter_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt mlconverter** [ *ML_data* ]
[ |-A| ]
[ |-G|\ [**s**] ]
[ |-I|\ *FZid* ] 
[ |-T|\ **c**\ |\ **g**\ |\ **o**\ |\ **s** ]
[ GMT_V_OPT ]

|No-spaces|

Description
-----------

**mlconverter** is a module developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see `GSFML <https://www.soest.hawaii.edu/PT/GSFML>`_ for a full
description of the project].  It reads a magnetic pick data file (or stdin)
and converts chron text-strings to ages using a selected magnetic time scale.
The input data must be OGR/GMT data files of the form distributed by the
GSFML project.

Optional Arguments
------------------

*ML_data*
    A magnetic ML pick data OGR/GMT file.  If not given then we read standard input.

.. _-A:

**-A**
    Append the metadata to the output records as additional columns [Default only
    writes *lon*, *lat*, *age* records].

.. _-G:

**-G**\ [**s**]
    Generate an extended OGR/GMT table by appending the crustal age.
    Append **s** to repair any lax chron nomenclature, if needed.

.. _-T:

**-Tc**\ |\ **g**\ |\ **o**\ |\ **s**
    Select the magnetic time scale to use.  Choose from **c** (Cande and Kent, 1995),
    **g** (Gee and Kent, 2007), **o** (Ogg, 2012), or **s** (Gradstein, 2004) [**g**].

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
    the minimum *F* statistic computed for the model [50], and *w_cut* is a typical
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

Examples
--------

To convert chrons to ages using the Cande and Kent, 1995 timescale, and append the
metadata at the end of the record, try::

    gmt mlconverter -A -Tc ML_datafile.gmt > convertedfile.txt

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzblender </supplements/gsfml/fzblender>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`fzmodeler </supplements/gsfml/fzmodeler>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`

References
----------

Wessel, P., Matthews, K. J., Müller, R. D., Mazzoni, A., Whittaker, J. M., Myhill, R., Chandler, M. T.,
2015, "Semiautomatic fracture zone tracking", *Geochem. Geophys. Geosyst.*, 16 (7), 2462–2472.
https://doi.org/10.1002/2015GC005853.
