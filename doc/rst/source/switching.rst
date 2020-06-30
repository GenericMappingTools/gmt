Switching between different versions
====================================

We encourage all GMT users to start using version 5 immediately; it
has been tested extensively by the GMT team and has benefitted from
bug reports for the 4.5.x versions. Users who still worry about the new
version breaking things may install GMT 4.5.x and 5 side by side.

Because GMT 5 is backwards compatible with the 4.5.x series
(provided you configured it that way) yet maintains its parameters and
history in separate files (e.g. ``.gmtdefaults4``, versus ``gmt.conf``)
it is possible to install and use both versions on the same workstation.
Switching between different GMT versions can be accomplished in several
ways, two of which will be addressed here:

#. By using the :doc:`gmtswitch` utility to select the current working version.
   **Pro:** easy, interactive way to switch versions on the command line; works with
   previous GMT syntax. **Con:** editing of shell startup files required; needs write
   access in ``$HOME``-directory; manual intervention necessary if symlink
   ``$HOME/this_gmt`` is broken.

#. By using the recommended ``gmt <module>``-syntax in conjunction with a shell
   helper function that points to the desired GMT executable. **Pro:** no need to
   create symlinks and edit shell startup files; scripts are more portable.
   **Con:** different syntax required.

Setup of gmtswitch
------------------

Run :doc:`gmtswitch` after you have finished installing all
GMT versions of interest. The first time you run :doc:`gmtswitch` it
will try to find all the available versions installed on your file
system. The versions found will be listed in the file ``.gmtversions`` in your home
directory; each line is the full path to a GMT root directory (e.g.,
/usr/local/GMT4.5.9). You may manually add or remove entries there at
any time. You are then instructed to make two changes to your
environment (the details are shell-dependent but explained by :doc:`gmtswitch`):

#. :doc:`gmtswitch` creates and maintains a symbolic link ``this_gmt`` in your home
   directory that will point to a directory with one of the installed
   GMT versions.

#. Make sure ``$HOME/this_gmt/bin`` is in your executable PATH.

Make those edits, logout, and login again. The next time you
run :doc:`gmtswitch` you will be able to switch between versions. Typing
:doc:`gmtswitch` with no argument will list the available versions in a
numerical menu and prompt you to choose one, whereas :doc:`gmtswitch`
*version* will immediately switch to that version (*version* must be a
piece of unique text making up the full path to a version, e.g., 4.5.9).
If you use **bash**, **tcsh**, or **csh** you may have to type ``hash -r`` or
``rehash`` to initiate the path changes.

On Windows, the process is somewhat similar. The GMT bin directory has one batch file
``gmtswitch.bat`` that works by changing the Windows PATH variable so that the BIN
directory of the preferred version always comes first. To do that the batch works in two
alternative modes.

#. Permanent mode

#. Temporary mode

The permanent mode makes use of the free executable program `EditPath
<http://www.softpedia.com/get/Tweak/Registry-Tweak/EditPath.shtml>`_
to change the user path in the registry. It's called permanent because the changes
remains until ... next change.
Off course the editpath.exe binary must be in your system's path as well.
WARNING: The path change will not be visible on the shell cmd where it was executed.
For the change to be active you will need to open a new cmd window.

The second mode is temporary because the path to the selected GMT binary dir is
prepended to previous path via a shell command line. This modification disappears
when the shell cmd window where it was executes is deleted.

For further details the user should read the entire help section at the header of the
``gmtswitch.bat``.

The ``gmtswitch.bat`` solution, however, has the drawback that the batch file must be located
elsewhere and in a directory that is on the user's PATH, otherwise it won't be located after
first use unless the other GMT bin directory has a similar batch file. A better solution is to install the
`Windows console enhancement <http://sourceforge.net/projects/console>`_
that includes multiple tabs and configure the different tabs to start the different GMT versions.
All it takes is in the Tab setting to call a batch that modifies the PATH locally. That PATH
modifying batch will have a single line with something like:

   ::

    set path=C:\programs\gmt5\bin;%PATH%


Version selection with helper function
--------------------------------------

A shell function can be used as a wrapper around the gmt executable. This even
works when a gmt application is in the search PATH as it would shadow the real
command. This method can easily be applied on the command line or in scripts
when the recommended ``gmt <module>``-syntax is used. Shell scripts using
old-style GMT commands would have to be converted first. The syntax conversion
can be accomplished with the :doc:`gmt5syntax` utility. A suitable bash wrapper
function for GMT 5 would look like this:

.. code-block:: bash

   function gmt() { /path/to/gmt5/bin/gmt "$@"; }
   export -f gmt

Exporting the function is necessary to make it available to subshells and
scripts. This wrapper function can be either set in your working shell or
inside a GMT shell script. The latter is useful to switch to a certain GMT
version on a per-script basis.

For GMT releases prior to GMT 5 which only provide the module commands,
we need a slightly modified version of the wrapper script:

.. code-block:: bash

   function gmt() { module=$1; shift; /path/to/gmt4/bin/${module} "$@"; }
   export -f gmt

On the command line this might be too much typing to switch between
versions. So we might as well put everything together in a script file
``gmtfun``:

.. code-block:: bash

   case $1 in
     4)
     function gmt() {
       module=$1; shift; /path/to/gmt4/bin/${module} "$@"
     }
     ;;
     5)
     function gmt() {
       /path/to/gmt5/bin/gmt "$@"
     }
     ;;
     *)
     return
     ;;
   esac
   export -f gmt

Source the file with either ``. gmtfun 4`` or ``. gmtfun 5`` to switch
between versions.
