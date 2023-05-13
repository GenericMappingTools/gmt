GMT Coordinate Transformations
==============================

GMT programs read real-world coordinates and convert them to positions on a plot. This is achieved by selecting one of
several coordinate transformations or projections. We distinguish between three sets of such conversions:

-  :ref:`Cartesian coordinate transformations <cookbook/coordinate-transformations:Cartesian coordinate transformations>`

-  :ref:`Polar coordinate transformations <cookbook/coordinate-transformations:Polar coordinate transformations>`

-  :doc:`Map coordinate transformations <map-projections>`

The next Chapter will be dedicated to GMT map projections in its entirety. Meanwhile, the present Chapter will summarize
the properties of the :ref:`Cartesian <cookbook/coordinate-transformations:Cartesian coordinate transformations>` and
:ref:`Polar <cookbook/coordinate-transformations:Polar coordinate transformations>` coordinate transformations available
in GMT, list which parameters define them, and demonstrate how they are used to create simple plot axes. We will mostly
be using :doc:`/basemap` (and occasionally :doc:`/plot`) to demonstrate the various transformations. Our illustrations
may differ from those you reproduce with the same commands because of different settings in our ``gmt.conf`` file.
Finally, note that while we will specify dimensions in inches (by appending **i**), you may want to use cm (**c**), or
points (**p**) as :ref:`unit <cookbook/features:Dimension units>` instead.

Cartesian coordinate transformations
--------------------------------------------------------------------------------

GMT Cartesian coordinate transformations come in three flavors:

-  :ref:`cookbook/coordinate-transformations:Linear coordinate transformation`

-  :ref:`cookbook/coordinate-transformations:Logarithmic coordinate transformation`

-  :ref:`cookbook/coordinate-transformations:Power (exponential) coordinate transformation`

These transformations convert input coordinates :math:`(x,y)` to locations :math:`(x', y')` on a plot. There is no
coupling between :math:`x` and :math:`y` (i.e., :math:`x' = f(x)` and :math:`y' = f(y)`); it is a **one-dimensional**
projection. Hence, we may use separate transformations for the :math:`x`- and :math:`y`-axes (and :math:`z`-axes for 3-D
plots). Below, we will use the expression :math:`u' = f(u)`, where :math:`u` is either :math:`x` or :math:`y` (or
:math:`z` for 3-D plots). The coefficients in :math:`f(u)` depend on the desired plot size (or scale), the chosen
:math:`(x,y)` domain, and the nature of :math:`f` itself.

Two subsets of linear will be discussed separately; these are a polar (cylindrical) projection and a linear projection
applied to geographic coordinates (with a 360° periodicity in the *x*-coordinate). We will show examples of all of these
projections using dummy data sets created with :doc:`/gmtmath`, a "Reverse Polish Notation" (RPN) calculator that
operates on or creates table data:

   ::

      gmt math -T0/100/1  T SQRT = sqrt.txt
      gmt math -T0/100/10 T SQRT = sqrt10.txt

.. _-Jx_linear:

Linear coordinate transformation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are in fact three different uses of the Cartesian linear transformation, each associated with specific command
line options. The different manifestations result from specific properties of three kinds of data:

#. :ref:`cookbook/coordinate-transformations:Regular floating point coordinates`

#. :ref:`cookbook/coordinate-transformations:Geographic coordinates`

#. :ref:`cookbook/coordinate-transformations:Calendar time coordinates`

Regular floating point coordinates
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Syntax**

   **-Jx**\|\ **X**\ *x-scale*\|\ *width*\ [*/y-scale*\|\ *height*]

**Parameters**

- The *x-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *width* (*x*-axis length) in
  :ref:`plot-units <plt-units>` (with **-JX**).
- Optionally, the *y-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *height* (*y*-axis length)
  in :ref:`plot-units <plt-units>` (with **-JX**). If the *y*-scale or *y*-axis length is different from that of the
  *x*-axis (which is most often the case), separate the two scales (or lengths) by a slash, e.g., **-Jx**\ 0.1i/0.5i
  or **-JX**\ 8i/5i.

**Description**

Selection of the Cartesian linear transformation with regular floating point coordinates will result in a simple linear
scaling :math:`u' = au + b` of the input coordinates.

Normally, the user's :math:`x`-values will increase to the right and the :math:`y`-values will increase upwards. It
should be noted that in many situations it is desirable to have the direction of positive coordinates be reversed. For
example, when plotting depth on the *y*-axis it makes more sense to have the positive direction downwards. All that is
required to reverse the sense of positive direction is to supply a negative *scale* (or *width*). Finally, sometimes it
is convenient to specify the *width* (or *height*) of a map and let the other dimension be computed based on the implied
scale and the range of the other axis. To do this, simply specify the length to be recomputed as **0**.

**Example**

Our :math:`y = \sqrt{x}` data set can be plotted as follows:

.. literalinclude:: /_verbatim/GMT_linear.txt

.. figure:: /_images/GMT_linear.*
   :width: 400 px
   :align: center

   Linear transformation of Cartesian coordinates.

Geographic coordinates
^^^^^^^^^^^^^^^^^^^^^^

**Syntax**

   **-Jx**\|\ **X**\ *x-scale*\|\ *width*\ [*/y-scale*\|\ *height*][**d**\|\ **g**] or **-Rg**\|\ **d**

**Parameters**

- The *x-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *width* (*x*-axis length) in
  :ref:`plot-units <plt-units>` (with **-JX**).
- Optionally, the *y-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *height* (*y*-axis length)
  in :ref:`plot-units <plt-units>` (with **-JX**). If the *y*-scale or *y*-axis length is different from that of the
  *x*-axis (which is most often the case), separate the two scales (or lengths) by a slash, e.g., **-Jx**\ 0.1i/0.5i
  or **-JX**\ 8i/5i.
- **d** or **g** to indicate that data are geographical coordinates

**Description**

While the Cartesian linear projection is primarily designed for regular floating point :math:`x`,\ :math:`y` data, it is
sometimes necessary to plot geographical data in a linear projection. This poses a problem since longitudes have a 360°
periodicity. GMT therefore needs to be informed that it has been given geographical coordinates even though a linear
transformation has been chosen. We do so by adding a **g** (for geographical) or **d** (for degrees) directly after
**-R** or by appending a **g** or **d** to the end of the **-Jx** (or **-JX**) option.

**Example**

A crude world map centered on 125°E can be plotted as follows:

.. literalinclude:: /_verbatim/GMT_linear_d.txt

.. figure:: /_images/GMT_linear_d.*
   :width: 500 px
   :align: center

   Linear transformation of map coordinates.

.. _-Jx_time:

Calendar time coordinates
^^^^^^^^^^^^^^^^^^^^^^^^^

**Syntax**

    **-Jx**\|\ **X**\ *x-scale*\|\ *width*\ [*/y-scale*\|\ *height*]\ **T**\|\ **t** or
    **-R** with [*date*]\ **T**\ [*clock*] time entry or relative time followed by **t**

**Parameters**

- The *x-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *width* (*x*-axis length) in
  :ref:`plot-units <plt-units>` (with **-JX**).
- Optionally, the *y-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *height* (*y*-axis length)
  in :ref:`plot-units <plt-units>` (with **-JX**). If the *y*-scale or *y*-axis length is different from that of the
  *x*-axis (which is most often the case), separate the two scales (or lengths) by a slash, e.g., **-Jx**\ 0.1i/0.5i
  or **-JX**\ 8i/5i.
- **t** to indicate that input coordinates are time relative to :term:`TIME_EPOCH` or **T** to indicate that input
  coordinates are absolute time.

**Description**

Several particular issues arise when we seek to make linear plots using calendar date/time as the input coordinates. As
far as setting up the coordinate transformation we must indicate whether our input data have absolute time coordinates
or relative time coordinates. For the former we append **T** after the axis scale (or width), while for the latter we
append **t** at the end of the **-Jx** (or **-JX**) option. However, other command line arguments (like the **-R**
option) may already specify whether the time coordinate is absolute or relative. An absolute time entry must be given
as [*date*]\ **T**\ [*clock*] (with *date* given as *yyyy*\ [-*mm*\ [-*dd*]], *yyyy*\ [-*jjj*], or
*yyyy*\ [-**W**\ *ww*\ [-*d*]], and *clock* using the *hh*\ [:*mm*\ [:*ss*\ [*.xxx*]]] 24-hour clock format) whereas the
relative time is simply given as the units of time since the epoch followed by **t** (see :term:`TIME_UNIT` and
:term:`TIME_EPOCH` for information on specifying the time unit and the epoch).

When the coordinate ranges provided by the **-R** option and the projection type given by **-JX** (including the
optional **d**, **g**, **t** or **T**) conflict, GMT will warn the users about it. In general, the options provided
with **-JX** will prevail.

**Example**

A simple plot of a school week calendar can be made as follows:

.. literalinclude:: /_verbatim/GMT_linear_cal.txt

.. figure:: /_images/GMT_linear_cal.*
   :width: 400 px
   :align: center

   Linear transformation of calendar coordinates.

.. _-Jx_log:

Logarithmic coordinate transformation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Syntax**

    **-Jx**\|\ **X**\ *x-scale*\|\ *width*\ [**l**]\ [*/y-scale*\|\ *height*\ [**l**]]

**Parameters**

- The *x-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *width* (*x*-axis length) in
  :ref:`plot-units <plt-units>` (with **-JX**).
- Optionally, the *y-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *height* (*y*-axis length)
  in :ref:`plot-units <plt-units>` (with **-JX**). If the *y*-scale or *y*-axis length is different from that of the
  *x*-axis (which is most often the case), separate the two scales (or lengths) by a slash, e.g., **-Jx**\ 0.1i/0.5i
  or **-JX**\ 8i/5i.
- **l** to take log10 of values before scaling.

**Description**

The :math:`\log_{10}` transformation is simply :math:`u' = a \log_{10}(u) + b` and is selected by appending an **l**
immediately following the *scale* (or axis length) value.

Note that if :math:`x`- and :math:`y`-scaling are different and a :math:`\log_{10}-\log_{10}` plot is desired, the
**l** must be appended twice: Once after the *x*-scale (before the /) and once after the *y*-scale.

**Example**

A plot in which the *x*-axis is logarithmic (the *y*-axis remains linear, i.e., a semi-log plot) can be made as follows:

.. literalinclude:: /_verbatim/GMT_log.txt

.. figure:: /_images/GMT_log.*
   :width: 400 px
   :align: center

   Logarithmic transformation of x–coordinates.

.. _-Jx_power:

Power (exponential) coordinate transformation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Syntax**

    **-Jx**\|\ **X**\ *x-scale*\|\ *width*\ [**p**\ *power*]\ [*/y-scale*\|\ *height*\ [**p**\ *power*]]

**Parameters**

- The *x-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *width* (*x*-axis length) in
  :ref:`plot-units <plt-units>` (with **-JX**).
- Optionally, the *y-scale* in :ref:`plot-units <plt-units>`/data-unit (with **-Jx**) or the *height* (*y*-axis length)
  in :ref:`plot-units <plt-units>` (with **-JX**). If the *y*-scale or *y*-axis length is different from that of the
  *x*-axis (which is most often the case), separate the two scales (or lengths) by a slash, e.g., **-Jx**\ 0.1i/0.5i
  or **-JX**\ 8i/5i.
- **p**\ *power* to raise values to *power* before scaling.

**Description**

This projection uses :math:`u' = a u^b + c` and allows us to explore exponential relationships like :math:`x^p` versus
:math:`y^q`.

**Example**

This example uses the exponents :math:`p = 0.5` and :math:`q = 1` which means we will plot :math:`x` versus
:math:`\sqrt{x}`. We indicate this scaling by appending a **p** followed by the desired exponent, in our case 0.5.
Since :math:`q = 1` we do not need to specify **p**\ 1 since it is identical to the linear transformation.

.. literalinclude:: /_verbatim/GMT_pow.txt

.. figure:: /_images/GMT_pow.*
   :width: 400 px
   :align: center

   Exponential or power transformation of x–coordinates.

.. _-Jp:

Polar coordinate transformations
--------------------------------------------------------------------------------

**Syntax**

    **-Jp**\|\ **P**\ *scale*\|\ *width*\ [**+a**]\ [**+f**\ [**e**\|\ **p**\|\ *radius*]]\
    [**+r**\ *offset*][**+t**\ *origin*][**+z**\ [**p**\|\ *radius*]]

**Parameters**

- The *scale* (with **-Jp**; in :ref:`plot-units <plt-units>`/r-unit) or *width*
  (with **-JP**; in :ref:`plot-units <plt-units>`).
- Optionally, **+a** to indicate that *theta* is azimuth CW from North instead of direction CCW from East [Default is
  CCW from East].
- Optionally, **+f** to flip the radial direction to point inwards, and append **e** to indicate that *r* represents
  *elevations* in degrees (requires *south* >= 0 and *north* <= 90), **p** to select current planetary radius
  (determined by :term:`PROJ_ELLIPSOID`) as maximum radius [*north*], or *radius* to specify a custom radius.
- Optionally, **+r**\ *offset* to include a radial offset in measurement units [default is **0**].
- Optionally, **+t**\ *origin* in degrees so that this angular value is aligned with the positive *x*-axis (or the
  azimuth to be aligned with the positive *y*-axis if **+a**) [default is **0**].
- Optionally, **+z** to annotate depth rather than radius [default is radius]. Alternatively, if your *r* data are
  actually depths then you can append **p** or *radius* to get radial annotations (*r = radius - z*) instead.

**Description**

This transformation converts polar coordinates (angle :math:`\theta` and radius *r*) to positions on a plot. Now
:math:`x' = f(\theta,r)` and :math:`y' = g(\theta,r)`, hence it is similar to a regular map projection because :math:`x`
and :math:`y` are coupled and :math:`x` (i.e., :math:`\theta`) has a 360° periodicity. With input and output points both
in the plane it is a **two-dimensional** projection. The transformation comes in several flavors:

#. Normally, :math:`\theta` is understood to be directions
   counter-clockwise from the horizontal axis, but we may choose to
   specify an angular offset [default is zero]. We will call
   this offset :math:`\theta_0`. Then,
   :math:`x' = f(\theta, r) = ar \cos (\theta-\theta_0) + b` and
   :math:`y' = g(\theta, r) = ar \sin (\theta-\theta_0) + c`.

#. Alternatively, :math:`\theta` can be interpreted to be azimuths
   clockwise from the vertical axis, yet we may again choose to specify
   the angular offset [default is zero]. Then,
   :math:`x' = f(\theta, r) = ar \cos (90 - (\theta-\theta_0)) + b` and
   :math:`y' = g(\theta, r) = ar \sin (90 - (\theta-\theta_0)) + c`.

#. The radius *r* can either be radius or inverted to mean depth from the surface,
   planetary radii, or even elevations in degrees.

**Example**

As an example of this projection we will create a gridded data set in polar coordinates
:math:`z(\theta, r) = r^2 \cdot \cos{4\theta}` using :doc:`/grdmath`, a RPN calculator that operates on or creates
grid files.

We will use :doc:`/grdcontour` to make a contour map of this data. Because the data file only contains values
with :math:`2 \leq r \leq 4`, a donut shaped plot appears in the example figure shown below.

.. literalinclude:: /_verbatim/GMT_polar.txt

.. figure:: /_images/GMT_polar.*
   :width: 400 px
   :align: center

   Polar (Cylindrical) transformation of (:math:`\theta, r`) coordinates.