PostScript Fonts Used by GMT
==============================

GMT uses the standard 35 fonts that come with most
PostScript laserwriters. If your printer does not support some of
these fonts, it will automatically substitute the default font (which is
usually Courier). The following is a list of the GMT fonts:

.. figure:: /_images/GMT_App_G.*
   :width: 500 px
   :align: center

   The standard 35 PostScript fonts recognized by GMT.


For the special fonts Symbol (12) and ZapfDingbats (34), see the octal
charts in Chapter :doc:`octal-codes`. When specifying fonts in GMT, you can
either give the entire font name *or* just the font number listed in
this table. To change the fonts used in plotting basemap frames, see the
man page for :doc:`/gmt.conf`. For direct
plotting of text-strings, see the man page for :doc:`/text`.

.. _non-default-fonts:

Using non-default fonts with GMT
--------------------------------

To add additional fonts that you may have purchased or that are
available freely in the internet or at your institution, you will need
to tell GMT some basic information about such fonts. GMT does
not actually read or process any font files and thus does not know anything about
installed fonts and their metrics. In order to use extra fonts in
GMT you need to specify the PostScript name of the relevant fonts in
the file ``PSL_custom_fonts.txt``. We recommend you place this file in
your GMT user directory (~/.gmt) as GMT will look there as well as in your
home directory.  Below is an example of a typical entry for two separate fonts::

    LinBiolinumO      0.700    0
    LinLibertineOB    0.700    0

The format is a space delimited list of the PostScript font name, the
font's height-point size-ratio, and a boolean variable that tells GMT to
re-encode the font (if set to zero). The latter has to be set to zero as
additional fonts will most likely not come in standard
PostScript encoding. GMT determines how tall typical annotations
might be from the font size ratio so that the vertical position of
labels and titles can be adjusted to a more uniform typesetting. This
ratio can be estimated from the height of the letter **A** for a unit font size.
Now, you can set the GMT font parameters to your non-standard fonts::

    gmt set FONT              LinBiolinumO \
            FONT_TITLE        28p,LinLibertineOB \
            PS_CHAR_ENCODING  ISO-8859-1 \
            MAP_DEGREE_SYMBOL degree

After setting the encoding and the degree symbol, the configuration part
for GMT is finished and you can proceed to create GMT maps as usual.
An example script is discussed in Example :ref:`example_31`.

Embedding fonts in PostScript and PDF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you have Type 1 fonts in PFA (Printer Font ASCII) format you can
embed them directly by copying them at the very top of your
PostScript file, before even the %!PS header comment. PFB (Printer
Font Binary), TrueType or OpenType fonts cannot be embedded in
PostScript directly and therefore have to be converted to PFA first.

However, you most likely will have to tell Ghostscript where to
find your custom fonts in order to convert your GMT PostScript plot
to PDF or an image with :doc:`/psconvert`.
When you have used the correct PostScript names of the fonts in ``PSL_custom_fonts.txt`` you
only need to point the ``GS_FONTPATH`` environment variable to the
directory where the font files can be found and invoke
:doc:`/psconvert` in the usual way. Likewise
you can specify Ghostscript's ``-sFONTPATH`` option on the
command line with ``-C-sFONTPATH=/path/to/fontdir``. Ghostscript,
which is invoked by :doc:`/psconvert`, does
not depend on file names. It will automatically find the relevant font
files by their PostScript names and embed and subset them in
PDF files. This is quite convenient as the document can be displayed and
printed even on other computers when the font is not available locally.
There is no need to convert your fonts as Ghostscript can handle
all Type 1, TrueType and OpenType fonts. Note also, that you do not need
to edit Ghostscript's Fontmap.

If you do not want or cannot embed the fonts you can convert them to
outlines (shapes with fills) with Ghostscript in the following
way::

     gs -q -dNOCACHE -dNOSAFER -dNOPAUSE -dBATCH -dNOPLATFONTS \
        -sDEVICE=ps2write -sFONTPATH="/path/to/fontdir" \
        -sOutputFile=mapWithOutlinedFonts.ps map.ps

Note, that this only works with the *ps2write* device. If you need
outlined fonts in PDF, create the PDF from the converted
PostScript file. Also, :doc:`/psconvert`
cannot correctly crop Ghostscript converted PostScript files
anymore. Use Heiko Oberdiek's instead or crop with
:doc:`/psconvert` **-A** **-Te** before (See Example :ref:`example_31`).

Character encoding
~~~~~~~~~~~~~~~~~~

Since PostScript itself does not support Unicode fonts,
Ghostscript will re-encode the fonts on the fly. You have to make
sure to set the correct :term:`PS_CHAR_ENCODING`
with :doc:`/gmtset` and save your
script file with the same character encoding. Alternatively, you can
substitute all non ASCII characters with their corresponding octal
codes, e.g., \\265 instead of μ. Note, that PostScript fonts support
only a small range of glyphs and you may have to switch the
:term:`PS_CHAR_ENCODING` within your script.
