#if 0
    Copyright 1996, UCAR/Unidata
    See netcdf/COPYRIGHT file for copying and redistribution conditions.
    $Id$
#endif


C
C Use for logging error messages
C
        subroutine error(msg)
        implicit        none
        character*(*)   msg
#include "tests.inc"

        nfails = nfails + 1
        if (nfails .le. max_nmpt) print *, msg
        end


C
C Use for logging error conditions
C
        subroutine errori(msg, i)
        implicit        none
        character*(*)   msg
        integer         i
#include "tests.inc"

        nfails = nfails + 1
        if (nfails .le. max_nmpt) print *, msg, i
        end


C
C Use for logging error conditions
C
        subroutine errord(msg, d)
        implicit        none
        character*(*)   msg
        doubleprecision d
#include "tests.inc"

        nfails = nfails + 1
        if (nfails .le. max_nmpt) print *, msg, d
        end


C
C Use for logging error conditions
C
        subroutine errorc(msg, string)
        implicit        none
        character*(*)   msg
        character*(*)   string
#include "tests.inc"

        nfails = nfails + 1
        if (nfails .le. max_nmpt) print *, msg, 
     +      string(1:len_trim(string))
        end


C
C Use for logging error conditions
C
        subroutine errore(msg, err)
        implicit        none
        character*(*)   msg
        integer         err
#include "tests.inc"

        nfails = nfails + 1
        call errorc(msg, nf_strerror(err))
        end
