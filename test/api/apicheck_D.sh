#!/bin/bash
# Test the C API for i/

. ../functions.sh
header "Test the API for various i/o combinations"

# DATASETS
# 1. Read File and write datasets via...
# 1a. File
testapi -If -Wf -Td
diff -q dtest[io].txt > fail
# 1b. Stream
testapi -If -Ws -Td
diff -q dtest[io].txt >> fail
# 1c. File descriptor
testapi -If -Wd -Td
diff -q dtest[io].txt >> fail
# 1d. Copy
testapi -If -Wc -Td
diff -q dtest[io].txt >> fail
# 1e. Reference
testapi -If -Wr -Td
diff -q dtest[io].txt >> fail
# 2. Read Stream and write datasets via...
# 2a. File
testapi -Is -Wf -Td
diff -q dtest[io].txt >> fail
# 2b. Stream
testapi -Is -Ws -Td
diff -q dtest[io].txt >> fail
# 2c. File descriptor
testapi -Is -Wd -Td
diff -q dtest[io].txt >> fail
# 2d. Copy
testapi -Is -Wc -Td
diff -q dtest[io].txt >> fail
# 2e. Reference
testapi -Is -Wr -Td
diff -q dtest[io].txt >> fail
# 3. Read File Descriptor and write datasets via...
# 3a. File
testapi -Id -Wf -Td
diff -q dtest[io].txt >> fail
# 3b. Stream
testapi -Id -Ws -Td
diff -q dtest[io].txt >> fail
# 3c. File descriptor
testapi -Id -Wd -Td
diff -q dtest[io].txt >> fail
# 3d. Copy
testapi -Id -Wc -Td
diff -q dtest[io].txt >> fail
# 3e. Reference
testapi -Id -Wr -Td
diff -q dtest[io].txt >> fail
# 4. Read Copy and write datasets via...
# 4a. File
testapi -Ic -Wf -Td
diff -q dtest[io].txt >> fail
# 4b. Stream
testapi -Ic -Ws -Td
diff -q dtest[io].txt >> fail
# 4c. File descriptor
testapi -Ic -Wd -Td
diff -q dtest[io].txt >> fail
# 4d. Copy
testapi -Ic -Wc -Td
diff -q dtest[io].txt >> fail
# 4e. Reference
testapi -Ic -Wr -Td
diff -q dtest[io].txt >> fail
# 5. Read Reference and write datasets via...
# 5a. File
testapi -Ir -Wf -Td
diff -q dtest[io].txt >> fail
# 5b. Stream
testapi -Ir -Ws -Td
diff -q dtest[io].txt >> fail
# 5c. File descriptor
testapi -Ir -Wd -Td
diff -q dtest[io].txt >> fail
# 5d. Copy
testapi -Ir -Wc -Td
diff -q dtest[io].txt >> fail
# 5e. Reference
testapi -Ir -Wr -Td
diff -q dtest[io].txt >> fail

rm -f dtesto.txt
passfail apicheck_D
