Using LaTeX Expressions in GMT
==============================

GMT supports the use of LaTeX equations embedded in text strings that are used
with **-B** in titles, subtitles and axis labels, as well as for single lines
of text placed via :doc:`/text`.  These expressions must be enclosed by the
marker @[ (e.g., "Plotting @[\\Delta \\sigma_{xx}^2@["), or alternatively enclose
the expressions with <math> and </math> (e.g., "Plotting <math>\\Delta \\sigma_{xx}^2</math>").
If these markers are found, the entire line will be converted to an
Encapsulated PostScript file (EPS) via system commands to latex and dvips,
which must be available on your system.  The EPS file is then placed in the
location instead of the regular text.


.. figure:: /_images/GMT_latex.*
   :width: 500 px
   :align: center

   Example of using a LaTeX equation in the x-axis label via -Bxaf+l"@[\\nabla^4 \\psi - \\Delta \\sigma_{xx}^2@[ (MPa)".

.. _gmt-latex-fonts:

GMT fonts and LaTeX
-------------------

LaTeX is a large and complicated type-setting system with many optional packages, and we simply
must assume that your LaTeX installation has all the needed features; see our
`wiki <https://github.com/GenericMappingTools/gmt/wiki>`_ for typical installs
via package managers.  Your installation must have support for the packages *fontenc*
and *inputenc*, as well as the required fonts *helvet, mathptmx, courier, symbol,
avantgar, bookman, newcent, mathpazo, zapfchan* and *zapfding*.  If the conversion
from LaTeX to EPS fails, please follow instructions to run the conversion script
manually in the terminal so you may determine what packages or fonts you may be
missing.  The list of fonts above were selected to match the standard fonts used
by GMT. Hence, if you change the default font for a title (i.e., :term:`FONT_TITLE`),
then we also set that font as the default font in the LaTeX script we generate under
the hood.  This is done so the regular text in a title will follow the current GMT
default settings.
