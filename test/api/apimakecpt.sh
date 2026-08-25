#!/usr/bin/env bash
#
# Test the C API for creating a CPT into memory then write to file output.cpt.

testapi_makecpt
gmt makecpt -Ccool -T1/65/1 -N > answer.cpt
diff answer.cpt output.cpt > fail
