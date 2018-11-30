#!/bin/bash
# Test that our UTF8 to PostScript code works.  We are using all the
# codes in the ISOLatin1+ table.  Not yet covered are what seems to
# be Serbian S, Z with inverted hat

ps=utf8.ps
gmt pstext -R-1/6/0/9 -Jx1i -P  -B0 -F+f24p+jBL --PS_CHAR_ENCODING=ISOLatin1+ << EOF > utf8.ps
0 8 ÀÁÂÃÄÅÆÇ
0 7 ÈÉÊEÌÍÎÏ
0 6 ÐÑÒÓÔÕÖ×Ÿ
0 5 ØÙÚÛÜÝÞß
0 4 àáâãäåæç
0 3 èéêeìíîï
0 2 ðñòóôõö÷
0 1 øùúûüýþÿ
EOF
