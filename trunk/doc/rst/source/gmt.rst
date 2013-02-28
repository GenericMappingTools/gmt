***
gmt
***

gmt - The Generic Mapping Tools data processing and display software
package

`Introduction <#toc1>`_
-----------------------

**GMT** is a collection of public-domain Unix tools that allows you to
manipulate x,y and x,y,z data sets (filtering, trend fitting, gridding,
projecting, etc.) and produce *PostScript* illustrations ranging from
simple x-y plots, via contour maps, to artificially illuminated surfaces
and 3-D perspective views in black/white or full color. Linear, log10,
and power scaling is supported in addition to over 30 common map
projections. The processing and display routines within **GMT** are
completely general and will handle any (x,y) or (x,y,z) data as input.

`Synopsis <#toc2>`_
-------------------

**gmt** is a wrapper program that can start any of the programs:

**gmt** module module-options

where module is the name of a **GMT** program and the options are those
that pertain to that particular program.

`Gmt Overview <#toc3>`_
-----------------------

The following is a summary of all the programs supplied with **GMT** and
a very short description of their purpose. Detailed information about
each program can be found in the separate manual pages.

.. include:: explain_gmt_modules.rst_

`The Common Gmt Options <#toc4>`_
---------------------------------

**-B**\ [**p**\ \|\ **s**]\ *parameters* **-J**\ *parameters*
**-Jz**\ \|\ **Z**\ *parameters* **-K** **-O** **-P**
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] **-V**\ [*level*\ ]
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
**-a**\ *col*\ =\ *name*\ [*...*\ ]
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] **-c**\ *copies*
**-f**\ [**i**\ \|\ **o**]\ *colinfo*
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
**-h**\ [**i**\ \|\ **o**][*n*\ ]
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
**-o**\ *cols*\ [,*...*]
**-p**\ [**x**\ \|\ **y**\ \|\ **z**]\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
**-r** **-s**\ [*cols*\ ][\ **a**\ \|\ **r**] **-t**\ [*transp*\ ]
**-:**\ [**i**\ \|\ **o**]

`Description <#toc5>`_
----------------------

These are all the common GMT options that remain the same for all GMT
programs. No space between the option flag and the associated arguments.

.. include:: explain_-B_full.rst_

.. include:: explain_-J_full.rst_

.. include:: explain_-Jz_full.rst_

.. include:: explain_-K_full.rst_

.. include:: explain_-O_full.rst_

.. include:: explain_-P_full.rst_

.. include:: explain_-R_full.rst_

.. include:: explain_-Rz_full.rst_

.. include:: explain_-U_full.rst_

.. include:: explain_-V_full.rst_

.. include:: explain_-XY_full.rst_

.. include:: explain_-aspatial_full.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi_full.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bo_full.rst_

.. include:: explain_-c_full.rst_

.. include:: explain_colon_full.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f_full.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g_full.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h_full.rst_

.. include:: explain_-icols_full.rst_

.. include:: explain_-n_full.rst_

.. include:: explain_-ocols_full.rst_

.. include:: explain_perspective_full.rst_

.. include:: explain_nodereg_full.rst_

.. include:: explain_-s_full.rst_

.. include:: explain_-t_full.rst_

.. include:: explain_help.rst_

.. include:: explain_color.rst_

.. include:: explain_fill.rst_

.. include:: explain_font.rst_

.. include:: explain_pen.rst_

.. include:: explain_precision.rst_

.. include:: explain_grd_input.rst_


`See Also <#toc12>`_
--------------------

Look up the individual man pages for more details and full syntax. Run
**gmt --help** to list all GMT programs and to show all installation
directories. For an explanation of the various **GMT** settings in this
man page (like **FORMAT\_FLOAT\_OUT**), see the man page of the GMT
configuration file **gmt.conf**. Information is also available on the
**GMT** home page gmt.soest.hawaii.edu
