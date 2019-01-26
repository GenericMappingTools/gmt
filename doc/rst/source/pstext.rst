.. index:: ! pstext

******
pstext
******

.. only:: not man

    pstext - Plot or typeset text on maps

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pstext** [ *textfiles* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A| ]
|SYN_OPT-B|
[ |-C|\ [*dx/dy*\ ][\ **+t**\ o\|\O\|\c\|C\ ] ]
[ |-D|\ [**j**\ \|\ **J**]\ *dx*\ [/*dy*][\ **+v**\ [*pen*]] ]
[ |-F|\ [**+a**\ [*angle*]][\ **+c**\ [*justify*]][\ **+f**\ [*font*]][\ **+j**\ [*justify*]][\ **+h**\ \|\ **+l**\|\ **+r**\ [*first*] \|\ **+t**\ *text*\ \|\ **+z**\ [*format*]] ] 
[ |-G|\ *color* ]
[ |-K| ]
[ |-L| ] [ |-M| ] [ |-N| ] [ |-O| ] [ |-P| ]
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

.. include:: common_classic.rst_

Examples
--------

To plot just the red outlines of the (lon lat text strings) stored in the
file text.txt on a Mercator plot with the given specifications, use

   ::

    gmt pstext text.txt -R-30/30/-10/20 -Jm0.1i -P -F+f18p,Helvetica,-=0.5p,red -B5 > plot.ps

To plot a text at the upper left corner of a 10 cm map

   ::

    echo TopLeft | gmt pstext -R1/10/1/10 -JX10 -F+cTL -P > plot.ps

To add a typeset figure caption for a 3-inch wide illustration, use

   ::

    gmt pstext -R0/3/0/5 -JX3i -O -h1 -M -N -F+f12,Times-Roman+jLT << EOF >> figure.ps


   ::

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
:doc:`psclip`,
:doc:`gmtcolors`,
:doc:`psconvert`,
:doc:`psbasemap`,
:doc:`pslegend`, :doc:`psxy`
