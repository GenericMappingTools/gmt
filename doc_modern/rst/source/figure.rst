.. index:: ! figure

******
figure
******

.. only:: not man

    Set attributes for the current figure

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt figure** *prefix* [*formats*] [*options*] [ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

A GMT modern session can make any number of illustrations.  In this situation,
the figure command tells GMT the name and format(s) to use for the current plot.
It must be issued before you start plotting to the current figure, and each
new call terminates the previous figure and changes focus to the next figure.
In addition to *prefix* and *formats*, you can supply a comma-separated series of
:doc:`psconvert` options that should override the default settings provided via
:ref:`PS_CONVERT <PS_CONVERT>`. The only other available option controls verbosity.

Required Arguments
------------------

*prefix*
    Name-stem used to construct final figure names,  Extensions are appended
    automatically from your *formats* selection.

Optional Arguments
------------------

.. _figure-formats:

*formats*
    Give one or more comma-separated graphics extensions from the allowable graphics
    :ref:`formats <tbl-formats>` [pdf].

.. _figure-options:

*options*
    Sets one or more comma-separated options arguments that
    will be passed to :doc:`psconvert` when preparing this figure [A,P].
    The valid subset of options are
    **A**\ [*args*],\ **C**\ *args*,\ **D**\ *dir*,\ **E**\ *dpi*,\ **P**\ ,\ **Q**\ *args*,\ **S**.
    See the :doc:`psconvert` documentation for details on these options.

.. _figure-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

Examples
--------

To start a new figure in your session by the name Regional, and request we make both
a PDF and and EPS file, try

   ::

    gmt figure Regional pdf,eps

To start a new figure GlobalMap that should be returned as a JPEG file with a 1 cm padding
around the image, tr¥

   ::

    gmt figure GlobalMap jpg A1c

See Also
--------

:doc:`begin`,
:doc:`clear`,
:doc:`end`,
:doc:`subplot`,
:doc:`gmt`
