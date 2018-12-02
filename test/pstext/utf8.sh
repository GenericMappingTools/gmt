#!/bin/bash
# Test that our UTF8 to PostScript code works.  We are using all the
# letter codes in the ISOLatin1+ table.

ps=utf8.ps
gmt pstext -R-1/6/0.6/9.6 -Jx0.9i -P -B0 -B+t"UTF-8 via ISOLatin1+" -F+f24p+jBL --PS_CHAR_ENCODING=ISOLatin1+ << EOF > utf8.ps
0 9 Š Ž š ž Œ œ Ÿ Ł ł
0 8 À Á Â Ã Ä Å Æ Ç
0 7 È É Ê E Ì Í Î Ï
0 6 Ð Ñ Ò Ó Ô Õ Ö ×
0 5 Ø Ù Ú Û Ü Ý Þ ß
0 4 à á â ã ä å æ ç
0 3 è é ê e ì í î ï
0 2 ð ñ ò ó ô õ ö ÷
0 1 ø ù ú û ü ý þ ÿ
EOF
