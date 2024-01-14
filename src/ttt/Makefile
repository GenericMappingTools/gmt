#	$Id: Makefile,v 1.7 2017/08/02 05:06:54 myself Exp $
#
#	Makefile to manage debug vs scrambled code.


#---------Stop here, no more edits----------------------

help::
	@grep '^#!' Makefile | cut -c3-
#!-------------------- MAKE HELP FOR TTT --------------------
#!
#!make <target>, where <target> can be:
#!
#!scramble  : Scramble TTT to TTTOBF and place in src for release
#!debug     : Copy TTT to TTTOBF and place in src for debug
#!clean     : Remove files we created in src 
#!

# Obfuscating the source code and removing comments

scramble:
		scramble.sh ttt OBF
		sed '/^$$/q' ttt.c > ../src/tttOBF.c
		sed '/^$$/q' ttt.h > ../src/tttOBF.h
		cstrip.sh ttt.c       | grep '[^[:space:]]' | sed -f scramble.sed >> ../src/tttOBF.c
		cstrip.sh ttt.h       | grep '[^[:space:]]' | sed -f scramble.sed >> ../src/tttOBF.h
		cstrip.sh tttfast.c   | grep '[^[:space:]]' | sed -f scramble.sed >  ../src/tttfastOBF.c
		cstrip.sh ttt_macro.h | grep '[^[:space:]]' | sed -f scramble.sed >  ../src/tttOBF_macro.h
		cstrip.sh ttt_subs.c  | grep '[^[:space:]]' | sed -f scramble.sed >  ../src/tttOBF_subs.c
		cstrip.sh ttt_pick.c  | grep '[^[:space:]]' | sed -f scramble.sed >  ../src/tttOBF_pick.c
		cp ttt_i.h ttt_j.h ../src
		#rm -f scramble.sed

debug:
		scramble.sh ttt OBF debug
		sed -f scramble.sed < ttt.c >  ../src/tttOBF.c
		sed -f scramble.sed < tttfast.c > ../src/tttfastOBF.c
		sed -f scramble.sed < ttt.h > ../src/tttOBF.h
		sed -f scramble.sed < ttt_macro.h > ../src/tttOBF_macro.h
		sed -f scramble.sed < ttt_subs.c > ../src/tttOBF_subs.c
		sed -f scramble.sed < ttt_pick.c > ../src/tttOBF_pick.c
		cp ttt_i.h ttt_j.h ../src
		rm -f scramble.sed

clean:
		rm -f ../src/tttOBF.c ../src/tttfastOBF.c ../src/tttOBF_subs.c ../src/ttt_subs.c \
			../src/tttOBF_pick.c ../src/tttOBF.h ../src/ttt.h ../src/ttt_i.h ../src/ttt_j.h \
			../src/ttt_macro.h ../src/tttOBF_macro.h
