.. index:: ! gmtdefaults
.. include:: module_core_purpose.rst_

********
defaults
********

|gmtdefaults_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt defaults** [ |-D|\ [**s**\|\ **u**] ]

|No-spaces|

Description
-----------

**defaults** lists all the GMT parameter defaults if the option |-D| is used.
There are three ways to change some of the settings: (1) use the command
:doc:`gmtset`, (2) use any text editor to edit the file :doc:`gmt.conf` in your
home, ~/.gmt or current directory (if you do not have this file, run
:doc:`gmtdefaults` |-D| > gmt.conf to get one with the system SI default
settings), or (3) override any parameter by specifying one or more
**-**\ **-PARAMETER**\ =\ *VALUE* statements on the command line of any GMT
command (**PARAMETER** and *VALUE* are any combination in :doc:`gmt.conf`). In
modern mode, settings can be reset to GMT defaults using :doc:`clear`. Changes
made using :doc:`gmtset` are permanent until explicitly changed back in classic
mode and are in effect for the duration of the current session in modern mode.
Manual changes to the :doc:`gmt.conf` file (option 2) are in effect until
explicitly changed back, or can be ignored for a modern mode session using the
**-C** argument of the :doc:`begin` module. Option 3 is ephemeral and only
applies to the single GMT command that received the override. GMT can provide
default values in US or SI units based on the choice determined at compile time.

Required Arguments
------------------

None.

Optional Arguments
------------------

.. _-D:

**-D**\ [**s**\|\ **u**]
    Print the system GMT defaults settings to standard output. Append **u**
    for US defaults or **s** for SI defaults. [|-D| alone gives the
    SI defaults; If |-D| is omitted, the user's
    currently active defaults are printed.]

.. include:: explain_help.rst_

Your currently active defaults come from the :doc:`gmt.conf` file in
the current working directory (in classic mode) or in your session directory
(in modern mode), if present; else from the
:doc:`gmt.conf` file in your home directory, if present; else from the
file **~/.gmt/gmt.conf** if present; else from the system defaults
set at the time GMT was compiled.

GMT PARAMETERS
--------------

Read the :doc:`gmt.conf` man page for a full list of the parameters that
are user-definable in GMT.

Examples
--------

To get a copy of the GMT parameter SI defaults in your home directory, run::

    gmt defaults -D > ~/gmt.conf

You may now change the settings by editing this file using a text editor
of your choice, or use :doc:`gmtset` to change specified parameters on the
command line.

Bugs
----

If you have typographical errors in your :doc:`gmt.conf` file(s), a warning
message will be issued, and the GMT defaults for the affected
parameters will be used.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`, :doc:`gmtget`,
:doc:`gmtset`
