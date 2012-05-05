***********
gmtdefaults
***********


gmtdefaults - List current **GMT** default parameters

`Synopsis <#toc1>`_
-------------------

**gmtdefaults** [ **-D**\ [**u**\ \|\ **s**] ]

`Description <#toc2>`_
----------------------

**gmtdefaults** lists the **GMT** parameter defaults if the option
**-D** is used. There are three ways to change some of the settings: (1)
Use the command **gmtset**, (2) use any texteditor to edit the file
**gmt.conf** in your home, ~/.gmt or current directory (if you do not
have this file, run **gmtset** **-D** to get one with the system default
settings), `or (3) <or.3.html>`_ override any parameter by specifying
one or more **--PARAMETER**\ =\ *VALUE* statements on the commandline of
any **GMT** command (**PARAMETER** and *VALUE* are any combination
listed below). The first two options are permanent changes until
explicitly changed back, while the last option is ephemeral and only
applies to the single **GMT** command that received the override.
**GMT** can provide default values in US or SI units. This choice is
determined at compile time.

`Required Arguments <#toc3>`_
-----------------------------

None.

`Optional Arguments <#toc4>`_
-----------------------------

**-D**
    Print the system **GMT** defaults to standard output. Append **u**
    for US defaults or **s** for SI defaults. [**-D** alone gives the
    version selected at compile time; If **-D** is omitted, the user’s
    currently active defaults are printed.]
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
    Your currently active defaults come from the **gmt.conf** file in
    the current working directory, if present; else from the
    **gmt.conf** file in your home directory, if present; else from the
    file **~/.gmt/gmt.conf** if present; else from the system defaults
    set at the time **GMT** was compiled.

`**GMT** PARAMETERS <#toc5>`_
-----------------------------

Read the **gmt.conf** man page for a full list of the parameters that
are user-definable in **GMT**.

`Examples <#toc6>`_
-------------------

To get a copy of the **GMT** parameter defaults in your home directory,
run

**gmtdefaults** **-D** > ~/gmt.conf

You may now change the settings by editing this file using a text editor
of your choice, or use **gmtset** to change specified parameters on the
command line.

`Bugs <#toc7>`_
---------------

If you have typographical errors in your **gmt.conf** file(s), a warning
message will be issued, and the **GMT** defaults for the affected
parameters will be used.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmt.conf*\ (5) <gmt.conf.5.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*gmtget*\ (1) <gmtget.1.html>`_ , `*gmtset*\ (1) <gmtset.1.html>`_

