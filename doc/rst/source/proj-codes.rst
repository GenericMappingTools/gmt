.. Substitution definitions:
.. |lon0| replace:: lon\ :sub:`0`
.. |lat0| replace:: lat\ :sub:`0`
.. |lon1| replace:: lon\ :sub:`1`
.. |lat1| replace:: lat\ :sub:`1`
.. |lat2| replace:: lat\ :sub:`2`
.. |lonp| replace:: lon\ :sub:`p`
.. |latp| replace:: lat\ :sub:`p`

.. list-table::
   :widths: 40 10 10 40
   :header-rows: 2

   * - Projection
     - GMT CODES
     - PROJ CODES
     - Parameters
   * -
     - **-J** (*scale*\|\ *WIDTH*);
     - **-J** (*scale*)
     -
   * - :ref:`Lambert azimuthal equal area <-Ja>`
     - **-Ja**\|\ **A**
     - **-Jlaea/**
     - |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*\|\ *width*
   * - :ref:`Albers conic equal area <-Jb>`
     - **-Jb**\|\ **B**
     - **-Jaea/**
     - |lon0|/|lat0|/|lat1|/|lat2|/\ *scale*\|\ *width*
   * - :ref:`Cassini cylindrical <-Jc>`
     - **-Jc**\|\ **C**
     - **-Jcass/**
     - |lon0|/|lat0|/\ *scale*\|\ *width*
   * - :ref:`Cylindrical stereographic <-Jcyl_stere>`
     - **-Jcyl_stere**\|\ **Cyc_stere**
     - **-Jcyl_stere/**
     - [|lon0|\ [/|lat0|]/]\ *scale*\|\ *width*
   * - :ref:`Equidistant conic <-Jd>`
     - **-Jd**\|\ **D**
     - **-Jeqdc/**
     - |lon0|/|lat0|/|lat1|/|lat2|/\ *scale*\|\ *width*
   * - :ref:`Azimuthal equidistant <-Je>`
     - **-Je**\|\ **E**
     - **-Jaeqd/**
     - |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*\|\ *width*
   * - :ref:`Azimuthal gnomonic <-Jf>`
     - **-Jf**\|\ **F**
     - **-Jgnom/**
     - |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*\|\ *width*
   * - :ref:`Azimuthal orthographic <-Jg>`
     - **-Jg**\|\ **G**
     - **-Jortho/**
     - |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*\|\ *width*
   * - :ref:`General perspective <-Jg_pers>`
     - **-Jg**\|\ **G**
     - **-Jnsper/**
     - |lon0|/|lat0|/\ *alt*/*azim*/*tilt*/*twist*/*W*/*H*/\ *scale*\|\ *width*
   * - :ref:`Hammer equal area <-Jh>`
     - **-Jh**\|\ **H**
     - **-Jhammer/**
     - |lon0|/\ *scale*\|\ *width*
   * - :ref:`Sinusoidal equal area <-Ji>`
     - **-Ji**\|\ **I**
     - **-Jsinu/**
     - |lon0|/\ *scale*\|\ *width*
   * - :ref:`Miller cylindrical <-Jj>`
     - **-Jj**\|\ **J**
     - **-Jmill/**
     - |lon0|/\ *scale*\|\ *width*
   * - :ref:`Eckert IV equal area <-Jk>`
     - **-Jkf**\|\ **Kf**
     - **-Jeck4/**
     - |lon0|/\ *scale*\|\ *width*
   * - :ref:`Eckert VI equal area <-Jk>`
     - **-Jks**\|\ **Ks**
     - **-Jeck6/**
     - |lon0|/\ *scale*\|\ *width*
   * - :ref:`Lambert conic conformal <-Jl>`
     - **-Jl**\|\ **L**
     - **-Jlcc/**
     - |lon0|/|lat0|/|lat1|/|lat2|/\ *scale*\|\ *width*
   * - :ref:`Mercator cylindrical <-Jm>`
     - **-Jm**\|\ **M**
     - **-Jmerc/**
     - [|lon0|\ [/|lat0|/]]\ *scale*\|\ *width*
   * - :ref:`Robinson <-Jn>`
     - **-Jn**\|\ **N**
     - **-Jrobin/**
     - [|lon0|/]\ *scale*\|\ *width*
   * - :ref:`Oblique Mercator, 1: origin and azim <-Jo>`
     - **-Jo**\|\ **O**\ [**a**\|\ **A**]
     - **-Jomerc/**
     - |lon0|/|lat0|/\ *azim*/*scale*\|\ *width* [**+v**]
   * - :ref:`Oblique Mercator, 2: two points <-Jo>`
     - **-Jo**\|\ **O**\ [**b**\|\ **B**]
     - **-Jomerc/**
     - |lon0|/|lat0|/|lon1|/|lat1|/\ *scale*\|\ *width*\ [**+v**]
   * - :ref:`Oblique Mercator, 3: origin and pole <-Jo>`
     - **-Jo**\|\ **O**\ [**c**\|\ **C**]
     - **-Jomercp/**
     - |lon0|/|lat0|/|lonp|/|latp|/\ *scale*\|\ *width*\ [**+v**]
   * - :ref:`Polar [azimuthal] <-Jp>`  (:math:`\theta, r`) (or cylindrical)
     - **-Jp**\|\ **P**
     - **-Jpolar/**
     - *scale*\|\ *width*\ [**+a**]\ [**+f**\ [**e**\|\ **p**\|\ *radius*]]\
       [**+r**\ *offset*][**+t**\ *origin*][**+z**\ [**p**\|\ *radius*]]
   * - :ref:`(American) polyconic <-Jpoly>` 
     - **-Jpoly**\|\ **Poly**
     - **-Jpoly/**
     - [|lon0|\ [/|lat0|/]]\ *scale*\|\ *width*
   * - :ref:`Equidistant cylindrical <-Jq>`
     - **-Jq**\|\ **Q**
     - **-Jeqc/**
     - [|lon0|\ [/|lat0|/]]\ *scale*\|\ *width*
   * - :ref:`Winkel Tripel <-Jr>`
     - **-Jr**\|\ **R**
     - **-Jwintri/**
     - [|lon0|/]\ *scale*\|\ *width*
   * - :ref:`General stereographic <-Js>`
     - **-Js**\|\ **S**
     - **-Jstere/**
     - |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*\|\ *width*
   * - :ref:`Transverse Mercator <-Jt>`
     - **-Jt**\|\ **T**
     - **-Jtmerc/**
     - [|lon0|\ [/|lat0|/]]\ *scale*\|\ *width*
   * - :ref:`Universal Transverse Mercator (UTM) <-Ju>`
     - **-Ju**\|\ **U**
     - **-Jutm/**
     - *zone*/*scale*\|\ *width*
   * - :ref:`Van der Grinten <-Jv>`
     - **-Jv**\|\ **V**
     - **-Jvandg/**
     - [|lon0|/]\ *scale*\|\ *width*
   * - :ref:`Mollweide <-Jw>`
     - **-Jw**\|\ **W**
     - **-Jmoll/**
     - [|lon0|/]\ *scale*\|\ *width*
   * - :ref:`Linear <-Jx_linear>`, :ref:`logarithmic <-Jx_log>`,
       :ref:`power <-Jx_power>`, and :ref:`time <-Jx_time>`
     - **-Jx**\|\ **X**
     - **-Jxy**
     - *xscale*\|\ *width*\ [**l**\|\ **p**\ *power*\|\ **T**\|\ **t**]\
       [/\ *yscale*\|\ *height*\ [**l**\|\ **p**\ *power*\|\ **T**\|\ **t**]][**d**]
   * - :ref:`Cylindrical equal area <-Jy>`
     - **-Jy**\|\ **Y**
     - **-Jcea/**
     - |lon0|/|lat0|/\ *scale*\|\ *width*
