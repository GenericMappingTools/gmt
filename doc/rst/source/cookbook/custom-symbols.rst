.. _App-custom_symbols:

Custom Plot Symbols
===================

Background
----------

The GMT tools :doc:`/plot` and :doc:`/plot3d` are capable of using custom
symbols as alternatives to the built-in, standard geometrical shapes
such as circles, triangles, and many others. One the command line, custom
symbols are selected via the **-Sk**\ *symbolname*\ [*size*] symbol
selection, where *symbolname* refers to a special symbol definition file
called ``symbolname.def`` that must be available via the standard GMT user paths. Several
custom symbols comes pre-configured with GMT (see
Figure :ref:`Custom symbols <Custom_symbols>`)

.. _Custom_symbols:

.. figure:: /_images/GMT_App_N_1.*
   :width: 500 px
   :align: center

   Custom plot symbols supported by GMT. These are all single-parameter symbols.
   Be aware that some symbols may have a hardwired fill or no-fill component,
   while others duplicate what is already available as standard built-in symbols.


You may find it convenient to examine some of these and use them as a
starting point for your own design; they can be found in GMT's
share/custom directory.  In addition to the ones listed in Figure :ref:`Custom symbols <Custom_symbols>`
you can use the symbol **QR** to place the GMT QR Code that links to www.generic-mapping-tools.org;
alternatively use **QR_transparent** to *not* plot the background opaque white square.

The macro language
------------------

To make your own custom plot symbol, you will need to design your own
\*.def files. This section defines the language used to build custom
symbols. You can place these definition files in your current directory
or in your ``~/.gmt`` user directory. When designing the symbol you are working
in a relative coordinate system centered on (0,0). This point will be
mapped to the actual location specified by your data coordinates.
Furthermore, your symbol should be constructed within the domain
:math:`{-\frac{1}{2},+\frac{1}{2},-\frac{1}{2},+\frac{1}{2}}`, resulting
in a 1 by 1 relative canvas area. This 1 x 1 square will be scaled to your
actual symbol size when plotted.  However, there are no requirement that
all your design fit inside this domain.  This command will produce a nice
template for you to draw your design::

    gmt basemap -R-0.5/0.5/-0.5/0.5 -JX15c -Bpa0.1fg0.02 -Bsg0.1 --GRID_PEN_PRIMARY=faint -pdf template

Comment lines
~~~~~~~~~~~~~

Your definition file may have any number of comment lines, defined to
begin with the character #. These are skipped by GMT but provides a
mechanism for you to clarify what your symbol does.

Symbol variables
~~~~~~~~~~~~~~~~

Simple symbols, such as circles and triangles, only take a single
parameter: the symbol size, which is either given on the command line
(via **-Sk**) or as part of the input data. However, more complicated
symbols that involve angles, or conditional tests, may require more
parameters. If your custom symbol requires more than the implicit single size
parameter you must include the line

    **N**: *n_extra_parameters* [*types*]

before any other macro commands. It is an optional statement in that
*n_extra_parameters* will default to 0 unless explicitly set. By
default the extra parameters are considered to be quantities that should
be passed directly to the symbol machinery. However, you can use the
*types* argument to specify different types of parameters and thus single
out parameters for pre-processing. The available types are

  **a** Geographic azimuth (positive clockwise from north toward east). Parameters
  identified as azimuth will first be converted to map angle
  (positive counter-clockwise from horizontal) given the current
  map projection (or simply via 90-azimuth for Cartesian plots).
  We ensure the angles fall in the 0-360 range and any macro test can rely on this range.

  **l** Length, i.e., an additional length scale (in cm, inch, or point as
  per :term:`PROJ_LENGTH_UNIT`) in addition to the given symbol size.

  **o** Other, i.e., a numerical quantity to be passed to the custom symbol unchanged.

  **r** rotation angles (positive counter-clockwise from horizontal).
  We ensure the angles fall in the 0-360 range and any macro test can rely on this range.

  **s** String, i.e., a single column of text to be placed by the **l** command.
  Use octal \\040 to include spaces to ensure the text string remains a single word.

To use the extra parameters in your macro you address them as $1, $2, etc.  There
is no limit on how many parameters your symbol may use. To access the trailing text in
the input file you use $t and for a particular word (number k = 0, 1, ...) in the
trailing text you use $t\ *k*.

Macro commands
~~~~~~~~~~~~~~

The custom symbol language contains commands to rotate the relative
coordinate system, draw free-form polygons and lines, change the current
fill and/or pen, place text, and include basic geometric symbols as part of the
overall design (e.g., circles, triangles, etc.). The available commands
are listed in Table :ref:`custsymb <tbl-custsymb>`.  Note that all angles
in the arguments can be provided as variables while the remaining parameters
are constants.

.. _tbl-custsymb:

+---------------+------------+----------------------------------------+--------------------------------------------+
| **Name**      | **Code**   | **Purpose**                            | **Arguments**                              |
+===============+============+========================================+============================================+
| rotate        | **O**      | Rotate the coordinate system           | :math:`\alpha`\[**a**]                     |
+---------------+------------+----------------------------------------+--------------------------------------------+
| moveto        | **M**      | Set a new anchor point                 | :math:`x_0, y_0`                           |
+---------------+------------+----------------------------------------+--------------------------------------------+
| drawto        | **D**      | Draw line from previous point          | :math:`x, y`                               |
+---------------+------------+----------------------------------------+--------------------------------------------+
| arc           | **A**      | Append circular arc to existing path   | :math:`x_c, y_c, d, \alpha_1, \alpha_2`    |
+---------------+------------+----------------------------------------+--------------------------------------------+
| stroke        | **S**      | Stroke existing path only              |                                            |
+---------------+------------+----------------------------------------+--------------------------------------------+
| texture       | **T**      | Change current pen and fill            |                                            |
+---------------+------------+----------------------------------------+--------------------------------------------+
| star          | **a**      | Plot a star                            | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| circle        | **c**      | Plot a circle                          | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| diamond       | **d**      | Plot a diamond                         | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| ellipse       | **e**      | Plot an ellipse                        | :math:`x, y, \alpha`,\ *major*,\ *minor*   |
+---------------+------------+----------------------------------------+--------------------------------------------+
| octagon       | **g**      | Plot an octagon                        | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| hexagon       | **h**      | Plot a hexagon                         | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| invtriangle   | **i**      | Plot an inverted triangle              | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| rotrectangle  | **j**      | Plot an rotated rectangle              | :math:`x, y, \alpha, width, height`        |
+---------------+------------+----------------------------------------+--------------------------------------------+
| letter        | **l**      | Plot a letter                          | :math:`x, y, size, string`                 |
+---------------+------------+----------------------------------------+--------------------------------------------+
| marc          | **m**      | Plot a math arc (no heads)             | :math:`x, y, r, \alpha_1, \alpha_2`        |
+---------------+------------+----------------------------------------+--------------------------------------------+
| pentagon      | **n**      | Plot a pentagon                        | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| plus          | **+**      | Plot a plus sign                       | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| rect          | **r**      | Plot a rectangle                       | :math:`x, y, width, height`                |
+---------------+------------+----------------------------------------+--------------------------------------------+
| roundrect     | **R**      | Plot a rounded rectangle               | :math:`x, y, width, height, radius`        |
+---------------+------------+----------------------------------------+--------------------------------------------+
| square        | **s**      | Plot a square                          | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| triangle      | **t**      | Plot a triangle                        | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| wedge         | **w**      | Plot a wedge                           | :math:`x, y, d, \alpha_1, \alpha_2`        |
+---------------+------------+----------------------------------------+--------------------------------------------+
| cross         | **x**      | Plot a cross                           | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| x-dash        | **-**      | Plot a x-dash                          | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+
| y-dash        | **y**      | Plot a y-dash                          | :math:`x, y, size`                         |
+---------------+------------+----------------------------------------+--------------------------------------------+

Note for **O**\: if an **a** is appended to the angle then :math:`\alpha` is considered
to be a map azimuth; otherwise it is a Cartesian map angle.  The **a** modifier
does not apply if the angle is given via a variable, in which case the type of angle
has already been specified via **N:** above and already converged before seen by **O**.
Finally, the **O** command can also be given the negative of a variable, e.g., -$2 to
undo a rotation, if necessary.

Symbol fill and outline
~~~~~~~~~~~~~~~~~~~~~~~

Normally, symbols, polygons and lines will be rendered using any
fill and outline options you have given on the command line, similarly to how
the regular built-in symbols behave. For **M**, **T**, and all the lower-case
symbol codes you may optionally append specific pens (with **-W**\ *pen*) and fills (with
**-G**\ *pen*).  These options will force the use of these settings and
ignore any pens and fills you may or may not have specified on the command line.
Passing **-G**- or **-W**- means a symbol or polygon will have no
fill or outline, respectively, regardless of what your command line settings are.
Unlike pen options on the command line, a pen setting inside the macro symbol
offers more control.  Here, pen width is a *dimension* and you can specify
it in three different ways: (1) Give a fixed pen width with trailing unit (e.g., **-W**\ 1p,red);
we then apply that pen exactly as it is regardless of the size of the symbol,
(2) give a normalized pen thickness in the 0-1 range (e.g., **-W**\ 0.02);
at run-time this thickness will be multiplied by the current symbol size to yield
the actual pen thickness, and (3) specify a variable pen thickness (e.g., **-W**\ $1,blue); we then
obtain the actual pen thickness from the data record at run-time.
Finally, you may indicate that a symbol or polygon should be filled using the color
of the current pen instead of the current fill; do this by specifying **-G+p**.
Likewise, you may indicate that an outline should be drawn with the color of the
current fill instead of the current pen; do this by appending **+g** to your
**-W** setting (which may also indicate pen thickness and texture).  E.g.,
**-W**\ 1p,-+g would mean "draw the outline with a 1p thick dashed pen but obtain
the color from the current fill".

Symbol substitution
~~~~~~~~~~~~~~~~~~~

Custom symbols that need to plot any of the standard geometric symbols
(i.e., those controlled by a single size) can make the symbol code a variable.  By specifying **?** instead
of the symbol codes **a**, **c**, **d**, **g**, **h**, **i**, **n**, **+**, **s**, **t**,
**x**, **-**, or **y** the actual symbol code is expected to be found at the end of
each data record.  Such custom symbols must be invoked with **-SK** rather than **-Sk**.

Text substitution
~~~~~~~~~~~~~~~~~

Normally, the **l** macro code will place a hard-wired text string.  However,
you can also obtain the entire string from your input file via a single symbol
variable **$t** that must be declared with type **s** (string).  The string will be taken
as all trialing text in your data record.  To select a single word from the trailing text
you just use **$t**\ *k*, where *k* starts at 0 for the first word, regardless of how many numerical
columns that precede it.  For each word you plan to use you must add a type **s** above.
Words must be separated by one tab or space only.  To place the dollar sign $ itself you must
use octal \\044 so as to not confuse the parser with a symbol variable.
The string itself, if obtained from the symbol definition file,
may contain special codes that will be expanded given information from the current record.  You
can embed the codes %X or %Y to add the current longitude (or x) and latitude (or y) in
your label string. You may also use $n (*n* is 1, 2, etc.) to embed a numerical symbol variable as text.
It will be formatted according to :term:`FORMAT_FLOAT_MAP`,
unless you append the modifiers **+X** (format as longitude via :term:`FORMAT_GEO_MAP`),
**+Y** (format as latitude via :term:`FORMAT_GEO_MAP`), or **+T** (format as calendar time via
:term:`FORMAT_DATE_MAP` and :term:`FORMAT_CLOCK_MAP`.

Text alignment and font attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Like the **Sl** symbol in :doc:`/plot`, you can change the current
font by appending to **l** the modifier **+f**\ *font* [:term:`FONT_ANNOT_PRIMARY`] and change the text justification
by appending the modifier **+j**\ *justify* [CM]. **Note**: Here, the *font* specification
will only be considered for the font type and not its size (which is set separately by your *size*
argument) or color and outline (which are set separately by **-G** and **-W** arguments).
Finally, there are two ways to specify the font size.  If a fixed font size is given in points
(e.g,, 12p) then the text will be set at that size regardless of the symbol size specified in **-S**.
Without the trailing **p** we interpret the size as a relative size in the 0-1 range and the actual
font size will then scale with the symbol size, just like other symbol items.

Conditional statements
~~~~~~~~~~~~~~~~~~~~~~

There are two types of conditional statements in the macro language: A
simple condition preceding a single command, or a more elaborate
if-then-elseif-else construct. In any test you may use one (and only
one) of many logical operators, as listed in Table :ref:`custop <tbl-custop>`.

.. _tbl-custop:

+----------------+----------------------------------------------------------+
| **Operator**   | **Purpose**                                              |
+================+==========================================================+
| <              | Is *left* less than *right*?                             |
+----------------+----------------------------------------------------------+
| <=             | Is *left* less than or equal to *right*?                 |
+----------------+----------------------------------------------------------+
| ==             | Is *left* equal to *right*?                              |
+----------------+----------------------------------------------------------+
| !=             | Is *left* not equal to *right*?                          |
+----------------+----------------------------------------------------------+
| >=             | Is *left* greater than or equal to *right*?              |
+----------------+----------------------------------------------------------+
| >              | Is *left* greater than *right*?                          |
+----------------+----------------------------------------------------------+
| %              | Does *left* have a remainder with *right*?               |
+----------------+----------------------------------------------------------+
| !%             | Is *left* an exact multiple of *right*?                  |
+----------------+----------------------------------------------------------+
| <>             | Is *left* within the exclusive range of *right*?         |
+----------------+----------------------------------------------------------+
| []             | Is *left* within the inclusive range of *right*?         |
+----------------+----------------------------------------------------------+
| <]             | Is *left* within the in/ex-clusive range of *right*?     |
+----------------+----------------------------------------------------------+
| [>             | Is *left* within the ex/in-clusive range of *right*?     |
+----------------+----------------------------------------------------------+

Above, *left* refers to one of your variable arguments (e.g., $1, $2) or any constant
(e.g. 45, 2c, 1i) on the left hand side of the operator.  On the right hand side of the
operator, *right* is either one of your other variables, or a constant, or a range indicated by
two colon-separated constants or variables (e.g., 10:50, $2:60, $3:$4, etc.).
You can also use $x and $y for tests involving the current point's longitude (or *x*) and
latitude (or *y*) values, respectively.  Note that any tests involving $x will not consider
the periodicity of longitudes.  Finally, $s can be used to access the current symbol size.
Note that symbol size internally is converted to inches so any test you write that compares
the size to a constant should use a constant with the appropriate unit appended (e.g., 2c).
For text comparison note that case will be considered, so "A" does not equal "a".

Simple conditional test
^^^^^^^^^^^^^^^^^^^^^^^

The simple if-test uses a one-line format, defined as

    **if** *left* *operator* *right* **then** *command*

where *left* must be one of the symbol parameters, specified as $1, $2,
$3, etc., or a constant. You must document what these additional parameters control. For
example, to plot a small cyan circle at (0.2, 0.3) with diameter 0.4
only if $2 exceeds 45 you would write

    ::

     if $2 > 45 then 0.2 0.3 0.4 c -Gcyan

Note that this form of the conditional test has no mechanism for an
**else** branch, but this can be accomplished by repeating the test but
reversing the logic for the second copy, e.g.,

    ::

     if $1 > 10 then 0 0 0.5 c -Gred
     if $1 <= 10 then 0 0 0.5 c -Gblue

or you may instead consider the complete conditional construct below.
Using a comparison between variables is similarly straightforward:

    ::

     if $2 > $3 then 0.2 0.3 0.4 c -Ggreen


If you are comparing text strings then $t can be on either side of the operator and
the other side would be a string constant (in quotes if containing spaces).

Complete conditional test
^^^^^^^^^^^^^^^^^^^^^^^^^

The complete conditional test uses a multi-line format, such as

| **if** *left* *operator* *right* **then** {
|  <one or more lines with commands>
| } **elseif** *left* *operator* *right* **then** {
|  <one or more lines with commands>
| } **else** {
|  <one or more lines with commands>
| }

The **elseif** (one or more) and **else** branches are optional. Note
that the syntax is strictly enforced, meaning the opening brace must
appear after **then** with nothing following it, and the closing brace
must appear by itself with no other text, and that the **elseif** and
**else** statements must have both closing and opening braces on the
same line (and nothing else). If you need comments please add them as
separate lines.  You may nest tests as well (up to 10
levels deep), e.g.,

   ::

    if $1 > 45 then {
            if $2 [> 0:10 then 0 0 0.5 c -Gred
    } elseif $1 < 15 then {
            if $2 [> 0:10 then 0 0 0.5 c -Ggreen
    } else {
            if $2 [> 10:20 then {
                    0 0 M -W1p,blue
                    0.3 0.3 D
                    S
                    0.3 0.3 0.3 c -Gcyan
            }
    }
