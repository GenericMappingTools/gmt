.. index:: ! text

******
text
******

.. only:: not man

    Plot or typeset text on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt text** [ *textfiles* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A| ]
|SYN_OPT-B|
[ |-C|\ [*dx/dy*\ ][\ **+t**\ o\|\O\|\c\|C\ ] ]
[ |-D|\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **+v**\ [*pen*]] ]
[ |-F|\ [**+a**\ [*angle*]][\ **+c**\ [*justify*]][\ **+f**\ [*font*]][\ **+j**\ [*justify*]][\ **+h**\ \|\ **+l**\|\ **+r**\ [*first*] \|\ **+t**\ *text*\ \|\ **+z**\ [*format*]] ] 
[ |-G|\ *color* ]
[ |-L| ] [ |-M| ] [ |-N| ]
[ |-Q|\ **l**\ \|\ **u** ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-U| ]
[ |-Z| ] [ **-a**\ *col*\ =\ *name*\ [...] ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: text_common.rst_

Examples
--------

To plot just the red outlines of the (lon lat text strings) stored in the
file text.txt on a Mercator plot with the given specifications, use

   ::

    gmt text text.txt -R-30/30/-10/20 -Jm0.1i -F+f18p,Helvetica,-=0.5p,red -B5 -pdf plot

To plot a text at the upper left corner of a 10 cm map

   ::

    echo TopLeft | gmt text -R1/10/1/10 -JX10 -F+cTL -pdf plot

To add a typeset figure caption for a 3-inch wide illustration, use

   ::

    gmt text -R0/3/0/5 -JX3i -O -h1 -M -N -F+f12,Times-Roman+jLT -pdf figure << EOF
    This is an unmarked header record not starting with #
    > 0 -0.5 13p 3i j
    @%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
    a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
    of cities where it is @\_impossible@\_ to get any good Thai food; these are to be avoided.
    EOF

.. include:: text_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`clip`,
:doc:`gmtcolors`,
:doc:`psconvert`,
:doc:`basemap`,
:doc:`legend`, :doc:`plot`
