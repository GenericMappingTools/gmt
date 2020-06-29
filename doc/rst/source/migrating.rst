Migrating from an earlier version
=================================

Many longtime GMT users have older scripts written in GMT 4 and even in GMT 5
that may not work immediately in GMT 6.  This reason is not that the syntax
of the commands cannot be recognized ("GMT 6 shall be compatible with GMT 4
and GMT 5 syntax"), but because the old scripts do not call the GMT module via
the only installed program called :doc:`gmt`.  Because of problems with "namespace
pollution" (there are many open source programs called surface and triangulate and
it is not possible to place them all in /usr/local/bin, for instance) the Linux
distributions usually will only build the gmt executable and we access all modules
via this single program, using the syntax **gmt module** [*options*].

There are several ways to address the lack of using the gmt program:

#. If you are building GMT from source (either from a checked-out GitHub repo or
   the source distributed via the tarballs, you can edit a parameter in the file
   *cmake/ConfigUser.cmake* and set **GMT_INSTALL_MODULE_LINKS** true.  This will add
   symbolic links called **blockmean**, **pscoast**, **grdimage**, etc. that all point
   to the gmt executable.  Your old scripts will now work without the leading gmt
   program for each module command.

#. You can run the script *gmt_links.sh* which lives in the **share/tools** directory
   (run ``gmt --show-sharedir`` to find the path to share, then look in tools). If
   your GMT installation was installed outside your user directory you will need
   to run the script as root.  It will create all the module links to allow users
   to run modules directly.  You can run the link-building script this way::

     $(gmt --show-sharedir)/tools/gmt_links.sh

   which will report the status of any existing links.  Add the argument *create*
   or *delete* to make actual changes.

#. If your default shell is bash or similar then you can call another **share/tools**
   script called **gmt_functions.sh** this way::

     source $(gmt --show-sharedir)/tools/gmt_functions.sh

   It will instead create bash functions with the
   names of the modules and thus let you run **blockmean**, etc. without a leading
   gmt invocation.

#. If your default shell is csh or similar then you must instead call **gmt_aliases.csh**
   which works similarly to the **gmt_functions.sh** but for csh::

     source `gmt --show-sharedir`/tools/gmt_aliases.csh

   Both of these two solutions can be implemented via your login setup so they are
   always set once you log in to your computer or open a new terminal window.

#. Finally, if the old script is important and is expected to be used in the future,
   maybe it is worth the effort to migrate the script code to the stricter default
   GMT 6 syntax by starting each GMT command with gmt.  If so, also consider to simplify
   the script (assuming if is a plotting script) by rewriting it in GMT modern mode.

If your script is particularly old (GMT 3) it may also use a deprecated syntax that
GMT no longer recognizes.  If that is the case then you should consult the documentation for
the old version (if you have access to it) and compare with the new documentation to
determine the required changes.  Error messages issued by GMT may help in that regard.
Also note that GMT may issue compatibility warnings when it detects deprecated
syntax.  In those instances the obsolete syntax will be correctly parsed, but you
may wish to take the opportunity and update the syntax when these issues are reported.
