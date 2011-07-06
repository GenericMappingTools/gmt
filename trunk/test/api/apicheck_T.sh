#!/bin/bash
# Test the C API for i/o involving textsets

. ../functions.sh
function textset_check {
	testapi -I$1 -W$2 -Tt
	diff -q --strip-trailing-cr ttest[io].txt >> fail
}
header "Test the API for various TEXTSET i/o combinations"

cat << EOF > ttesti.txt
> first segment
0 0 This is some sentence
0 1 that is followed by this sentence.
> second segment
0 2 Moving on to the next segment,
0 3 we find that there is more test,
0 4 in fact one extra line.
EOF
rm -f fail
# 1. Read File and write TEXTSET via...
textset_check f f	# 1a. File
textset_check f s	# 1b. Stream
textset_check f d	# 1c. File descriptor
textset_check f c	# 1d. Copy
textset_check f r	# 1e. Reference
# 2. Read Stream and write TEXTSET via...
textset_check s f	# 2a. File
textset_check s s	# 2b. Stream
textset_check s d	# 2c. File descriptor
textset_check s c	# 2d. Copy
textset_check s r	# 2e. Reference
# 3. Read File Descriptor and write TEXTSET via...
textset_check d f	# 3a. File
textset_check d s	# 3b. Stream
textset_check d d	# 3c. File descriptor
textset_check d c	# 3d. Copy
textset_check d r	# 3e. Reference
# 4. Read Copy and write TEXTSET via...
textset_check c f	# 4a. File
textset_check c s	# 4b. Stream
textset_check c d	# 4c. File descriptor
textset_check c c	# 4d. Copy
textset_check c r	# 4e. Reference
# 5. Read Reference and write TEXTSET via...
textset_check r f	# 5a. File
textset_check r s	# 5b. Stream
textset_check r d	# 5c. File descriptor
textset_check r c	# 5d. Copy
textset_check r r	# 5e. Reference

#rm -f ttest?.txt
passfail apicheck_T
