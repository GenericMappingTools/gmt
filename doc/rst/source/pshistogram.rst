.. index:: ! pshistogram

***********
pshistogram
***********

.. only:: not man

    pshistogram - Calculate and plot histograms

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**pshistogram** [ *table* ] |-J|\ **x**\ \|\ **X**\ *parameters*
|-T|\ [\ *min/max*\ /]\ *inc*\ [**n**] \|\ |-T|\ *file*\ \|\ *list*
[ |-A| ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**+b**][**+f**\ *font*][**+o**\ *off*][**+r**] ]
[ |-F| ]
[ |-G|\ *fill* ] [ |-J|\ **z**\ \|\ **Z**\ *parameters* ]
[ |-I|\ [**o**\ \|\ **O**] ]
[ |-K| ]
[ |-L|\ **l**\ \|\ **h**\ \|\ **b**] ]
[ |-N|\ [*mode*][**+p**\ *pen*] ]
[ |-O| ] [|-P| ] [ |-Q|\ **r** ]
[ |SYN_OPT-R| ]
[ |-S| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ] 
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ [*type*][**+w**] ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT--| ]

.. include:: histogram_common.rst_

.. include:: common_classic.rst_

Examples
--------

To draw a histogram of the data v3206.t containing seafloor depths,
using a 250 meter bin width, center bars, and draw bar outline, use:

   ::

    gmt pshistogram v3206.t -JXh -T250 -F -W0.5p -V > plot.ps

If you know the distribution of your data, you may explicitly specify
range and scales. E.g., to plot a histogram of the y-values (2nd column)
in the file errors.xy using a 1 meter bin width, plot from -10 to +10
meters @ 0.75 cm/m, annotate every 2 m and 100 counts, and use black
bars, run:

   ::

    gmt pshistogram errors.xy -T1 -R-10/10/0/0 -Jxc/0.01c \
                    -Bx2+lError -By100+lCounts -Gblack -i1 -V > plot.ps

Since no y-range was specified, **pshistogram** will calculate *ymax* in even
increments of 100.

See Also
--------

:doc:`gmt`, :doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psrose`,
:doc:`psxy`
