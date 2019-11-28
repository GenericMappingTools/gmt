.. index:: ! gmtswitch

*********
gmtswitch
*********

.. only:: not man

    Switching between different GMT versions

Synopsis
--------

**gmtswitch** [ D \| *version* ]

Introduction
------------

**gmtswitch** helps you modify your environment to allow for the
switching back and forth between several installed GMT versions. It works by
maintaining a list of directories to GMT installations in a file in your
home directory, then manipulates a symbolic link to point to the GMT
directory whose executables we wish to use [The Windows version works a
bit differently; see WINDOWS below].

Required Arguments
------------------

None. If no arguments are given you are presented with a menu of
installed GMT versions from 1 to *n* and you specify which one you wish
to switch to.

Optional Arguments
------------------

**D**
    Select the default GMT version. This is the first entry in the
    ~/.gmtversions file
*version*
    Search for a unique match in the ~/.gmtversions file. If one match
    is found we switch to that entry; otherwise an error is generated.
    where module is the name of a GMT program and the options are
    those that pertain to that particular program.

Setup
-----

If you have official versions installed then running gmtswitch the very
first time will examine your hard disk starting at / and look for
directories with GMT4, GMT5, etc., in the names. This will fail to find the
subversion directories and possibly others you have placed elsewhere.
The fastest way to get up and running is this:

1. Edit/Create ~/.gmtversions and add the paths to all GMT installations
    you have or care to consider. Each path goes on separate lines and
    points to the top dir of each distribution, e.g.,
    /Users/pwessel/UH/RESEARCH/PROJECTS/GMTdev/GMT4.5.7

2. In your .bashrc or .[t]csrh or wherever you are maintaining your PATH
    or path variable, remove any directories you have added that contain
    GMT, and add the new path $HOME/this\_gmt/bin (might be $home for csh users).
    Make sure this path appears before any others that might contain a GMT
    installation, such as those used by package managers (e.g., /sw/bin for
    fink, /opt/local/bin for Macports, etc.).

3. Make the new path take effect (quit/restart terminal, logout/login, etc).

4. cd to the most recent GMT directory where a gmtswitch version lives,
    and run gmtswitch with no argument. Select one of the version from the
    menu.

5. If in csh you may have to say rehash afterwards.

6. Type "plot -" and the synopsis should tell you that you got the
    correct version. You can now run gmtswitch from anywhere; try it out and
    make sure that you can switch between the versions.

Examples
--------

To switch to GMT version 4.5.7 (assuming it was installed as such and not
via a package manager), try

  ::

    gmtswitch GMT4.5.7

To switch to the default (your top choice), do

  ::

    gmtswitch D

Finally, to select from the menu, just run

  ::

    gmtswitch

and pick the one you want.

Beware
------

GMT remembers where it was installed the first time and uses that dir to
find the default GMT share directory. If you move entire GMT
installation after compilation then you may have to set GMT\_SHAREDIR to
point to the top dir in order for things to work. It is best not to move
things after installation.

Windows
-------

Under Windows use gmtswitch.bat which is a batch script that changes the
Windows PATH variable so that the BIN directory of the preferred version
always comes first. To do that the batch works in two alternative modes:

1 - Permanent mode

2 - Temporary mode

The permanent mode makes use of the free executable program "EditPath"
to change the user path in the registry. It's called permanent because
the changes remains until ... next change. See

`http://www.softpedia.com/get/Tweak/Registry-Tweak/EditPath.shtml <http://www.softpedia.com/get/Tweak/Registry-Tweak/EditPath.shtml>`_

Of course the editpath.exe binary must be in your system's path as well.
WARNING: The path change will not be visible on the shell cmd where it
was executed. For the change to be active you will need to open a new
cmd window.

The second mode is temporary because the path to the selected GMT binary
dir is prepended to the previous path via a shell command line. This
modification disappears when the shell cmd window where it was executes
is deleted.

It is the user responsibility to set the contents of the G32\_32 to
G5\_64 below to valid paths where the binaries of the different GMT
versions are installed Note that it is not mandatory to have all four of
them in you computer. For the ones you do not have just let them
pointing to nothing e.g.,

set G4\_64=

The permanent mode is the default one (but this can be changed. See edit
section) To run in the temporary mode just give a second argument
(doesn't matter what)

Example usage to set a GMT5 64 bits permanent

gmtswitch g5\_64

To temporary set a GMT4 32 bits do

gmtswitch g4\_32 1

Run without arguments to get a "Usage" (for permanent mode)
