// Purpose: Converts ida3 format xma data to netcdf4
// Usage:   xma2netcdf <shot number>


#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <netcdf>
#include <iomanip>
#include "test_utilities.h"
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

int main()
{
  try
    {
      {
	cout<<"Opening file \"firstFile.cdf\" with NcFile::replace"<<endl;
	NcFile ncFile("firstFile.cdf",NcFile::replace);
    
	cout<<left<<setw(55)<<"Testing addGroup(\"groupName\")";
	NcGroup groupA(ncFile.addGroup("groupA"));
	NcGroup groupA0(ncFile.addGroup("groupA0"));
	NcGroup groupB(groupA.addGroup("groupB"));
	NcGroup groupC(groupA.addGroup("groupC"));
	cout <<"    -----------   passed\n";
    
	cout <<left<<setw(55)<<"Testing addDim(\"dimensionName\")";
	NcDim dim1 = ncFile.addDim("dim1",11);
	NcDim dim2 = ncFile.addDim("dim2");
	NcDim dim3 = ncFile.addDim("dim3",13);
	NcDim dim4 = groupB.addDim("dim4",14);
	NcDim dim5 = groupB.addDim("dim5",15);
	NcDim dim6 = groupB.addDim("dim6",16);
	NcDim dim7 = groupB.addDim("dim7",17);
	cout <<"    -----------   passed\n";

	cout <<left<<setw(55)<<"Testing addVar(\"varName\",\"typeName\",\"dimName\")";

	NcVar varA1_1  = ncFile.addVar("varA1_1",ncByte,dim1);
	NcVar varA1_2  = ncFile.addVar("varA1_2","byte","dim1");
	vector<NcDim> dimArray(2);
	dimArray[0]=dim1;
	dimArray[1]=dim2;
	vector<string> stringArray(2);
	stringArray[0] = "dim1";
	stringArray[1] = "dim2";
	NcVar varA1_3  = ncFile.addVar("varA1_3",ncByte,dimArray);
	NcVar varA1_4  = ncFile.addVar("varA1_4","byte",stringArray);

	NcVar varA1_5  = groupB.addVar("varA1_5",ncByte,dim4);
	NcVar varA1_6  = groupB.addVar("varA1_6",ncByte,dim2);

	dimArray[0]=dim1;
	dimArray[1]=dim7;
	NcVar varA1_7  = groupB.addVar("varA1_7",ncByte,dimArray);

	dimArray[0]=dim1;
	dimArray[1]=dim2;
	NcVar varA1_8  = groupC.addVar("varA1_8",ncByte,dimArray);
	cout <<"    -----------   passed\n";
      }



      cout<<"Preparing for tests..."<<endl;
      NcFile ncFile("firstFile.cdf",NcFile::replace);
    
      NcGroup groupA(ncFile.addGroup("groupA"));
      NcGroup groupA0(ncFile.addGroup("groupA0"));
      NcGroup groupB(groupA.addGroup("groupB"));
      NcGroup groupC(groupA.addGroup("groupC"));
    
      NcDim dim1 = ncFile.addDim("dim1",11);
      NcDim dim2 = ncFile.addDim("dim2");
      NcDim dim3 = ncFile.addDim("dim3",13);
      NcDim dim4 = groupB.addDim("dim4",14);
      NcDim dim5 = groupB.addDim("dim5",15);
      NcDim dim6 = groupB.addDim("dim6",16);
      NcDim dim7 = groupB.addDim("dim7",17);


      NcVar var_1   = ncFile.addVar("var_1",   ncByte,dim1);
      NcVar varA_1  = groupA.addVar("varA_1",  ncByte,dim1);
      NcVar varA_2  = groupA.addVar("varA_2",  ncByte,dim1);
      NcVar varA0_1 = groupA0.addVar("varA0_1",ncByte,dim1);
      NcVar varA0_2 = groupA0.addVar("varA0_2",ncByte,dim1);
      NcVar varA0_3 = groupA0.addVar("varA0_3",ncByte,dim1);
      NcVar varB_1  = groupB.addVar("varB_1",  ncByte,dim1);
      NcVar varB_2  = groupB.addVar("varB_2",  ncByte,dim1);
      NcVar varB_3  = groupB.addVar("varB_3",  ncByte,dim1);
      NcVar varB_4  = groupB.addVar("varB_4",  ncByte,dim1);
      NcVar varC_1  = groupC.addVar("varC_1",  ncByte,dim1);
      NcVar varC_2  = groupC.addVar("varC_2",  ncByte,dim1);
      NcVar varC_3  = groupC.addVar("varC_3",  ncByte,dim1);
      NcVar varC_4  = groupC.addVar("varC_4",  ncByte,dim1);
      NcVar varC_5  = groupC.addVar("varC_5",  ncByte,dim1);

      {
	cout <<left<<setw(55)<<"Testing addCount([netCDF::Location])";
	if(ncFile.getVarCount() != 1 )                            throw NcException("NcException","Error in test 4.1",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::Current) != 1 )           throw NcException("NcException","Error in test 4.2",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::Parents) != 0)            throw NcException("NcException","Error in test 4.3",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::Children) != 14)          throw NcException("NcException","Error in test 4.4",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::ParentsAndCurrent) != 1 ) throw NcException("NcException","Error in test 4.5",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::ChildrenAndCurrent) !=15 )throw NcException("NcException","Error in test 4.6",__FILE__,__LINE__);
	if(ncFile.getVarCount(NcGroup::All) != 15 )              throw NcException("NcException","Error in test 4.7",__FILE__,__LINE__);
	if(groupB.getVarCount() != 4)                            throw NcException("NcException","Error in test 4.8",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::Current) != 4)            throw NcException("NcException","Error in test 4.9",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::Parents) != 3 )           throw NcException("NcException","Error in test 4.10",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::Children) != 0)           throw NcException("NcException","Error in test 4.11",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::ParentsAndCurrent) != 7 ) throw NcException("NcException","Error in test 4.12",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::ChildrenAndCurrent) != 4) throw NcException("NcException","Error in test 4.13",__FILE__,__LINE__);
	if(groupB.getVarCount(NcGroup::All) != 7 )               throw NcException("NcException","Error in test 4.14",__FILE__,__LINE__);
	if(groupA0.getVarCount() != 3)                           throw NcException("NcException","Error in test 4.15",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::Current) != 3)           throw NcException("NcException","Error in test 4.16",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::Parents) != 1 )          throw NcException("NcException","Error in test 4.17",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::Children) != 0)          throw NcException("NcException","Error in test 4.18",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::ParentsAndCurrent) != 4 )throw NcException("NcException","Error in test 4.19",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::ChildrenAndCurrent) != 3)throw NcException("NcException","Error in test 4.20",__FILE__,__LINE__);
	if(groupA0.getVarCount(NcGroup::All) != 4 )              throw NcException("NcException","Error in test 4.21",__FILE__,__LINE__);

	if(groupA.getVarCount() != 2)                            throw NcException("NcException","Error in test 4.15",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::Current) != 2)            throw NcException("NcException","Error in test 4.16",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::Parents) != 1 )           throw NcException("NcException","Error in test 4.17",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::Children) != 9)           throw NcException("NcException","Error in test 4.18",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::ParentsAndCurrent) != 3 ) throw NcException("NcException","Error in test 4.19",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::ChildrenAndCurrent) !=11) throw NcException("NcException","Error in test 4.20",__FILE__,__LINE__);
	if(groupA.getVarCount(NcGroup::All) != 12 )              throw NcException("NcException","Error in test 4.21",__FILE__,__LINE__);
	cout <<"    -----------   passed\n";
	
      }


      {
	cout <<left<<setw(55)<<"Testing getVars([netCDF::Location])";
	multimap<string,NcVar> groupMap;
	multimap<string,NcVar>::iterator iter;

	groupMap = ncFile.getVars();
	if(groupMap.size() != 1)                                  throw NcException("NcException","Error in test 5.1",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.2",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.3",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.4",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.5",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.6",__FILE__,__LINE__);

	groupMap = ncFile.getVars(NcGroup::Current);
	if(groupMap.size() != 1)                                  throw NcException("NcException","Error in test 5.7",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.8",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.9",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.10",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.11",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.12",__FILE__,__LINE__);

	groupMap = ncFile.getVars(NcGroup::Parents);
	if(groupMap.size() != 0)                                  throw NcException("NcException","Error in test 5.13",__FILE__,__LINE__);

	groupMap = ncFile.getVars(NcGroup::Children);
	if(groupMap.size() != 14)                                 throw NcException("NcException","Error in test 5.14",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.15",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.16",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.17",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.18",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.19",__FILE__,__LINE__);


	groupMap = ncFile.getVars(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 1)                                  throw NcException("NcException","Error in test 5.20",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.21",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.22",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.23",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.24",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.25",__FILE__,__LINE__);


	groupMap = ncFile.getVars(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 15)                                 throw NcException("NcException","Error in test 5.26",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.27",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.28",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.29",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.30",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.31",__FILE__,__LINE__);

	groupMap = ncFile.getVars(NcGroup::All               );
	if(groupMap.size() != 15)                                 throw NcException("NcException","Error in test 5.32",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.33",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.34",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.35",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.36",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.37",__FILE__,__LINE__);

	// now groupC........

	groupMap = groupC.getVars();
	if(groupMap.size() != 5)                                  throw NcException("NcException","Error in test 5.38",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.39",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.40",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.41",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.42",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.43",__FILE__,__LINE__);

	groupMap = groupC.getVars(NcGroup::Current);
	if(groupMap.size() != 5)                                  throw NcException("NcException","Error in test 5.44",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.45",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.46",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.47",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.48",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.49",__FILE__,__LINE__);

	groupMap = groupC.getVars(NcGroup::Parents);
	if(groupMap.size() != 3)                                  throw NcException("NcException","Error in test 5.50",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.51",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.52",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.53",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.54",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.55",__FILE__,__LINE__);

	groupMap = groupC.getVars(NcGroup::Children);
	if(groupMap.size() != 0)                                  throw NcException("NcException","Error in test 5.56",__FILE__,__LINE__);


	groupMap = groupC.getVars(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 8)                                  throw NcException("NcException","Error in test 5.57",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.58",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.59",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.60",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.61",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.62",__FILE__,__LINE__);


	groupMap = groupC.getVars(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 5)                                  throw NcException("NcException","Error in test 5.63",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.64",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.65",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.66",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.67",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.68",__FILE__,__LINE__);

	groupMap = groupC.getVars(NcGroup::All               );
	if(groupMap.size() != 8)                                  throw NcException("NcException","Error in test 5.69",__FILE__,__LINE__);
	iter=groupMap.find("var_1");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.70",__FILE__,__LINE__);
	iter=groupMap.find("varA_1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.71",__FILE__,__LINE__);
	iter=groupMap.find("varA0_1");if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.72",__FILE__,__LINE__);
	iter=groupMap.find("varB_3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 5.73",__FILE__,__LINE__);
	iter=groupMap.find("varC_5"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 5.74",__FILE__,__LINE__);

	cout <<"    -----------   passed\n";




	cout <<left<<setw(55)<<"Testing getVar(\"name\",[netCDF::Location])";
	if(ncFile.getVar("var_1") != var_1)                            throw NcException("NcException","Error in test 6.1",__FILE__,__LINE__);
	if(ncFile.getVar("var_1",NcGroup::Current) != var_1)           throw NcException("NcException","Error in test 6.2",__FILE__,__LINE__);
	if(!ncFile.getVar("var_1",NcGroup::Parents).isNull())          throw NcException("NcException","Error in test 6.3",__FILE__,__LINE__);
	if(!ncFile.getVar("var_1",NcGroup::Children).isNull())         throw NcException("NcException","Error in test 6.4",__FILE__,__LINE__);
	if(ncFile.getVar("var_1",NcGroup::ParentsAndCurrent) != var_1) throw NcException("NcException","Error in test 6.5",__FILE__,__LINE__);
	if(ncFile.getVar("var_1",NcGroup::ChildrenAndCurrent)!= var_1) throw NcException("NcException","Error in test 6.6",__FILE__,__LINE__);
	if(ncFile.getVar("var_1",NcGroup::All) != var_1)               throw NcException("NcException","Error in test 6.7",__FILE__,__LINE__);

	if(!ncFile.getVar("varA_2").isNull())                           throw NcException("NcException","Error in test 6.8",__FILE__,__LINE__);
	if(!ncFile.getVar("varA_2",NcGroup::Current).isNull())          throw NcException("NcException","Error in test 6.9",__FILE__,__LINE__);
	if(!ncFile.getVar("varA_2",NcGroup::Parents).isNull())          throw NcException("NcException","Error in test 6.10",__FILE__,__LINE__);
	if(ncFile.getVar("varA_2",NcGroup::Children) != varA_2)         throw NcException("NcException","Error in test 6.11",__FILE__,__LINE__);
	if(!ncFile.getVar("varA_2",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.12",__FILE__,__LINE__);
	if(ncFile.getVar("varA_2",NcGroup::ChildrenAndCurrent)!= varA_2)throw NcException("NcException","Error in test 6.13",__FILE__,__LINE__);
	if(ncFile.getVar("varA_2",NcGroup::All) != varA_2)              throw NcException("NcException","Error in test 6.14",__FILE__,__LINE__);

	if(!ncFile.getVar("varA0_2").isNull())                             throw NcException("NcException","Error in test 6.15",__FILE__,__LINE__);
	if(!ncFile.getVar("varA0_2",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 6.16",__FILE__,__LINE__);
	if(!ncFile.getVar("varA0_2",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 6.17",__FILE__,__LINE__);
	if(ncFile.getVar("varA0_2",NcGroup::Children) != varA0_2)          throw NcException("NcException","Error in test 6.18",__FILE__,__LINE__);
	if(!ncFile.getVar("varA0_2",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 6.19",__FILE__,__LINE__);
	if(ncFile.getVar("varA0_2",NcGroup::ChildrenAndCurrent)!= varA0_2)throw NcException("NcException","Error in test 6.20",__FILE__,__LINE__);
	if(ncFile.getVar("varA0_2",NcGroup::All) != varA0_2)               throw NcException("NcException","Error in test 6.21",__FILE__,__LINE__);

	if(!ncFile.getVar("varB_3").isNull())                            throw NcException("NcException","Error in test 6.22",__FILE__,__LINE__);
	if(!ncFile.getVar("varB_3",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.23",__FILE__,__LINE__);
	if(!ncFile.getVar("varB_3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.24",__FILE__,__LINE__);
	if(ncFile.getVar("varB_3",NcGroup::Children) != varB_3)          throw NcException("NcException","Error in test 6.25",__FILE__,__LINE__);
	if(!ncFile.getVar("varB_3",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.26",__FILE__,__LINE__);
	if(ncFile.getVar("varB_3",NcGroup::ChildrenAndCurrent)!= varB_3)throw NcException("NcException","Error in test 6.27",__FILE__,__LINE__);
	if(ncFile.getVar("varB_3",NcGroup::All) != varB_3)               throw NcException("NcException","Error in test 6.28",__FILE__,__LINE__);

	if(!ncFile.getVar("varC_5").isNull())                            throw NcException("NcException","Error in test 6.29",__FILE__,__LINE__);
	if(!ncFile.getVar("varC_5",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.30",__FILE__,__LINE__);
	if(!ncFile.getVar("varC_5",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.31",__FILE__,__LINE__);
	if(ncFile.getVar("varC_5",NcGroup::Children) != varC_5)          throw NcException("NcException","Error in test 6.32",__FILE__,__LINE__);
	if(!ncFile.getVar("varC_5",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.33",__FILE__,__LINE__);
	if(ncFile.getVar("varC_5",NcGroup::ChildrenAndCurrent)!= varC_5)throw NcException("NcException","Error in test 6.34",__FILE__,__LINE__);
	if(ncFile.getVar("varC_5",NcGroup::All) != varC_5)               throw NcException("NcException","Error in test 6.35",__FILE__,__LINE__);


	if(!groupA.getVar("var_1").isNull())                            throw NcException("NcException","Error in test 6.36",__FILE__,__LINE__);
	if(!groupA.getVar("var_1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.37",__FILE__,__LINE__);
	if(groupA.getVar("var_1",NcGroup::Parents) != var_1)            throw NcException("NcException","Error in test 6.38",__FILE__,__LINE__);
	if(!groupA.getVar("var_1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.38",__FILE__,__LINE__);
	if(groupA.getVar("var_1",NcGroup::ParentsAndCurrent) != var_1)  throw NcException("NcException","Error in test 6.39",__FILE__,__LINE__);
	if(!groupA.getVar("var_1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.40",__FILE__,__LINE__);
	if(groupA.getVar("var_1",NcGroup::All) != var_1)                throw NcException("NcException","Error in test 6.41",__FILE__,__LINE__);

	if(groupA.getVar("varA_2") != varA_2)                           throw NcException("NcException","Error in test 6.42",__FILE__,__LINE__);
	if(groupA.getVar("varA_2",NcGroup::Current) != varA_2)          throw NcException("NcException","Error in test 6.43",__FILE__,__LINE__);
	if(!groupA.getVar("varA_2",NcGroup::Parents).isNull())          throw NcException("NcException","Error in test 6.44",__FILE__,__LINE__);
	if(!groupA.getVar("varA_2",NcGroup::Children).isNull())         throw NcException("NcException","Error in test 6.45",__FILE__,__LINE__);
	if(groupA.getVar("varA_2",NcGroup::ParentsAndCurrent) != varA_2)throw NcException("NcException","Error in test 6.46",__FILE__,__LINE__);
	if(groupA.getVar("varA_2",NcGroup::ChildrenAndCurrent)!= varA_2)throw NcException("NcException","Error in test 6.47",__FILE__,__LINE__);
	if(groupA.getVar("varA_2",NcGroup::All) != varA_2)              throw NcException("NcException","Error in test 6.48",__FILE__,__LINE__);

	if(!groupA.getVar("varA0_2").isNull())                             throw NcException("NcException","Error in test 6.49",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 6.50",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 6.51",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 6.52",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 6.53",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.54",__FILE__,__LINE__);
	if(!groupA.getVar("varA0_2",NcGroup::All).isNull())                throw NcException("NcException","Error in test 6.55",__FILE__,__LINE__);

	if(!groupA.getVar("varB_3").isNull())                            throw NcException("NcException","Error in test 6.56",__FILE__,__LINE__);
	if(!groupA.getVar("varB_3",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.57",__FILE__,__LINE__);
	if(!groupA.getVar("varB_3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.58",__FILE__,__LINE__);
	if(groupA.getVar("varB_3",NcGroup::Children) != varB_3)          throw NcException("NcException","Error in test 6.59",__FILE__,__LINE__);
	if(!groupA.getVar("varB_3",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.60",__FILE__,__LINE__);
	if(groupA.getVar("varB_3",NcGroup::ChildrenAndCurrent)!= varB_3)throw NcException("NcException","Error in test 6.61",__FILE__,__LINE__);
	if(groupA.getVar("varB_3",NcGroup::All) != varB_3)               throw NcException("NcException","Error in test 6.62",__FILE__,__LINE__);

	if(!groupA.getVar("varC_5").isNull())                            throw NcException("NcException","Error in test 6.63",__FILE__,__LINE__);
	if(!groupA.getVar("varC_5",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.64",__FILE__,__LINE__);
	if(!groupA.getVar("varC_5",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.65",__FILE__,__LINE__);
	if(groupA.getVar("varC_5",NcGroup::Children) != varC_5)          throw NcException("NcException","Error in test 6.66",__FILE__,__LINE__);
	if(!groupA.getVar("varC_5",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.67",__FILE__,__LINE__);
	if(groupA.getVar("varC_5",NcGroup::ChildrenAndCurrent)!= varC_5)throw NcException("NcException","Error in test 6.68",__FILE__,__LINE__);
	if(groupA.getVar("varC_5",NcGroup::All) != varC_5)               throw NcException("NcException","Error in test 6.69",__FILE__,__LINE__);

	if(!groupB.getVar("var_1").isNull())                            throw NcException("NcException","Error in test 6.70",__FILE__,__LINE__);
	if(!groupB.getVar("var_1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.71",__FILE__,__LINE__);
	if(groupB.getVar("var_1",NcGroup::Parents) != var_1)            throw NcException("NcException","Error in test 6.72",__FILE__,__LINE__);
	if(!groupB.getVar("var_1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.73",__FILE__,__LINE__);
	if(groupB.getVar("var_1",NcGroup::ParentsAndCurrent) != var_1)  throw NcException("NcException","Error in test 6.74",__FILE__,__LINE__);
	if(!groupB.getVar("var_1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.75",__FILE__,__LINE__);
	if(groupB.getVar("var_1",NcGroup::All) != var_1)                throw NcException("NcException","Error in test 6.76",__FILE__,__LINE__);

	if(!groupB.getVar("varA_2").isNull())                            throw NcException("NcException","Error in test 6.77",__FILE__,__LINE__);
	if(!groupB.getVar("varA_2",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.78",__FILE__,__LINE__);
	if(groupB.getVar("varA_2",NcGroup::Parents) != varA_2)           throw NcException("NcException","Error in test 6.79",__FILE__,__LINE__);
	if(!groupB.getVar("varA_2",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.80",__FILE__,__LINE__);
	if(groupB.getVar("varA_2",NcGroup::ParentsAndCurrent) != varA_2) throw NcException("NcException","Error in test 6.81",__FILE__,__LINE__);
	if(!groupB.getVar("varA_2",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.82",__FILE__,__LINE__);
	if(groupB.getVar("varA_2",NcGroup::All) != varA_2)               throw NcException("NcException","Error in test 6.83",__FILE__,__LINE__);

	if(!groupB.getVar("varA0_2").isNull())                             throw NcException("NcException","Error in test 6.84",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 6.85",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 6.86",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 6.87",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 6.88",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.89",__FILE__,__LINE__);
	if(!groupB.getVar("varA0_2",NcGroup::All).isNull())                throw NcException("NcException","Error in test 6.90",__FILE__,__LINE__);

	if(groupB.getVar("varB_3") != varB_3)                            throw NcException("NcException","Error in test 6.91",__FILE__,__LINE__);
	if(groupB.getVar("varB_3",NcGroup::Current) != varB_3)           throw NcException("NcException","Error in test 6.92",__FILE__,__LINE__);
	if(!groupB.getVar("varB_3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.93",__FILE__,__LINE__);
	if(!groupB.getVar("varB_3",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.94",__FILE__,__LINE__);
	if(groupB.getVar("varB_3",NcGroup::ParentsAndCurrent) != varB_3)throw NcException("NcException","Error in test 6.95",__FILE__,__LINE__);
	if(groupB.getVar("varB_3",NcGroup::ChildrenAndCurrent)!= varB_3)throw NcException("NcException","Error in test 6.96",__FILE__,__LINE__);
	if(groupB.getVar("varB_3",NcGroup::All) != varB_3)               throw NcException("NcException","Error in test 6.97",__FILE__,__LINE__);

	if(!groupB.getVar("varC_5").isNull())                            throw NcException("NcException","Error in test 6.98",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.99",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 6.100",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.101",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.102",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.103",__FILE__,__LINE__);
	if(!groupB.getVar("varC_5",NcGroup::All).isNull())               throw NcException("NcException","Error in test 6.104",__FILE__,__LINE__);

	if(!groupA0.getVar("var_1").isNull())                            throw NcException("NcException","Error in test 6.105",__FILE__,__LINE__);
	if(!groupA0.getVar("var_1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 6.106",__FILE__,__LINE__);
	if(groupA0.getVar("var_1",NcGroup::Parents) != var_1)            throw NcException("NcException","Error in test 6.107",__FILE__,__LINE__);
	if(!groupA0.getVar("var_1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 6.108",__FILE__,__LINE__);
	if(groupA0.getVar("var_1",NcGroup::ParentsAndCurrent) != var_1)  throw NcException("NcException","Error in test 6.109",__FILE__,__LINE__);
	if(!groupA0.getVar("var_1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.110",__FILE__,__LINE__);
	if(groupA0.getVar("var_1",NcGroup::All) != var_1)                throw NcException("NcException","Error in test 6.111",__FILE__,__LINE__);

	if(!groupA0.getVar("varA_2").isNull())                           throw NcException("NcException","Error in test 6.112",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::Current).isNull())          throw NcException("NcException","Error in test 6.113",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::Parents).isNull())          throw NcException("NcException","Error in test 6.114",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::Children).isNull())         throw NcException("NcException","Error in test 6.115",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::ParentsAndCurrent).isNull())throw NcException("NcException","Error in test 6.116",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.117",__FILE__,__LINE__);
	if(!groupA0.getVar("varA_2",NcGroup::All).isNull())              throw NcException("NcException","Error in test 6.118",__FILE__,__LINE__);

	if(groupA0.getVar("varA0_2") != varA0_2)                             throw NcException("NcException","Error in test 6.119",__FILE__,__LINE__);
	if(groupA0.getVar("varA0_2",NcGroup::Current) != varA0_2)            throw NcException("NcException","Error in test 6.120",__FILE__,__LINE__);
	if(!groupA0.getVar("varA0_2",NcGroup::Parents).isNull())             throw NcException("NcException","Error in test 6.121",__FILE__,__LINE__);
	if(!groupA0.getVar("varA0_2",NcGroup::Children).isNull())            throw NcException("NcException","Error in test 6.122",__FILE__,__LINE__);
	if(groupA0.getVar("varA0_2",NcGroup::ParentsAndCurrent) != varA0_2) throw NcException("NcException","Error in test 6.123",__FILE__,__LINE__);
	if(groupA0.getVar("varA0_2",NcGroup::ChildrenAndCurrent) != varA0_2)throw NcException("NcException","Error in test 6.124",__FILE__,__LINE__);
	if(groupA0.getVar("varA0_2",NcGroup::All) != varA0_2)                throw NcException("NcException","Error in test 6.125",__FILE__,__LINE__);

	if(!groupA0.getVar("varB_3").isNull())                             throw NcException("NcException","Error in test 6.126",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 6.127",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 6.128",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 6.129",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 6.130",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.131",__FILE__,__LINE__);
	if(!groupA0.getVar("varB_3",NcGroup::All).isNull())                throw NcException("NcException","Error in test 6.132",__FILE__,__LINE__);

	if(!groupA0.getVar("varC_5").isNull())                             throw NcException("NcException","Error in test 6.133",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 6.134",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 6.135",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 6.136",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 6.137",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 6.138",__FILE__,__LINE__);
	if(!groupA0.getVar("varC_5",NcGroup::All).isNull())                throw NcException("NcException","Error in test 6.139",__FILE__,__LINE__);

	cout <<"    -----------   passed\n";




	{
	  cout <<left<<setw(55)<<"Testing getVars(\"name\",[netCDF::Location])";
	  set<NcVar> groupSet;
	  set<NcVar>::iterator iter;

	  groupSet = ncFile.getVars("var_1");
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.1",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.2",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.3",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.4",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.5",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.6",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::Current);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.7",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.8",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.9",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.10",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.11",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.12",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.13",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.14",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.15",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.16",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.17",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.18",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.19",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.20",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.21",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.22",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.23",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.24",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.25",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.26",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.27",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.28",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.29",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.30",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.31",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.32",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.33",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.34",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.35",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.36",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.37",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.38",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.39",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.40",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.41",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.42",__FILE__,__LINE__);

	  groupSet = ncFile.getVars("var_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.43",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.44",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.45",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.46",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varA0_2",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.47",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.48",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varB_3",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.49",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.50",__FILE__,__LINE__);
	  groupSet = ncFile.getVars("varC_5",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.51",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.52",__FILE__,__LINE__);

	  //////////////////

	  groupSet = groupA.getVars("var_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.53",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1");
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.54",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.55",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.56",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.57",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.58",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.59",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::Current);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.60",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.61",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.62",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.63",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.64",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::Parents);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.65",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.66",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.67",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.68",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.69",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.70",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.71",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.72",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.73",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.74",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.75",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::Children);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.76",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.77",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.78",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.79",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.80",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.82",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.83",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.84",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.85",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.86",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.87",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.88",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.89",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.90",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.91",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.92",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.93",__FILE__,__LINE__);

	  groupSet = groupA.getVars("var_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.94",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.95",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.96",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.97",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varA0_2",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.98",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varB_3",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.99",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.100",__FILE__,__LINE__);
	  groupSet = groupA.getVars("varC_5",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.101",__FILE__,__LINE__);
	  iter=groupSet.find(varC_5);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 7.102",__FILE__,__LINE__);

	  /////////////

	  groupSet = groupA0.getVars("var_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.103",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.104",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2");
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.105",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.106",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.107",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.108",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.109",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.110",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::Current);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.111",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.112",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.113",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.114",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::Parents);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.115",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.116",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.117",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.118",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.119",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.120",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.121",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.122",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.123",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.124",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.125",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.126",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.127",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.128",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.129",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.130",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.131",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.132",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.133",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.134",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.135",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.136",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.137",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.138",__FILE__,__LINE__);

	  groupSet = groupA0.getVars("var_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.139",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.140",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA_1",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.141",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varA0_2",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.142",__FILE__,__LINE__);
	  iter=groupSet.find(varA0_2);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.143",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varB_3",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.144",__FILE__,__LINE__);
	  groupSet = groupA0.getVars("varC_5",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.145",__FILE__,__LINE__);

	  /////////////

	  /////////////

	  groupSet = groupB.getVars("var_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.146",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.147",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.148",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3");
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.149",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.150",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5");
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.151",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.152",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.153",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.154",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::Current);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.155",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.156",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::Current);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.157",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::Parents);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.158",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.159",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::Parents);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.160",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.161",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.162",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.163",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::Parents);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.164",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.165",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.166",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.167",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.168",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::Children);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.169",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.170",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.171",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.172",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.173",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.174",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.175",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.176",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::ParentsAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.177",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.178",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.179",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.180",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.181",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.182",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::ChildrenAndCurrent);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.183",__FILE__,__LINE__);

	  groupSet = groupB.getVars("var_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.184",__FILE__,__LINE__);
	  iter=groupSet.find(var_1);  if( iter == groupSet.end())    throw NcException("NcException","Error in test 7.185",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA_1",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.186",__FILE__,__LINE__);
	  iter=groupSet.find(varA_1);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.187",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varA0_2",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.188",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varB_3",NcGroup::All);
	  if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 7.189",__FILE__,__LINE__);
	  iter=groupSet.find(varB_3);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 7.190",__FILE__,__LINE__);
	  groupSet = groupB.getVars("varC_5",NcGroup::All);
	  if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 7.191",__FILE__,__LINE__);

	  cout <<"    -----------   passed\n";
	}


      }







    }
  catch (NcException& e)
    {
      cout << "unknown error"<<endl;
      e.what();
    }
}
