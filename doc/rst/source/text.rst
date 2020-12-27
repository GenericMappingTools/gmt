.. index:: ! text
.. include:: module_core_purpose.rst_

******
text
******

|text_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt text** [ *textfiles* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A| ]
|SYN_OPT-B|
[ |-C|\ [*dx/dy*][**+to**\|\ **O**\|\ **c**\|\ **C**] ]
[ |-D|\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]] ]
[ |-F|\ [**+a**\ [*angle*]][**+c**\ [*justify*]][**+f**\ [*font*]][**+j**\ [*justify*]][**+h**\|\ **l**\|\ **r**\ [*first*] \|\ **t**\ *text*\|\ **z**\ [*format*]] ]
[ |-G|\ [*fill*][**+n**] ]
[ |-L| ] [ |-M| ] [ |-N| ]
[ |-Q|\ **l**\|\ **u** ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z| ]
[ **-a**\ *col*\ =\ *name*\ [...] ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ **-it**\ *word* ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: text_common.rst_

Examples
--------

.. include:: explain_example.rst_

.. include:: oneliner_info.rst_

To plot just the red outlines of the (lon lat text strings) stored in the
file text.txt on a Mercator plot with the given specifications, use::

    gmt text text.txt -R-30/30/-10/20 -Jm0.1i -F+f18p,Helvetica,-=0.5p,red -B5 -pdf plot

To plot a text at the upper left corner of a 10 cm map::

    echo TopLeft | gmt text -R1/10/1/10 -JX10 -B -F+cTL -pdf plot

To add a typeset figure caption for a 3-inch wide illustration, use

   ::

    gmt text -R0/3/0/5 -JX3i -O -h1 -M -N -F+f12,Times-Roman+jLT -pdf figure << EOF
    This is an unmarked header record not starting with #
    > 0 -0.5 13p 3i j
    @%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
    a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
    of cities where it is @\_impossible@\_ to get any good Thai food; these are to be avoided.
    EOF

To place a line containing a Latex equation, try::

    echo 3 3 'Use @[\Delta g = 2\pi\rho Gh@[' | gmt text -R0/6/0/6 -JX15c -B -F+f32p+a30 -pdf map

.. include:: text_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`clip`,
:doc:`gmtcolors`,
:doc:`psconvert`,
:doc:`basemap`,
:doc:`legend`, :doc:`plot`
