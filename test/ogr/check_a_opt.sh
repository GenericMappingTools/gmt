#!/usr/bin/env bash
#
# Test that the -a option correctly retrieves the desired output
# Make a tiny GSFML OGR file using first 2 records of an actual file
cat << EOF > gsml_testfile.gmt
# @VGMT1.0 @GPOINT
# @R-151.97998/-150.94748>	<-73.89936/-73.70616                              
# @Jp"+proj=longlat +datum=WGS84 +no_defs "
# @Jw"GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_84\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.017453292519943295]]"
# @NChron|AnomalyEnd|AnomEndQua|CruiseName|Reference|DOI|GeeK2007|IDMethod
# @Tstring|string|integer|string|string|string|double|string
# FEATURE_DATA
# @DC32n.1n|o|1|V2222|Wobbe++_2012_G3|10.1029/2011GC003742|71.338|"Magnetic Anomaly"
-151.97998	-73.89936
# @DC32n.1n|o|1|V2222|Wobbe++_2012_G3|10.1029/2011GC003742|71.338|"Magnetic Anomaly"
-150.94748	-73.70616
EOF
# answer.txt is the expected output of the commands below by manual verification
# IF YOU CHANGE THE COMMANDS BELOW THEN PROBABLY NEED TO CHANGE answer.txt as well
cat << EOF > answer.txt
# Run gmt convert without -a:
-151.97998	-73.89936
-150.94748	-73.70616
# Run gmt convert with -a no arguments
-151.97998	-73.89936	1	71.338	C32n.1n	o	V2222	Wobbe++_2012_G3	10.1029/2011GC003742	Magnetic Anomaly
-150.94748	-73.70616	1	71.338	C32n.1n	o	V2222	Wobbe++_2012_G3	10.1029/2011GC003742	Magnetic Anomaly
# Run gmt convert with -a just all text aspatial values
-151.97998	-73.89936	C32n.1n	o	V2222	Wobbe++_2012_G3	10.1029/2011GC003742	Magnetic Anomaly
-150.94748	-73.70616	C32n.1n	o	V2222	Wobbe++_2012_G3	10.1029/2011GC003742	Magnetic Anomaly
# Run gmt convert with -a some text aspatial values
-151.97998	-73.89936	C32n.1n	10.1029/2011GC003742
-150.94748	-73.70616	C32n.1n	10.1029/2011GC003742
# Run gmt convert with -a all numerical aspatial values
-151.97998	-73.89936	1	71.338
-150.94748	-73.70616	1	71.338
# Same but flip order via specific column assignments
-151.97998	-73.89936	71.338	1
-150.94748	-73.70616	71.338	1
# Run gmt convert with -a with a mix of numerical and text requests
-151.97998	-73.89936	71.338	1	C32n.1n
-150.94748	-73.70616	71.338	1	C32n.1n
EOF
# Run the test
echo "# Run gmt convert without -a:" > result.txt
gmt convert gsml_testfile.gmt >> result.txt
echo "# Run gmt convert with -a no arguments" >> result.txt
gmt convert -a gsml_testfile.gmt >> result.txt
echo "# Run gmt convert with -a just all text aspatial values" >> result.txt
gmt convert -aChron,AnomalyEnd,CruiseName,Reference,DOI,IDMethod gsml_testfile.gmt >> result.txt
echo "# Run gmt convert with -a some text aspatial values" >> result.txt
gmt convert -aChron,DOI gsml_testfile.gmt >> result.txt
echo "# Run gmt convert with -a all numerical aspatial values" >> result.txt
gmt convert -aAnomEndQua,GeeK2007 gsml_testfile.gmt >> result.txt
echo "# Same but flip order via specific column assignments" >> result.txt
gmt convert -a3=AnomEndQua,2=GeeK2007 gsml_testfile.gmt >> result.txt
echo "# Run gmt convert with -a with a mix of numerical and text requests" >> result.txt
gmt convert -a3=AnomEndQua,Chron,2=GeeK2007 gsml_testfile.gmt >> result.txt

# An empty fail file means success
diff -q --strip-trailing-cr result.txt answer.txt > fail
