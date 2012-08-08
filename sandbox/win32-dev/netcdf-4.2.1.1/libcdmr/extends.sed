/package[ ][ ]*ncstream;/a\
\
option config_h="true"; \
option include="nccrnode.h";

/^message[ ][ ]*[a-zA-Z0-9_$]*/a\
    option extends="CRnode node";
