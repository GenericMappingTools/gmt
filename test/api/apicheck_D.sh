#!/bin/bash
# Test the C API for i/o involving datasets

. ./functions.sh
function dataset_check {
	testapi -I$1 -W$2 -Td
	diff -q --strip-trailing-cr dtest[io].txt >> fail
}
header "Test the API for various DATASET i/o combinations"

cat << EOF > dtesti.txt
> first seg
1	10
2	20
3	30
> 2nd seg
5	50
6	60
7	70
8	80
EOF
rm -f fail
# 1. Read File and write DATASET via...
dataset_check f f	# 1a. File
dataset_check f s	# 1b. Stream
dataset_check f d	# 1c. File descriptor
dataset_check f c	# 1d. Copy
dataset_check f r	# 1e. Reference
# 2. Read Stream and write DATASET via...
dataset_check s f	# 2a. File
dataset_check s s	# 2b. Stream
dataset_check s d	# 2c. File descriptor
dataset_check s c	# 2d. Copy
dataset_check s r	# 2e. Reference
# 3. Read File Descriptor and write DATASET via...
dataset_check d f	# 3a. File
dataset_check d s	# 3b. Stream
dataset_check d d	# 3c. File descriptor
dataset_check d c	# 3d. Copy
dataset_check d r	# 3e. Reference
# 4. Read Copy and write DATASET via...
dataset_check c f	# 4a. File
dataset_check c s	# 4b. Stream
dataset_check c d	# 4c. File descriptor
dataset_check c c	# 4d. Copy
dataset_check c r	# 4e. Reference
# 5. Read Reference and write DATASET via...
dataset_check r f	# 5a. File
dataset_check r s	# 5b. Stream
dataset_check r d	# 5c. File descriptor
dataset_check r c	# 5d. Copy
dataset_check r r	# 5e. Reference

passfail apicheck_D
