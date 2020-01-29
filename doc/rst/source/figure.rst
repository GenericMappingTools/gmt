.. index:: ! figure
.. include:: module_core_purpose.rst_

******
figure
******

|figure_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt figure** *prefix* [*formats*] [*options*]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

A GMT modern session can make any number of illustrations (including none).
In situations when multiple illustrations will be made during a single session ,
the **figure** module is used to specify the name and format(s) to use for the next plot.
It must be issued before you start plotting to the intended figure, and each
new **figure** call changes plotting focus to the next figure.  You may go back and forth between
different figures but the optional arguments (*formats* and *options*) can only
be given the first time you specify a new figure.
In addition to *prefix* and *formats*, you can supply a comma-separated series of
:doc:`psconvert` *options* that will override the default settings provided via
:term:`PS_CONVERT` [**A**]. The only other available options control the verbosity.
Each figure maintains its own history and settings, so
memory of region and projection settings only apply on a per figure basis.

Required Arguments
------------------

*prefix*
    Name stem used to construct the figure name.  The extension(s) are appended
    automatically from your *formats* selection(s).
    While not recommended, if your *prefix* has spaces in it then you must enclose your
    prefix in single quotes.

Optional Arguments
------------------

.. _figure-formats:

*formats*
    Give one or more comma-separated graphics extensions from the list of allowable graphics
    :ref:`formats <tbl-formats>` (default is configurable via setting GMT_GRAPHICS_FORMAT [pdf]).

.. _figure-options:

*options*
    Sets one or more comma-separated options (and possibly arguments) that
    can be passed to :doc:`psconvert` when preparing this figure [**A**].
    The valid subset of options are
    **A**\ [*args*],\ **C**\ *args*,\ **D**\ *dir*,\ **E**\ *dpi*,\ **H**\ *factor*,\ **M**\ *args*,\ **Q**\ *args*,\ **S**.
    See the :doc:`psconvert` documentation for details on these options.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help_nopar.rst_

.. include:: explain_postscript.rst_

Examples
--------

To start a new figure in your current modern mode session by the name Regional and
request we make both a PDF and an EPS file, try::

    gmt begin
    gmt figure Regional pdf,eps
    gmt ...
    gmt end show

To start a new figure GlobalMap that should be returned as a JPEG file with a 1 cm padding
around the image, try::

    gmt begin
    gmt figure GlobalMap jpg A+m1c
    gmt ...
    gmt end show

If the same figure were to be called Global Map.jpg you would need quotes::

    gmt begin
    gmt figure 'Global Map' jpg A+m1c
    gmt ...
    gmt end show

To make two figures in one session and go back and forth between different figures::

    gmt begin

    # Activate figure Fig1
    gmt figure Fig1 pdf
    gmt ...

    # Activate figure Fig1
    gmt figure Fig2 pdf
    gmt ...

    # Go back to figure Fig1
    gmt figure Fig1
    gmt ...

    # Go back to figure Fig2
    gmt figure Fig2
    gmt ...

    gmt end show

Technical Note
--------------

If you are calling **figure** from an external environment and you do not want the plot to
be converted automatically when the session ends, perhaps because you wish to do this yourself,
you can specify the file name or figure format as - (i.e., just a hyphen).

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`docs`,
:doc:`end`,
:doc:`inset`,
:doc:`subplot`,
:doc:`gmt`
