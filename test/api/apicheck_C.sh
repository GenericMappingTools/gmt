#!/bin/bash
# Test the C API for i/o involving CPTs

. functions.sh

function cpt_check {
	testapi -I$1 -W$2 -Tc
	diff -q --strip-trailing-cr ctest[io].cpt >> fail
}

header "Test the API for various CPT i/o combinations"

makecpt -Cno_green -T0/10/1 > ctesti.cpt
rm -f fail
# 1. Read File and write CPT via...
cpt_check f f	# 1a. File
cpt_check f s	# 1b. Stream
cpt_check f d	# 1c. File descriptor
cpt_check f c	# 1d. Copy
cpt_check f r	# 1e. Reference
# 2. Read Stream and write CPT via...
cpt_check s f	# 2a. File
cpt_check s s	# 2b. Stream
cpt_check s d	# 2c. File descriptor
cpt_check s c	# 2d. Copy
cpt_check s r	# 2e. Reference
# 3. Read File Descriptor and write CPT via...
cpt_check d f	# 3a. File
cpt_check d s	# 3b. Stream
cpt_check d d	# 3c. File descriptor
cpt_check d c	# 3d. Copy
cpt_check d r	# 3e. Reference
# 4. Read Copy and write CPT via...
cpt_check c f	# 4a. File
cpt_check c s	# 4b. Stream
cpt_check c d	# 4c. File descriptor
cpt_check c c	# 4d. Copy
cpt_check c r	# 4e. Reference
# 5. Read Reference and write CPT via...
cpt_check r f	# 5a. File
cpt_check r s	# 5b. Stream
cpt_check r d	# 5c. File descriptor
cpt_check r c	# 5d. Copy
cpt_check r r	# 5e. Reference

passfail apicheck_C
