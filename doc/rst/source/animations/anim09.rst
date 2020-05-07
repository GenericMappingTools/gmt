.. _anim09:

(9) Flying over the Antarctic Ridge and the East Pacific Rise
=============================================================

Animation of a simulated fly-over of the Pacific basin mid-ocean ridge system.
It uses a premade flight path that originally was derived from a data file
of the world's ridges, then filtered and manipulated to give the equidistant
path to simulate a constant velocity at the given altitude, with synthetic
banking as we turn to follow the path.  We use the 30 arc second global relief
grid and overlay a few labels for named features.

- Grid:   @earth_relief_30s.grd
- Path:   MOR_PAC_twist_path.txt
- Labels: MOR_names.txt

We create a global intensity grid using shading from East and a CPT file; these are
created by the preflight script.
We add a 1 second fade in and a 1 second fade out for the animation
The resulting movie was presented at the Fall 2019 AGU meeting in an eLighting talk:
P. Wessel, 2019, GMT science animations for the masses, Abstract IN21B-11.

The finished movie is available in our YouTube channel as well (without fading):
https://youtu.be/LTxlR5LuJ8g

The movie took ~6 hours to render on a 24-core MacPro 2013.

.. literalinclude:: /_verbatim/anim09.txt
   :language: bash

..  youtube:: LTxlR5LuJ8g
    :width: 100%
