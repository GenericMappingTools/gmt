.. index:: ! isogmt

******
isogmt
******

.. only:: not man

    isogmt - Run GMT command or script in isolation mode [classic mode only]

Synopsis
--------

**isogmt** *command*

Description
-----------

**isogmt** runs a single GMT command or shell script in **isolation
mode**. This means that the files *gmt.history* and *gmt.conf* will be
read from the usual locations (current directory, *~/.gmt*, or home
directory), but changes will only be written in a temporary directory,
which will be removed after execution. The name of the temporary
directory will be available to the command or script as the environment
variable **GMT_TMPDIR**.

Examples
--------

Run the shell script *script.gmt* in isolation mode::

    isogmt sh script.gmt

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`
