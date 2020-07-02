#########################
Projection Specifications
#########################

GMT offers 31 map projections. These are specified using the **-J** common option.
There are two conventions you may use: (a) GMT-style syntax and (b) `PROJ <https://proj.org/>`_\ -style syntax.
The projection codes for the GMT-style and the PROJ-style are tabulated below.

.. Substitution definitions:
.. |lon0| replace:: lon\ :sub:`0`
.. |lat0| replace:: lat\ :sub:`0`
.. |lon1| replace:: lon\ :sub:`1`
.. |lat1| replace:: lat\ :sub:`1`
.. |lat2| replace:: lat\ :sub:`2`
.. |lonp| replace:: lon\ :sub:`p`
.. |latp| replace:: lat\ :sub:`p`

.. list-table::
   :widths: 33 33 33
   :header-rows: 2

   * - GMT PROJECTION CODES
     - PROJ PROJECTION CODES
     -
   * - **-J** (upper case for *width*, lower case for *scale*)
     - **-J** (lower case for *scale* only)
     -
   * - **-JA**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *width*
     - **-Jlaea/**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *scale* 
     - Lambert azimuthal equal area :ref:`... <-Ja_full>`
   * - **-JB**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *width*
     - **-Jaea/**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *scale*
     - Albers conic equal area :ref:`... <-Jb_full>`
   * - **-JC**\ |lon0|/|lat0|/\ *width* 
     - **-Jcass/**\ |lon0|/|lat0|/\ *scale*
     - Cassini cylindrical :ref:`... <-Jc_full>`
   * - **-JCyl_stere/**\ [|lon0|\ [/|lat0|/]]\ *width* 
     - **-Jcyl_stere/**\ [|lon0|\ [/|lat0|]/]\ *scale* 
     - Cylindrical stereographic :ref:`... <-Jcyl_stere_full>`
   * - **-JD**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *width*
     - **-Jeqdc/**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *scale* 
     - Equidistant conic :ref:`... <-Jd_full>`
   * - **-JE**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *width* 
     - **-Jaeqd/**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *scale* 
     - Azimuthal equidistant :ref:`... <-Je_full>`
   * - **-JF**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *width*
     - **-Jgnom/**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*
     - Azimuthal gnomonic :ref:`... <-Jf_full>`
   * - **-JG**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *width*
     - **-Jortho/**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*
     - Azimuthal orthographic :ref:`... <-Jg_full>`
   * - **-JG**\ |lon0|/|lat0|/\ *alt*/*azim*/*tilt*/*twist*/*W*/*H*/*width*
     - **-Jnsper/**\ |lon0|/|lat0|/\ *alt*/*azim*/*tilt*/*twist*/*W*/*H*/*scale*
     - General perspective :ref:`... <-Jg_full>`
   * - **-JH**\ [|lon0|/]\ *width*
     - **-Jhammer/**\ |lon0|/\ *scale*
     - Hammer equal area :ref:`... <-Jh_full>`
   * - **-JI**\ [|lon0|/]\ *width*
     - **-Jsinu/**\ |lon0|/\ *scale*
     - Sinusoidal equal area :ref:`... <-Ji_full>`
   * - **-JJ**\ [|lon0|/]\ *width*
     - **-Jmill/**\ |lon0|/\ *scale*  
     - Miller cylindrical :ref:`... <-Jj_full>`
   * - **-JKf**\ [|lon0|/]\ *width*
     - **-Jeck4/**\ |lon0|/\ *scale*
     - Eckert IV equal area :ref:`... <-Jk_full>`
   * - **-JKs**\ [|lon0|/]\ *width*
     - **-Jeck6/**\ |lon0|/\ *scale*
     - Eckert VI equal area :ref:`... <-Jk_full>`
   * - **-JL**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *width* 
     - **-Jlcc/**\ |lon0|/|lat0|/|lat1|/|lat2|/\ *scale*
     - Lambert conic conformal :ref:`... <-Jl_full>`
   * - **-JM**\ [|lon0|\ [/|lat0|]/]\ *width*
     - **-Jmerc/**\ [|lon0|\ [/|lat0|/]]\ *scale*
     - Mercator cylindrical :ref:`... <-Jm_full>`
   * - **-JN**\ [|lon0|/]\ *width*
     - **-Jrobin/**\ [|lon0|/]\ *scale*
     - Robinson :ref:`... <-Jn_full>`
   * - **-JOa**\ |lon0|/|lat0|/\ *azim*/*width*\ [**+v**]
     - **-Jomerc/**\ |lon0|/|lat0|/\ *azim*/*scale*\ [**+v**]
     - Oblique Mercator, 1: origin and azim   :ref:`... <-Jo_full>`
   * - **-JOb**\ |lon0|/|lat0|/|lon1|/|lat1|/\ *width*\ [**+v**]
     - **-Jomerc/**\ |lon0|/|lat0|/|lon1|/|lat1|/\ *scale*\ [**+v**]
     - Oblique Mercator, 2: two points :ref:`... <-Jo_full>`
   * - **-JOc**\ |lon0|/|lat0|/|lonp|/|latp|/\ *width*\ [**+v**]
     - **-Jomercp/**\ |lon0|/|lat0|/|lonp|/|latp|/\ *scale*\ [**+v**]
     - Oblique Mercator, 3: origin and pole :ref:`... <-Jo_full>`
   * - **-JP**\ *width*\ [**+a**]\ [**+f**\ [**e**\|\ **p**\|\ *radius*]][**+r**\ *offset*][**+t**\ *origin*][**+z**\ [**p**\|\ *radius*]]
     - **-Jpolar/**\ [**+a**]\ [**+f**\ [**e**\|\ **p**\|\ *radius*]][**+r**\ *offset*][**+t**\ *origin*][**+z**\ [**p**\|\ *radius*]]
     - Polar [azimuthal] (:math:`\theta, r`) (or cylindrical)
   * - **-JPoly**\ [|lon0|\ [/|lat0|]/]\ *width*
     - **-Jpoly/**\ [|lon0|\ [/|lat0|/]]\ *scale*
     - (American) polyconic :ref:`... <-Jpoly_full>`
   * - **-JQ**\ [|lon0|\ [/|lat0|/]]\ *width*
     - **-Jeqc/**\ [|lon0|\ [/|lat0|/]]\ *scale* 
     - Equidistant cylindrical :ref:`... <-Jq_full>`
   * - **-JR**\ [|lon0|/]\ *width*
     - **-Jwintri/**\ [|lon0|/]\ *scale*
     - Winkel Tripel :ref:`... <-Jr_full>`
   * - **-JS**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *width* 
     - **-Jstere/**\ |lon0|/|lat0|\ [/\ *horizon*]/\ *scale*
     - General stereographic :ref:`... <-Js_full>`
   * - **-JT**\ [|lon0|\ [/|lat0|]/]\ *width*
     - **-Jtmerc/**\ [|lon0|\ [/|lat0|/]]\ *scale*
     - Transverse Mercator :ref:`... <-Jt_full>`
   * - **-JU**\ *zone*/*width*
     - **-Jutm/**\ *zone*/*scale* 
     - Universal Transverse Mercator (UTM) :ref:`... <-Ju_full>`
   * - **-JV**\ [|lon0|/]\ *width*
     - **-Jvandg/**\ [|lon0|/]\ *scale*
     - Van der Grinten :ref:`... <-Jv_full>`
   * - **-JW**\ [|lon0|/]\ *width*
     - **-Jmoll/**\ [|lon0|/]\ *scale*
     - Mollweide :ref:`... <-Jw_full>`
   * - **-JX**\ *width*\ [**l**\|\ **p**\ *exp*\|\ **T**\|\ **t**][/\ *height*\ [**l**\|\ **p**\ *exp*\|\ **T**\|\ **t**]][**d**]
     - **-Jxy**\ *xscale*\ [**l**\|\ **p**\ *exp*\|\ **T**\|\ **t**][/\ *yscale*\ [**l**\|\ **p**\ *exp*\|\ **T**\|\ **t**]][**d**]
     - Linear, log\ :math:`_{10}`, :math:`x^a-y^b`, and time :ref:`... <-Jx_full>`
   * - **-JY**\ |lon0|/|lat0|/\ *width*
     - **-Jcea/**\ |lon0|/|lat0|/\ *scale* 
     - Cylindrical equal area :ref:`... <-Jy_full>`
