.. index:: ! psxy
.. include:: module_core_purpose.rst_

****
psxy
****

|psxy_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt psxy** [ *table* ] |-J|\ *parameters*
|SYN_OPT-Rz|
[ |-A|\ [**m**\|\ **p**\|\ **x**\|\ **y**] ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ *dx*/*dy* ]
[ |-E|\ [**x**\|\ **y**\|\ **X**\|\ **Y**][**+a**][**+cl**\|\ **f**][**+n**][**+w**\ *cap*][**+p**\ *pen*] ]
[ |-F|\ [**c**\|\ **n**\|\ **r**][**a**\|\ **f**\|\ **s**\|\ **r**\|\ *refpoint*] ]
[ |-G|\ *fill* ]
[ |-I|\ [*intens*] ]
[ |-K| ]
[ |-L|\ [**+b**\|\ **d**\|\ **D**][**+xl**\|\ **r**\|\ *x0*][**+yl**\|\ **r**\|\ *y0*][**+p**\ *pen*] ]
[ |-N|\ [**c**\|\ **r**] ]
[ |-O| ] [ |-P| ]
[ |-S|\ [*symbol*][*size*] ]
[ |-T| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][*attr*] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *value* [**+f**\|\ **l**] ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. include:: plot_common.rst_

.. include:: common_classic.rst_

Examples
--------

.. include:: explain_example.rst_

To plot solid red circles (diameter = 0.2 cm) at the positions listed
in the file DSDP.txt on a Mercator map at 0.3 cm/degree of the area 100E to
160E, 20S to 30N, with automatic tick-marks and gridlines, use

   ::

    gmt psxy DSDP.txt -R100/160/-20/30 -Jm0.3c -Sc0.2c -Gred -Bafg > map.ps

To plot the xyz values in the file quakes.xyzm as circles with size
given by the magnitude in the 4th column and color based on the depth in
the third using the CPT rgb.cpt on a linear map, use

   ::

    gmt psxy quakes.xyzm -R0/1000/0/1000 -JX6i -Sc -Crgb -B200 > map.ps

To plot the file trench.txt on a Mercator map, with white triangles with
sides 0.25 inch on the left side of the line, spaced every 0.8 inch, use

   ::

    gmt psxy trench.txt -R150/200/20/50 -Jm0.15i -Sf0.8i/0.1i+l+t -Gwhite -W -Baf > map.ps

To plot a point with color dictated by the *t.cpt* file for the *z*-value 65, try

   ::

    echo 175 30 | gmt psxy -R150/200/20/50 -JX15c -Sc0.5c -Zf65 -Ct.cpt > map.ps

To plot the data in the file misc.txt as symbols determined by the code in
the last column, and with size given by the magnitude in the 4th column,
and color based on the third column via the CPT chrome on a
linear map, use

   ::

    gmt psxy misc.txt -R0/100/-50/100 -JX6i -S -Cchrome -B20 > map.ps

If you need to place vectors on a plot you can choose among
straight Cartesian vectors, math circular vectors, or geo-vectors (these
form small or great circles on the Earth).  These can have optional heads at either
end, and heads may be the traditional arrow, a circle, or a terminal cross-line.
To place a few vectors with
a circle at the start location and an arrow head at the end, try

   ::

    gmt psxy -R0/50/-50/50 -JX6i -Sv0.15i+bc+ea -Gyellow -W0.5p -Baf << EOF > map.ps
    10 10 45 2i
    30 -20 0 1.5i
    EOF

To plot vectors (red vector heads, solid stem) from the file data.txt that contains
record of the form lon, lat, dx, dy, where dx, dy are the Cartesian
vector components given in user units, and these user units should be converted
to cm given the scale 3.60, try

   ::

    gmt psxy -R20/40/-20/0 -JM6i -Sv0.15i+e+z3.6c -Gred -W0.25p -Baf data.txt > map.ps

.. include:: plot_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`gmtcolors`,
:doc:`psbasemap`, :doc:`psxyz`

.. ------------------------------------- Examples per option -------------------

.. |ex_OPT-L| raw:: html

   <a href="#openModal">Example</a>
   <div id="openModal" class="modalDialog">
    <div>
        <a href="#close" title="Close" class="close">X</a>
        <h2>-L example</h2>
        <p>
        ps=filler.ps</br>
        cat << EOF > t.txt</br>
        1 1</br>
        2 3</br>
        3 2</br>
        4 4</br>
        EOF</br>
        gmt psxy -R0/5/0/5 -JX3i -P -K -B0 t.txt -Gred -W2p -L+yb > $ps</br>
        gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+yt -X3.25i >> $ps</br>
        gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+xl -X-3.25i -Y3.25i >> $ps</br>
        gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+xr -X3.25i >> $ps</br>
        gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+y4 -X-3.25i -Y3.25i >> $ps</br>
        gmt psxy -R -J -O -K -B0 t.txt -Gred -W2p -L+x4.5 -X3.25i >> $ps</br>
        gmt psxy -R -J -O -T >> $ps</br>
        </p>
    </div>
   </div>
