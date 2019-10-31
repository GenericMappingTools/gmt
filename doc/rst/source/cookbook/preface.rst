Preface
=======

While GMT has served the map-making and data processing needs of
scientists since 1988 [1]_, the current global use was heralded by the
first official release in *EOS Trans. AGU* in the fall of 1991. Since
then, GMT has grown to become a standard tool for many users,
particularly in the Earth and Ocean Sciences but the global collective
of GMT users is incredibly diverse. Development has at times been
rapid, and numerous releases have seen the light of day since the early
versions. For a detailed history of the changes from release to release,
see :doc:`/changes`. For a nightly snapshot of ongoing
activity, see the `GMT repository on GitHub <https://github.com/GenericMappingTools/gmt>`__.
For a historical perspective of the origins and development of GMT,
see the video podcast `"20 Years with GMT – The Generic Mapping Tools" <https://doi.org/10.5446/19869>`__
produced following a seminar given by Paul Wessel on the 20th anniversary of GMT.

The success of GMT is to a large degree due to the input of the user
community. In fact, most of the capabilities and options in the
GMT modules originated as user requests. We would like to hear from
you should you have any suggestions for future enhancements and
modification. Please submit a bug report or a feature request
on GitHub (`<https://github.com/GenericMappingTools/gmt/issues>`_).

.. _command-line-completion:

Command-line completion
-----------------------

GMT provides basic command-line completion (tab completion) for bash.
The easiest way to use the feature is to install the
`bash-completion <https://github.com/scop/bash-completion/>`_ package
which is available in many operating system distributions.

Depending on the distribution, you may still need to source it from
``~/.bashrc``, e.g.:

.. code-block:: bash

   # Use bash-completion, if available
   if [ -r /usr/share/bash-completion/bash_completion ]; then
     . /usr/share/bash-completion/bash_completion
   fi

When you install GMT from a distribution package, the completion rules
are installed in ``/etc/bash_completion.d/gmt`` and loaded automatically.
Custom GMT builds provide ``<prefix>/share/tools/gmt_completion.bash``
which needs to be manually sourced from either ``~/.bash_completion`` or
``~/.bashrc``.

Mac users should note that bash-completion >=2.0 requires bash >=4.1.
However, OS X won't ship anything that's licensed under GPL version 3.
The last version of bash available under the GPLv2 is 3.2 from 2006.
It is recommended that *bash-completion* is installed together with
*bash* via `MacPorts <http://www.macports.org/>`_,
`Fink <http://finkproject.org/>`_, or `Homebrew <http://brew.sh/>`_.
You then need to change the shell used by your terminal application.
The `bash-completion HOWTO from MacPorts
<http://trac.macports.org/wiki/howto/bash-completion>`_
explains how to change the preferences of Terminal.app and iTerm.app.
Another way is to change the default shell by editing of the user
database:

.. code-block:: bash

   Add /opt/local/bin/bash to /etc/shells
   chsh -s /opt/local/bin/bash

Modify the path to bash, ``/opt/local/bin/bash``, in the example above
accordingly.

.. [1]
   Version 1.0 was then informally released at the Lamont-Doherty Earth Observatory.
