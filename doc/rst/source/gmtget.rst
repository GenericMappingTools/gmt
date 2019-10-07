.. index:: ! gmtget

***
get
***

.. only:: not man

    Get individual GMT default parameters

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt get** [ |-G|\ *defaultsfile* ] [ |-L| ] *PARAMETER1* [ *PARAMETER2* *PARAMETER3* ... ]

|No-spaces|

Description
-----------

**gmt get** will list the value of one or more GMT default parameters.

Required Arguments
------------------

PARAMETER
    Provide one or several parameters of interest. The current value of
    those parameters will be written to *stdout*. For a complete listing
    of available parameters and their meaning, see the :doc:`gmt.conf` man page.

Optional Arguments
------------------

.. _-G:

**-G**\ *defaultsfile*
    Name of specific :doc:`gmt.conf` file to read [Default looks first in
    current directory, then in your home directory, then in ~/.gmt and
    finally in the system defaults].

.. _-L:

**-L**
    Return the values of the parameters on separate lines [Default
    returns all selected parameter values on one line separated by
    spaces]

Example
-------

To list the value of the parameter PS_COMMENTS:

   ::

    gmt get PS_COMMENTS

To get both the values of the parameter
MAP_GRID_CROSS_SIZE_PRIMARY and MAP_GRID_CROSS_SIZE_SECONDARY on one line, try

   ::

    gmt get MAP_GRID_CROSS_SIZE_PRIMARY MAP_GRID_CROSS_SIZE_SECONDARY

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmtdefaults`,
:doc:`gmtset`
