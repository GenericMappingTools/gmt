.. index:: ! fzmodeler
.. include:: ../module_supplements_purpose.rst_

*********
fzmodeler
*********

|fzmodeler_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt fzmodeler** [ |-A|\ *asymmetry* ]
[ |-C|\ *compression* ]
[ |-D|\ *min*/*max*/*inc* ]
[ |-F|\ *prefix* ]
[ |-M| ]
[ |-N|\ *amplitude* ]
[ |-O| ]
[ |-P| ]
[ |-S|\ *shift* ]
[ GMT_V_OPT ]
[ |-W|\ *width* ]

|No-spaces|

Description
-----------

**zmodeler** is a script developed as part of the Global Seafloor Fabric
and Magnetic Lineation Project [see www.soest.hawaii.edu/PT/GSFML for a full
description of the project].  It builds a synthetic model cross-profile given
the chosen model parameters and optionally images the profile via a PDF plot.

Optional Arguments
------------------

.. _-A:

**-A**\ *asymmetry*
    Sets the asymmetry parameter used for the blend between symmetric ("Atlantic")
    and asymmetric ("Pacific") signals [0].

.. _-C:

**-C**\ *compression*
    Sets the amount of compression (0-1) to use in the blending [0].

.. _-D:

**-D**\ *min*/*max*/*inc*
    Sets the domain for which to evaluate the model.  If |-M| is used then
    the domain is expected to be in km; otherwise *min* and *max* will be
    expected to be in degrees of latitude which *inc* will be decoded as
    arc minutes [-5/5/2 or -100/100/2, depending on |-M|].

.. _-F:

**-F**\ *prefix*
    Set the output prefix for the model profile [fzprof].  Give |-F|\ **-** to send
    the model profile to stdout.

.. _-M:

**-M**
    The chosen domain (|-D|) is given degrees of latitude, with increment in arc minutes
    [Default is in km].

.. _-N:

**-N**\ *amplitude*
    Sets the peak-to-trough amplitude of the blended signal [100].

.. _-O:

**-O**
    Instead of making a stand-alone PDF plot, write a PostScript overlay to stdout,
    i.e., make the plot using the GMT **-O -K** options.  Requires (or sets) |-P|.

.. _-P:

**-P**
    Produce a PDF plot (named *prefix*.pdf) of the synthetic FZ profile [no plot].

.. _-S:

**-S**\ *shift*
    Sets the shift of the FZ location from the origin, in km [0].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *width*
    Sets the full width of the FZ, in km [25].
 
Examples
--------

To plot a synthetic profile for the Pacific, using otherwise default arguments,
try::
    fzmodeler -C1 -V

The final plot will be named *prefix*.pdf, with the model data in *prefix*.txt.

See Also
--------

:doc:`gmt </gmt>`
:doc:`fzanalyzer </supplements/gsfml/fzanalyzer>`,
:doc:`fzblender </supplements/gsfml/fzblender>`
:doc:`mlconverter </supplements/gsfml/mlconverter>`,
:doc:`fzinformer </supplements/gsfml/fzinformer>`,
:doc:`fzprofiler </supplements/gsfml/fzprofiler>`,
:doc:`fzmapper </supplements/gsfml/fzmapper>`,
:doc:`grdtrack </grdtrack>`
