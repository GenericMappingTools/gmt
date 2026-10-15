#!/usr/bin/env bash
#
# GMT must survive a long list of shared libraries.  The list starts with 8 entries and grows
# by realloc, which left the new entries uninitialized, so dlclose/FreeLibrary was handed a
# garbage handle at the end of the session (a crash on Linux, "Failure while closing GMT <lib>
# shared library" on Windows).  Here GMT_CUSTOM_LIBS points to a directory with 9 libraries.

rm -f fail
# Let glibc hand out dirty memory so the bug shows up there too; ignored by other C runtimes
export MALLOC_PERTURB_=165

# Empty files are enough since the core module below never opens any of them
mkdir -p plugins
for k in 1 2 3 4 5 6 7 8 9; do
	: > plugins/libfake$k.so
	: > plugins/libfake$k.dll
done

gmt set GMT_CUSTOM_LIBS plugins/
gmt gmtmath -T0/1/1 T = > out.txt 2> err.txt
# Any message on stderr is a failure
[ -s err.txt ] && cat err.txt >> fail
cat << EOF > truth.txt
0	0
1	1
EOF
diff -q out.txt truth.txt > /dev/null || echo "Wrong output" >> fail
