.. index:: ! gmt5syntax

**********
gmt5syntax
**********

.. only:: not man

    Convert old GMT script to use new 'gmt <module>' syntax

Synopsis
--------

**gmt5syntax** old_script > new_script

Description
-----------

**gmt5syntax** is a perl script that converts old-style GMT commands in,
e.g., shell scripts, to the new ``gmt <module>``-syntax.  This utility is located
in the tools subdirectory of the data directory. ``gmt --show-datadir`` will
show the path to the latter.

See Also
--------

:doc:`gmt`
