.. _Chart-Octal-Codes-for-Chars:

Chart of Octal Codes for Characters
===================================

The characters and their octal codes in the Standard and ISOLatin1
encoded fonts are shown in
Figure :ref:`Octal codes for Standard and ISO <Octal_codes_stand_iso>`. Light red areas signify
codes reserved for control characters. In order to use all the extended
characters (shown in the light green boxes) you need to set
:term:`PS_CHAR_ENCODING` to Standard+ or ISOLatin1+ in your :doc:`/gmt.conf` file [29]_.

**Download PDF version:** :download:`GMT Standard+ and ISOLatin1+ octal codes </_images/GMT_App_F_stand+_iso+.pdf>`

.. _Octal_codes_stand_iso:

.. figure:: /_images/GMT_App_F_stand+_iso+.*
   :width: 500 px
   :align: center

   Octal codes and corresponding symbols for StandardEncoding (left) and ISOLatin1Encoding (right) fonts.

The chart for the Symbol character set (GMT font number 12) and Pifont
ZapfDingbats character set (font number 34) are presented in
Figure :ref:`Octal codes for Symbol and ZapfDingbats <Octal_codes_symbol_zap>` below. The octal code
is obtained by appending the column value to the \\??
value, e.g., :math:`\partial` is \\266 in the Symbol
font. The euro currency symbol is \\240 in the Symbol
font and will print if your printer supports it (older printer's
firmware will not know about the euro).

**Download PDF version:** :download:`GMT Symbol and ZapfDingbats octal codes </_images/GMT_App_F_symbol_dingbats.pdf>`

.. _Octal_codes_symbol_zap:

.. figure:: /_images/GMT_App_F_symbol_dingbats.*
   :width: 500 px
   :align: center

   Octal codes and corresponding symbols for Symbol (left) and ZapfDingbats (right) fonts.

Footnote
--------

.. [29]
   If you chose SI units during the installation then the default
   encoding is ISOLatin1+, otherwise it is Standard+.
