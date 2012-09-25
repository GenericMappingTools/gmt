******
isogmt
******

isogmt - Run GMT command or script in isolation mode

`Synopsis <#toc1>`_
-------------------

**isogmt** *command*

`Description <#toc2>`_
----------------------

**isogmt** runs a single **GMT** command or shell script in **isolation
mode**. This means that the files *.gmtcommands* and *gmt.conf* will be
read from the usual locations (current directory, *~/.gmt*, or home
directory), but changes will only be written in a temporary directory,
which will be removed after execution. The name of the temporary
directory will be available to the command or script as the environment
variable **GMT\_TMPDIR**.

`Examples <#toc3>`_
-------------------

Run the shell script *script.gmt* in isolation mode

isogmt sh script.gmt

`See Also <#toc4>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_
