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
      cout<<"Opening file \"firstFile.cdf\" with NcFile::replace"<<endl;
      NcFile ncFile("firstFile.cdf",NcFile::replace);
    
      cout<<left<<setw(55)<<"Testing addGroup(\"groupName\")";
      NcGroup groupA(ncFile.addGroup("groupA"));
      NcGroup groupA0(ncFile.addGroup("groupA0"));
      NcGroup groupB(groupA.addGroup("groupB"));
      NcGroup groupC(groupA.addGroup("groupC"));
      cout <<"    -----------   passed\n";
    
      cout <<left<<setw(55)<<"Testing putAtt(\"attName\",\"typeName\",len,dataValues)";
      vector<string>  b1(10);
      vector<unsigned char *>  b2(10);
      vector<signed char*>  b3(10);
      vector<short>  a1(10);
      vector<int>  a2(10);
      vector<float>  a3(10);
      vector<double>  a4(10);
      vector<unsigned short>  a5(10);
      vector<unsigned int>  a6(10);
      vector<long long>  a7(10);
      vector<unsigned long long>  a8(10);
      vector<double>  a9(10);
      initializeVector(a1);
      initializeVector(a2);
      initializeVector(a3);
      initializeVector(a4);
      initializeVector(a5);
      initializeVector(a6);
      initializeVector(a7);
      initializeVector(a8);
      initializeVector(a9);

      NcGroupAtt attA1_1  = ncFile.putAtt("att1_1",ncByte,size_t(10),    &a1[0]);
      NcGroupAtt attA2_1  = ncFile.putAtt("att2_1",ncUbyte,size_t(10),   &a1[0]);
      NcGroupAtt attA3_1  = ncFile.putAtt("att3_1",ncShort,size_t(10),   &a1[0]);
      NcGroupAtt attA5_1  = ncFile.putAtt("att5_1",ncUshort,size_t(10),  &a1[0]);
      NcGroupAtt attA6_1  = ncFile.putAtt("att6_1",ncInt,size_t(10),     &a1[0]);
      NcGroupAtt attA7_1  = ncFile.putAtt("att7_1",ncUint,size_t(10),    &a1[0]);
      NcGroupAtt attA8_1  = ncFile.putAtt("att8_1",ncInt64,size_t(10),   &a1[0]);
      NcGroupAtt attA9_1  = ncFile.putAtt("att9_1",ncUint64,size_t(10),  &a1[0]);
      NcGroupAtt attA10_1 = ncFile.putAtt("att10_1",ncFloat,size_t(10),  &a1[0]);
      NcGroupAtt attA11_1 = ncFile.putAtt("att11_1",ncDouble,size_t(10), &a1[0]);

      NcGroupAtt attA1_2  = ncFile.putAtt("att1_2",ncByte,size_t(10),    &a2[0]);
      NcGroupAtt attA2_2  = ncFile.putAtt("att2_2",ncUbyte,size_t(10),   &a2[0]);
      NcGroupAtt attA3_2  = ncFile.putAtt("att3_2",ncShort,size_t(10),   &a2[0]);
      NcGroupAtt attA5_2  = ncFile.putAtt("att5_2",ncUshort,size_t(10),  &a2[0]);
      NcGroupAtt attA6_2  = ncFile.putAtt("att6_2",ncInt,size_t(10),     &a2[0]);
      NcGroupAtt attA7_2  = ncFile.putAtt("att7_2",ncUint,size_t(10),    &a2[0]);
      NcGroupAtt attA8_2  = ncFile.putAtt("att8_2",ncInt64,size_t(10),   &a2[0]);
      NcGroupAtt attA9_2  = ncFile.putAtt("att9_2",ncUint64,size_t(10),  &a2[0]);
      NcGroupAtt attA10_2 = ncFile.putAtt("att10_2",ncFloat,size_t(10),  &a2[0]);
      NcGroupAtt attA11_2 = ncFile.putAtt("att11_2",ncDouble,size_t(10), &a2[0]);

      NcGroupAtt attA1_3  = ncFile.putAtt("att1_3",ncByte,size_t(10),    &a3[0]);
      NcGroupAtt attA2_3  = ncFile.putAtt("att2_3",ncUbyte,size_t(10),   &a3[0]);
      NcGroupAtt attA3_3  = ncFile.putAtt("att3_3",ncShort,size_t(10),   &a3[0]);
      NcGroupAtt attA5_3  = ncFile.putAtt("att5_3",ncUshort,size_t(10),  &a3[0]);
      NcGroupAtt attA6_3  = ncFile.putAtt("att6_3",ncInt,size_t(10),     &a3[0]);
      NcGroupAtt attA7_3  = ncFile.putAtt("att7_3",ncUint,size_t(10),    &a3[0]);
      NcGroupAtt attA8_3  = ncFile.putAtt("att8_3",ncInt64,size_t(10),   &a3[0]);
      NcGroupAtt attA9_3  = ncFile.putAtt("att9_3",ncUint64,size_t(10),  &a3[0]);
      NcGroupAtt attA10_3 = ncFile.putAtt("att10_3",ncFloat,size_t(10),  &a3[0]);
      NcGroupAtt attA11_3 = ncFile.putAtt("att11_3",ncDouble,size_t(10), &a3[0]);

      NcGroupAtt attA1_4  = ncFile.putAtt("att1_4",ncByte,size_t(10),    &a4[0]);
      NcGroupAtt attA2_4  = ncFile.putAtt("att2_4",ncUbyte,size_t(10),   &a4[0]);
      NcGroupAtt attA3_4  = ncFile.putAtt("att3_4",ncShort,size_t(10),   &a4[0]);
      NcGroupAtt attA5_4  = ncFile.putAtt("att5_4",ncUshort,size_t(10),  &a4[0]);
      NcGroupAtt attA6_4  = ncFile.putAtt("att6_4",ncInt,size_t(10),     &a4[0]);
      NcGroupAtt attA7_4  = ncFile.putAtt("att7_4",ncUint,size_t(10),    &a4[0]);
      NcGroupAtt attA8_4  = ncFile.putAtt("att8_4",ncInt64,size_t(10),   &a4[0]);
      NcGroupAtt attA9_4  = ncFile.putAtt("att9_4",ncUint64,size_t(10),  &a4[0]);
      NcGroupAtt attA10_4 = ncFile.putAtt("att10_4",ncFloat,size_t(10),  &a4[0]);
      NcGroupAtt attA11_4 = ncFile.putAtt("att11_4",ncDouble,size_t(10), &a4[0]);

      NcGroupAtt attA1_5  = ncFile.putAtt("att1_5",ncByte,size_t(10),    &a5[0]);
      NcGroupAtt attA2_5  = ncFile.putAtt("att2_5",ncUbyte,size_t(10),   &a5[0]);
      NcGroupAtt attA3_5  = ncFile.putAtt("att3_5",ncShort,size_t(10),   &a5[0]);
      NcGroupAtt attA5_5  = ncFile.putAtt("att5_5",ncUshort,size_t(10),  &a5[0]);
      NcGroupAtt attA6_5  = ncFile.putAtt("att6_5",ncInt,size_t(10),     &a5[0]);
      NcGroupAtt attA7_5  = ncFile.putAtt("att7_5",ncUint,size_t(10),    &a5[0]);
      NcGroupAtt attA8_5  = ncFile.putAtt("att8_5",ncInt64,size_t(10),   &a5[0]);
      NcGroupAtt attA9_5  = ncFile.putAtt("att9_5",ncUint64,size_t(10),  &a5[0]);
      NcGroupAtt attA10_5 = ncFile.putAtt("att10_5",ncFloat,size_t(10),  &a5[0]);
      NcGroupAtt attA11_5 = ncFile.putAtt("att11_5",ncDouble,size_t(10), &a5[0]);

      NcGroupAtt attA1_6  = ncFile.putAtt("att1_6",ncByte,size_t(10),    &a6[0]);
      NcGroupAtt attA2_6  = ncFile.putAtt("att2_6",ncUbyte,size_t(10),   &a6[0]);
      NcGroupAtt attA3_6  = ncFile.putAtt("att3_6",ncShort,size_t(10),   &a6[0]);
      NcGroupAtt attA5_6  = ncFile.putAtt("att5_6",ncUshort,size_t(10),  &a6[0]);
      NcGroupAtt attA6_6  = ncFile.putAtt("att6_6",ncInt,size_t(10),     &a6[0]);
      NcGroupAtt attA7_6  = ncFile.putAtt("att7_6",ncUint,size_t(10),    &a6[0]);
      NcGroupAtt attA8_6  = ncFile.putAtt("att8_6",ncInt64,size_t(10),   &a6[0]);
      NcGroupAtt attA9_6  = ncFile.putAtt("att9_6",ncUint64,size_t(10),  &a6[0]);
      NcGroupAtt attA10_6 = ncFile.putAtt("att10_6",ncFloat,size_t(10),  &a6[0]);
      NcGroupAtt attA11_6 = ncFile.putAtt("att11_6",ncDouble,size_t(10), &a6[0]);

      NcGroupAtt attA1_7  = ncFile.putAtt("att1_7",ncByte,size_t(10),    &a7[0]);
      NcGroupAtt attA2_7  = ncFile.putAtt("att2_7",ncUbyte,size_t(10),   &a7[0]);
      NcGroupAtt attA3_7  = ncFile.putAtt("att3_7",ncShort,size_t(10),   &a7[0]);
      NcGroupAtt attA5_7  = ncFile.putAtt("att5_7",ncUshort,size_t(10),  &a7[0]);
      NcGroupAtt attA6_7  = ncFile.putAtt("att6_7",ncInt,size_t(10),     &a7[0]);
      NcGroupAtt attA7_7  = ncFile.putAtt("att7_7",ncUint,size_t(10),    &a7[0]);
      NcGroupAtt attA8_7  = ncFile.putAtt("att8_7",ncInt64,size_t(10),   &a7[0]);
      NcGroupAtt attA9_7  = ncFile.putAtt("att9_7",ncUint64,size_t(10),  &a7[0]);
      NcGroupAtt attA10_7 = ncFile.putAtt("att10_7",ncFloat,size_t(10),  &a7[0]);
      NcGroupAtt attA11_7 = ncFile.putAtt("att11_7",ncDouble,size_t(10), &a7[0]);

      NcGroupAtt attA1_8  = ncFile.putAtt("att1_8",ncByte,size_t(10),    &a8[0]);
      NcGroupAtt attA2_8  = ncFile.putAtt("att2_8",ncUbyte,size_t(10),   &a8[0]);
      NcGroupAtt attA3_8  = ncFile.putAtt("att3_8",ncShort,size_t(10),   &a8[0]);
      NcGroupAtt attA5_8  = ncFile.putAtt("att5_8",ncUshort,size_t(10),  &a8[0]);
      NcGroupAtt attA6_8  = ncFile.putAtt("att6_8",ncInt,size_t(10),     &a8[0]);
      NcGroupAtt attA7_8  = ncFile.putAtt("att7_8",ncUint,size_t(10),    &a8[0]);
      NcGroupAtt attA8_8  = ncFile.putAtt("att8_8",ncInt64,size_t(10),   &a8[0]);
      NcGroupAtt attA9_8  = ncFile.putAtt("att9_8",ncUint64,size_t(10),  &a8[0]);
      NcGroupAtt attA10_8 = ncFile.putAtt("att10_8",ncFloat,size_t(10),  &a8[0]);
      NcGroupAtt attA11_8 = ncFile.putAtt("att11_8",ncDouble,size_t(10), &a8[0]);

      NcGroupAtt attA1_9  = ncFile.putAtt("att1_9",ncByte,size_t(10),    &a9[0]);
      NcGroupAtt attA2_9  = ncFile.putAtt("att2_9",ncUbyte,size_t(10),   &a9[0]);
      NcGroupAtt attA3_9  = ncFile.putAtt("att3_9",ncShort,size_t(10),   &a9[0]);
      NcGroupAtt attA5_9  = ncFile.putAtt("att5_9",ncUshort,size_t(10),  &a9[0]);
      NcGroupAtt attA6_9  = ncFile.putAtt("att6_9",ncInt,size_t(10),     &a9[0]);
      NcGroupAtt attA7_9  = ncFile.putAtt("att7_9",ncUint,size_t(10),    &a9[0]);
      NcGroupAtt attA8_9  = ncFile.putAtt("att8_9",ncInt64,size_t(10),   &a9[0]);
      NcGroupAtt attA9_9  = ncFile.putAtt("att9_9",ncUint64,size_t(10),  &a9[0]);
      NcGroupAtt attA10_9 = ncFile.putAtt("att10_9",ncFloat,size_t(10),  &a9[0]);
      NcGroupAtt attA11_9 = ncFile.putAtt("att11_9",ncDouble,size_t(10), &a9[0]);

      const string b("abc");
      NcGroupAtt att4  = ncFile.putAtt("att4",ncChar,size_t(2), b.c_str());
      const char* c(b.c_str());
      NcGroupAtt att12 = ncFile.putAtt("att12",ncString,size_t(1), &c);
      string stringName("A Text string Attribute");
      NcGroupAtt att12a = ncFile.putAtt("att12a",stringName);

      NcGroupAtt att_A1  = groupA0.putAtt("att_A1",ncByte,size_t(10),    &a9[0]);
      NcGroupAtt att_A2  = groupA0.putAtt("att_A2",ncUbyte,size_t(10),   &a9[0]);

      NcGroupAtt att_A01  = groupA.putAtt("att_A01",ncByte,size_t(10),    &a9[0]);
      NcGroupAtt att_A02  = groupA.putAtt("att_A02",ncUbyte,size_t(10),   &a9[0]);
      NcGroupAtt att_A03  = groupA.putAtt("att_A03",ncShort,size_t(10),   &a9[0]);

      NcGroupAtt att_B1  = groupB.putAtt("att_B1",ncByte,size_t(10),    &a9[0]);
      NcGroupAtt att_B2  = groupB.putAtt("att_B2",ncUbyte,size_t(10),   &a9[0]);
      NcGroupAtt att_B3  = groupB.putAtt("att_B3",ncShort,size_t(10),   &a9[0]);
      NcGroupAtt att_B4  = groupB.putAtt("att_B4",ncUshort,size_t(10),  &a9[0]);

      NcGroupAtt att_C1  = groupC.putAtt("att_C1",ncByte,size_t(10),    &a9[0]);
      NcGroupAtt att_C2  = groupC.putAtt("att_C2",ncUbyte,size_t(10),   &a9[0]);
      NcGroupAtt att_C3  = groupC.putAtt("att_C3",ncShort,size_t(10),   &a9[0]);
      NcGroupAtt att_C4  = groupC.putAtt("att_C4",ncUshort,size_t(10),  &a9[0]);
      NcGroupAtt att_C5  = groupC.putAtt("att_C5",ncInt,size_t(10),     &a9[0]);
      cout <<"    -----------   passed\n";


      cout <<left<<setw(55)<<"Testing getAttLength()";
      if(att_C1.getAttLength() != 10) throw NcException("NcException","Error in test 2.1",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";

      cout <<left<<setw(55)<<"Testing getName()";
      if(att_C1.getName() != "att_C1") throw NcException("NcException","Error in test 3.1",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";

      cout <<left<<setw(55)<<"Testing getType()";

      if(att_C1.getType() != ncByte) throw NcException("NcException","Error in test 4.1",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";

      cout <<left<<setw(55)<<"Testing getParentGroup()";
      if(att_C1.getParentGroup() != groupC) throw NcException("NcException","Error in test 5.1",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";

      cout <<left<<setw(55)<<"Testing getValues()";

      vector<double>  atest(10);
      att_C1.getValues(&atest[0]);
      if(atest != a9) throw NcException("NcException","Error in test 6.1",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";


      cout <<left<<setw(55)<<"Testing attCount([netCDF::Location])";
      if(ncFile.getAttCount() != 93)                           throw NcException("NcException","Error in test 7.1",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::Current) != 93)           throw NcException("NcException","Error in test 7.2",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::Parents) != 0)            throw NcException("NcException","Error in test 7.3",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::Children) != 14)          throw NcException("NcException","Error in test 7.4",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::ParentsAndCurrent) != 93) throw NcException("NcException","Error in test 7.5",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::ChildrenAndCurrent) !=107)throw NcException("NcException","Error in test 7.6",__FILE__,__LINE__);
      if(ncFile.getAttCount(NcGroup::All) != 107)              throw NcException("NcException","Error in test 7.7",__FILE__,__LINE__);
      if(groupB.getAttCount() != 4)                            throw NcException("NcException","Error in test 7.8",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::Current) != 4)            throw NcException("NcException","Error in test 7.9",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::Parents) != 96)           throw NcException("NcException","Error in test 7.10",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::Children) != 0)           throw NcException("NcException","Error in test 7.11",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::ParentsAndCurrent) != 100) throw NcException("NcException","Error in test 7.12",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::ChildrenAndCurrent) != 4) throw NcException("NcException","Error in test 7.13",__FILE__,__LINE__);
      if(groupB.getAttCount(NcGroup::All) != 100)               throw NcException("NcException","Error in test 7.14",__FILE__,__LINE__);
      if(groupA0.getAttCount() != 2)                           throw NcException("NcException","Error in test 7.15",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::Current) != 2)           throw NcException("NcException","Error in test 7.16",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::Parents) != 93)          throw NcException("NcException","Error in test 7.17",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::Children) != 0)          throw NcException("NcException","Error in test 7.18",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::ParentsAndCurrent) != 95)throw NcException("NcException","Error in test 7.19",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::ChildrenAndCurrent) != 2)throw NcException("NcException","Error in test 7.20",__FILE__,__LINE__);
      if(groupA0.getAttCount(NcGroup::All) != 95)              throw NcException("NcException","Error in test 7.21",__FILE__,__LINE__);

      if(groupA.getAttCount() != 3)                            throw NcException("NcException","Error in test 7.15",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::Current) != 3)            throw NcException("NcException","Error in test 7.16",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::Parents) != 93)           throw NcException("NcException","Error in test 7.17",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::Children) != 9)           throw NcException("NcException","Error in test 7.18",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::ParentsAndCurrent) != 96) throw NcException("NcException","Error in test 7.19",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::ChildrenAndCurrent) !=12) throw NcException("NcException","Error in test 7.20",__FILE__,__LINE__);
      if(groupA.getAttCount(NcGroup::All) != 105)              throw NcException("NcException","Error in test 7.21",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";


      {
	cout <<left<<setw(55)<<"Testing getAtts([netCDF::Location])";
	multimap<string,NcGroupAtt> groupMap;
	multimap<string,NcGroupAtt>::iterator iter;

	groupMap = ncFile.getAtts();
	if(groupMap.size() != 93)                                  throw NcException("NcException","Error in test 8.1",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.2",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.3",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.4",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.5",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.6",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::Current);
	if(groupMap.size() != 93)                                  throw NcException("NcException","Error in test 8.7",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.8",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.9",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.10",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.11",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.12",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::Parents);
	if(groupMap.size() != 0)                                   throw NcException("NcException","Error in test 8.13",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::Children);
	if(groupMap.size() != 14)                                  throw NcException("NcException","Error in test 8.14",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.15",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.16",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.17",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.18",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.19",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 93)                                  throw NcException("NcException","Error in test 8.20",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.21",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.22",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.23",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.24",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.25",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 107)                                 throw NcException("NcException","Error in test 8.26",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.27",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.28",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.29",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.30",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.31",__FILE__,__LINE__);

	groupMap = ncFile.getAtts(NcGroup::All);
	if(groupMap.size() != 107)                                 throw NcException("NcException","Error in test 8.32",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.33",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.34",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.35",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.36",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.37",__FILE__,__LINE__);


	//group A now
	groupMap = groupA.getAtts();
	if(groupMap.size() != 3)                                   throw NcException("NcException","Error in test 8.38",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.39",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.40",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.41",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.42",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.43",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::Current);
	if(groupMap.size() != 3)                                   throw NcException("NcException","Error in test 8.44",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.45",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.46",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.47",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.48",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.49",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::Parents);
	if(groupMap.size() != 93)                                  throw NcException("NcException","Error in test 8.50",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.51",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.52",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.53",__FILE__,__LINE__);
	iter=groupMap.find("att_b3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.54",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.55",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::Children);
	if(groupMap.size() != 9)                                   throw NcException("NcException","Error in test 8.56",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.57",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.58",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.59",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.60",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.61",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 96)                                  throw NcException("NcException","Error in test 8.62",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.63",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.64",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.65",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.66",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.67",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 12)                                  throw NcException("NcException","Error in test 8.68",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.69",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.70",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.71",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.72",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.73",__FILE__,__LINE__);

	groupMap = groupA.getAtts(NcGroup::All);
	if(groupMap.size() != 105)                                 throw NcException("NcException","Error in test 8.74",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.75",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.76",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.77",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.78",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.79",__FILE__,__LINE__);

	// group A0 now
	groupMap = groupA0.getAtts();
	if(groupMap.size() != 2)                                   throw NcException("NcException","Error in test 8.80",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.81",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.82",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.83",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.84",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.85",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::Current);
	if(groupMap.size() != 2)                                   throw NcException("NcException","Error in test 8.86",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.87",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.88",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.89",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.90",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.91",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::Parents);
	if(groupMap.size() != 93)                                  throw NcException("NcException","Error in test 8.92",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.93",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.94",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.95",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.96",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.97",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::Children);
	if(groupMap.size() != 0)                                   throw NcException("NcException","Error in test 8.98",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 95)                                  throw NcException("NcException","Error in test 8.99",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.100",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.101",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.102",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.103",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.104",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 2)                                   throw NcException("NcException","Error in test 8.105",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.106",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.107",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.108",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.109",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.110",__FILE__,__LINE__);

	groupMap = groupA0.getAtts(NcGroup::All);
	if(groupMap.size() != 95)                                  throw NcException("NcException","Error in test 8.111",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.112",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.113",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.114",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.115",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.116",__FILE__,__LINE__);


	// now for groupB    
	groupMap = groupB.getAtts();
	if(groupMap.size() != 4)                                   throw NcException("NcException","Error in test 8.117",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.118",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.119",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.120",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.121",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.122",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::Current);
	if(groupMap.size() != 4)                                   throw NcException("NcException","Error in test 8.123",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.124",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.125",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.126",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.127",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.128",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::Parents);
	if(groupMap.size() != 96)                                  throw NcException("NcException","Error in test 8.129",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.130",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.131",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.132",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.133",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.134",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::Children);
	if(groupMap.size() != 0)                                   throw NcException("NcException","Error in test 8.135",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::ParentsAndCurrent);
	if(groupMap.size() != 100)                                  throw NcException("NcException","Error in test 8.136",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.137",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.138",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.139",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.140",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.141",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::ChildrenAndCurrent);
	if(groupMap.size() != 4)                                   throw NcException("NcException","Error in test 8.142",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.143",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.144",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.145",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.146",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.147",__FILE__,__LINE__);

	groupMap = groupB.getAtts(NcGroup::All);
	if(groupMap.size() != 100)                                  throw NcException("NcException","Error in test 8.148",__FILE__,__LINE__);
	iter=groupMap.find("att1_7");  if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.149",__FILE__,__LINE__);
	iter=groupMap.find("att_A01"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.150",__FILE__,__LINE__);
	iter=groupMap.find("att_A1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.151",__FILE__,__LINE__);
	iter=groupMap.find("att_B3"); if( iter == groupMap.end()) throw NcException("NcException","Error in test 8.152",__FILE__,__LINE__);
	iter=groupMap.find("att_C1"); if( iter != groupMap.end()) throw NcException("NcException","Error in test 8.153",__FILE__,__LINE__);

	cout <<"    -----------   passed\n";
      }
    
      cout <<left<<setw(55)<<"Testing getAtt(\"name\",[netCDF::Location])";
      if(ncFile.getAtt("att1_7") != attA1_7)                            throw NcException("NcException","Error in test 9.1",__FILE__,__LINE__);
      if(ncFile.getAtt("att1_7",NcGroup::Current) != attA1_7)           throw NcException("NcException","Error in test 9.2",__FILE__,__LINE__);
      if(!ncFile.getAtt("att1_7",NcGroup::Parents).isNull())            throw NcException("NcException","Error in test 9.3",__FILE__,__LINE__);
      if(!ncFile.getAtt("att1_7",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 9.4",__FILE__,__LINE__);
      if(ncFile.getAtt("att1_7",NcGroup::ParentsAndCurrent) != attA1_7) throw NcException("NcException","Error in test 9.5",__FILE__,__LINE__);
      if(ncFile.getAtt("att1_7",NcGroup::ChildrenAndCurrent)!= attA1_7) throw NcException("NcException","Error in test 9.6",__FILE__,__LINE__);
      if(ncFile.getAtt("att1_7",NcGroup::All) != attA1_7)               throw NcException("NcException","Error in test 9.7",__FILE__,__LINE__);

      if(!ncFile.getAtt("att_A01").isNull())                            throw NcException("NcException","Error in test 9.8",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A01",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.9",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A01",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.10",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A01",NcGroup::Children) != att_A01)         throw NcException("NcException","Error in test 9.11",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A01",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.12",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A01",NcGroup::ChildrenAndCurrent) != att_A01)throw NcException("NcException","Error in test 9.13",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A01",NcGroup::All) != att_A01)              throw NcException("NcException","Error in test 9.14",__FILE__,__LINE__);

      if(!ncFile.getAtt("att_A1").isNull())                            throw NcException("NcException","Error in test 9.15",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.16",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.17",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A1",NcGroup::Children) != att_A1)          throw NcException("NcException","Error in test 9.18",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_A1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.19",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A1",NcGroup::ChildrenAndCurrent) != att_A1)throw NcException("NcException","Error in test 9.20",__FILE__,__LINE__);
      if(ncFile.getAtt("att_A1",NcGroup::All) != att_A1)               throw NcException("NcException","Error in test 9.21",__FILE__,__LINE__);

      if(!ncFile.getAtt("att_B3").isNull())                            throw NcException("NcException","Error in test 9.22",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_B3",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.23",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_B3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.24",__FILE__,__LINE__);
      if(ncFile.getAtt("att_B3",NcGroup::Children) != att_B3)          throw NcException("NcException","Error in test 9.25",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_B3",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.26",__FILE__,__LINE__);
      if(ncFile.getAtt("att_B3",NcGroup::ChildrenAndCurrent) != att_B3)throw NcException("NcException","Error in test 9.27",__FILE__,__LINE__);
      if(ncFile.getAtt("att_B3",NcGroup::All) != att_B3)               throw NcException("NcException","Error in test 9.28",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_C1").isNull())                            throw NcException("NcException","Error in test 9.29",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_C1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.30",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_C1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.31",__FILE__,__LINE__);
      if(ncFile.getAtt("att_C1",NcGroup::Children) != att_C1)          throw NcException("NcException","Error in test 9.32",__FILE__,__LINE__);
      if(!ncFile.getAtt("att_C1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.33",__FILE__,__LINE__);
      if(ncFile.getAtt("att_C1",NcGroup::ChildrenAndCurrent) != att_C1)throw NcException("NcException","Error in test 9.34",__FILE__,__LINE__);
      if(ncFile.getAtt("att_C1",NcGroup::All) != att_C1)               throw NcException("NcException","Error in test 9.35",__FILE__,__LINE__);

      // now for next group
    
      if(!groupA0.getAtt("att1_7").isNull())                             throw NcException("NcException","Error in test 9.36",__FILE__,__LINE__);
      if(!groupA0.getAtt("att1_7",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 9.37",__FILE__,__LINE__);
      if(groupA0.getAtt("att1_7",NcGroup::Parents)  != attA1_7)          throw NcException("NcException","Error in test 9.38",__FILE__,__LINE__);
      if(!groupA0.getAtt("att1_7",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 9.39",__FILE__,__LINE__);
      if(groupA0.getAtt("att1_7",NcGroup::ParentsAndCurrent) != attA1_7) throw NcException("NcException","Error in test 9.40",__FILE__,__LINE__);
      if(!groupA0.getAtt("att1_7",NcGroup::ChildrenAndCurrent).isNull()) throw NcException("NcException","Error in test 9.41",__FILE__,__LINE__);
      if(groupA0.getAtt("att1_7",NcGroup::All) != attA1_7)               throw NcException("NcException","Error in test 9.42",__FILE__,__LINE__);
    
      if(!groupA0.getAtt("att_A01").isNull())                           throw NcException("NcException","Error in test 9.43",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::Current).isNull())          throw NcException("NcException","Error in test 9.44",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.45",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.46",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.47",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.48",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A01",NcGroup::All).isNull())              throw NcException("NcException","Error in test 9.49",__FILE__,__LINE__);
    
      if(groupA0.getAtt("att_A1") != att_A1)                            throw NcException("NcException","Error in test 9.50",__FILE__,__LINE__);
      if(groupA0.getAtt("att_A1",NcGroup::Current) != att_A1)           throw NcException("NcException","Error in test 9.51",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.52",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_A1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.53",__FILE__,__LINE__);
      if(groupA0.getAtt("att_A1",NcGroup::ParentsAndCurrent) != att_A1)throw NcException("NcException","Error in test 9.54",__FILE__,__LINE__);
      if(groupA0.getAtt("att_A1",NcGroup::ChildrenAndCurrent)!= att_A1) throw NcException("NcException","Error in test 9.55",__FILE__,__LINE__);
      if(groupA0.getAtt("att_A1",NcGroup::All) != att_A1)               throw NcException("NcException","Error in test 9.56",__FILE__,__LINE__);
    
      if(!groupA0.getAtt("att_B3").isNull())                            throw NcException("NcException","Error in test 9.57",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.58",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.59",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.60",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.61",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.62",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_B3",NcGroup::All).isNull())               throw NcException("NcException","Error in test 9.63",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1").isNull())                            throw NcException("NcException","Error in test 9.64",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.65",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.66",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.67",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.68",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.69",__FILE__,__LINE__);
      if(!groupA0.getAtt("att_C1",NcGroup::All).isNull())               throw NcException("NcException","Error in test 9.70",__FILE__,__LINE__);

      // now for next group

      if(!groupA.getAtt("att1_7").isNull())                             throw NcException("NcException","Error in test 9.71",__FILE__,__LINE__);
      if(!groupA.getAtt("att1_7",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 9.72",__FILE__,__LINE__);
      if(groupA.getAtt("att1_7",NcGroup::Parents) != attA1_7)           throw NcException("NcException","Error in test 9.73",__FILE__,__LINE__);
      if(!groupA.getAtt("att1_7",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 9.74",__FILE__,__LINE__);
      if(groupA.getAtt("att1_7",NcGroup::ParentsAndCurrent) != attA1_7) throw NcException("NcException","Error in test 9.75",__FILE__,__LINE__);
      if(!groupA.getAtt("att1_7",NcGroup::ChildrenAndCurrent).isNull()) throw NcException("NcException","Error in test 9.76",__FILE__,__LINE__);
      if(groupA.getAtt("att1_7",NcGroup::All) != attA1_7)               throw NcException("NcException","Error in test 9.77",__FILE__,__LINE__);

      if(groupA.getAtt("att_A01") != att_A01)                           throw NcException("NcException","Error in test 9.78",__FILE__,__LINE__);
      if(groupA.getAtt("att_A01",NcGroup::Current) != att_A01)          throw NcException("NcException","Error in test 9.79",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A01",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.80",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A01",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.81",__FILE__,__LINE__);
      if(groupA.getAtt("att_A01",NcGroup::ParentsAndCurrent) != att_A01)throw NcException("NcException","Error in test 9.82",__FILE__,__LINE__);
      if(groupA.getAtt("att_A01",NcGroup::ChildrenAndCurrent) != att_A01)throw NcException("NcException","Error in test 9.83",__FILE__,__LINE__);
      if(groupA.getAtt("att_A01",NcGroup::All) != att_A01)               throw NcException("NcException","Error in test 9.84",__FILE__,__LINE__);

      if(!groupA.getAtt("att_A1").isNull())                            throw NcException("NcException","Error in test 9.85",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.86",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.87",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.88",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.89",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.90",__FILE__,__LINE__);
      if(!groupA.getAtt("att_A1",NcGroup::All).isNull())               throw NcException("NcException","Error in test 9.91",__FILE__,__LINE__);

      if(!groupA.getAtt("att_B3").isNull())                            throw NcException("NcException","Error in test 9.92",__FILE__,__LINE__);
      if(!groupA.getAtt("att_B3",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.93",__FILE__,__LINE__);
      if(!groupA.getAtt("att_B3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.94",__FILE__,__LINE__);
      if(groupA.getAtt("att_B3",NcGroup::Children) != att_B3)          throw NcException("NcException","Error in test 9.95",__FILE__,__LINE__);
      if(!groupA.getAtt("att_B3",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.96",__FILE__,__LINE__);
      if(groupA.getAtt("att_B3",NcGroup::ChildrenAndCurrent) != att_B3)throw NcException("NcException","Error in test 9.97",__FILE__,__LINE__);
      if(groupA.getAtt("att_B3",NcGroup::All) != att_B3)               throw NcException("NcException","Error in test 9.98",__FILE__,__LINE__);

      if(!groupA.getAtt("att_C1").isNull())                            throw NcException("NcException","Error in test 9.99",__FILE__,__LINE__);
      if(!groupA.getAtt("att_C1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.100",__FILE__,__LINE__);
      if(!groupA.getAtt("att_C1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.101",__FILE__,__LINE__);
      if(groupA.getAtt("att_C1",NcGroup::Children) != att_C1)          throw NcException("NcException","Error in test 9.102",__FILE__,__LINE__);
      if(!groupA.getAtt("att_C1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.103",__FILE__,__LINE__);
      if(groupA.getAtt("att_C1",NcGroup::ChildrenAndCurrent) != att_C1)throw NcException("NcException","Error in test 9.104",__FILE__,__LINE__);
      if(groupA.getAtt("att_C1",NcGroup::All) != att_C1)               throw NcException("NcException","Error in test 9.105",__FILE__,__LINE__);

      // now for next group

      if(!groupB.getAtt("att1_7").isNull())                             throw NcException("NcException","Error in test 9.106",__FILE__,__LINE__);
      if(!groupB.getAtt("att1_7",NcGroup::Current).isNull())            throw NcException("NcException","Error in test 9.107",__FILE__,__LINE__);
      if(groupB.getAtt("att1_7",NcGroup::Parents) != attA1_7)           throw NcException("NcException","Error in test 9.108",__FILE__,__LINE__);
      if(!groupB.getAtt("att1_7",NcGroup::Children).isNull())           throw NcException("NcException","Error in test 9.109",__FILE__,__LINE__);
      if(groupB.getAtt("att1_7",NcGroup::ParentsAndCurrent) != attA1_7) throw NcException("NcException","Error in test 9.110",__FILE__,__LINE__);
      if(!groupB.getAtt("att1_7",NcGroup::ChildrenAndCurrent).isNull()) throw NcException("NcException","Error in test 9.111",__FILE__,__LINE__);
      if(groupB.getAtt("att1_7",NcGroup::All) != attA1_7)               throw NcException("NcException","Error in test 9.112",__FILE__,__LINE__);

      if(!groupB.getAtt("att_A01").isNull())                            throw NcException("NcException","Error in test 9.113",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A01",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.114",__FILE__,__LINE__);
      if(groupB.getAtt("att_A01",NcGroup::Parents) != att_A01)          throw NcException("NcException","Error in test 9.115",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A01",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.116",__FILE__,__LINE__);
      if(groupB.getAtt("att_A01",NcGroup::ParentsAndCurrent) != att_A01) throw NcException("NcException","Error in test 9.117",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A01",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.118",__FILE__,__LINE__);
      if(groupB.getAtt("att_A01",NcGroup::All) != att_A01)              throw NcException("NcException","Error in test 9.119",__FILE__,__LINE__);

      if(!groupB.getAtt("att_A1").isNull())                            throw NcException("NcException","Error in test 9.120",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.121",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.122",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.123",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.124",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.125",__FILE__,__LINE__);
      if(!groupB.getAtt("att_A1",NcGroup::All).isNull())               throw NcException("NcException","Error in test 9.126",__FILE__,__LINE__);
    
      if(groupB.getAtt("att_B3") != att_B3)                            throw NcException("NcException","Error in test 9.127",__FILE__,__LINE__);
      if(groupB.getAtt("att_B3",NcGroup::Current) != att_B3)           throw NcException("NcException","Error in test 9.128",__FILE__,__LINE__);
      if(!groupB.getAtt("att_B3",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.129",__FILE__,__LINE__);
      if(!groupB.getAtt("att_B3",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.130",__FILE__,__LINE__);
      if(groupB.getAtt("att_B3",NcGroup::ParentsAndCurrent) != att_B3) throw NcException("NcException","Error in test 9.131",__FILE__,__LINE__);
      if(groupB.getAtt("att_B3",NcGroup::ChildrenAndCurrent) != att_B3)throw NcException("NcException","Error in test 9.132",__FILE__,__LINE__);
      if(groupB.getAtt("att_B3",NcGroup::All) != att_B3)               throw NcException("NcException","Error in test 9.133",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1").isNull())                            throw NcException("NcException","Error in test 9.134",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::Current).isNull())           throw NcException("NcException","Error in test 9.135",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::Parents).isNull())           throw NcException("NcException","Error in test 9.136",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::Children).isNull())          throw NcException("NcException","Error in test 9.137",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::ParentsAndCurrent).isNull()) throw NcException("NcException","Error in test 9.138",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::ChildrenAndCurrent).isNull())throw NcException("NcException","Error in test 9.139",__FILE__,__LINE__);
      if(!groupB.getAtt("att_C1",NcGroup::All).isNull())               throw NcException("NcException","Error in test 9.140",__FILE__,__LINE__);
      cout <<"    -----------   passed\n";


      {
	cout <<left<<setw(55)<<"Testing getAtts(\"name\",[netCDF::Location])";
	set<NcGroupAtt> groupSet;
	set<NcGroupAtt>::iterator iter;

	groupSet = ncFile.getAtts("att1_7");
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.1",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.2",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.3",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.4",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.5",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.6",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::Current);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.7",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.8",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.9",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.10",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.11",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.12",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.13",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.14",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.15",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.16",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.17",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::Children);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.18",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.19",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 10.20",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.21",__FILE__,__LINE__);
	iter=groupSet.find(att_A1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.20",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.22",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.20",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.23",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.20",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.24",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.25",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.26",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.27",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.28",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.29",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.30",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.32",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.33",__FILE__,__LINE__);
	iter=groupSet.find(att_A1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.34",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.35",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);

	groupSet = ncFile.getAtts("att1_7",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.36",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.37",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A01",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.38",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end())  throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_A1",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.39",__FILE__,__LINE__);
	iter=groupSet.find(att_A1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_B3",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.40",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);
	groupSet = ncFile.getAtts("att_C1",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.41",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end())   throw NcException("NcException","Error in test 10.31",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.42",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01");
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.43",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.44",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.45",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.46",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1");
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.47",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.48",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::Current);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.49",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.50",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.51",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.52",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::Current);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.53",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::Parents);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.54",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.55",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.56",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.57",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.58",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::Parents);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.59",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::Children);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.60",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::Children);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.61",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::Children);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.62",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.63",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.64",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::Children);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.65",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.66",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.67",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.68",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.69",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.70",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.71",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.72",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::ParentsAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.73",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.74",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.75",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.76",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.77",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.78",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.79",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::ChildrenAndCurrent);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.80",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.81",__FILE__,__LINE__);

	groupSet = groupA.getAtts("att1_7",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.82",__FILE__,__LINE__);
	iter=groupSet.find(attA1_7);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.83",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A01",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.84",__FILE__,__LINE__);
	iter=groupSet.find(att_A01);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.85",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_A1",NcGroup::All);
	if(groupSet.size() != 0)                                   throw NcException("NcException","Error in test 10.86",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_B3",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.87",__FILE__,__LINE__);
	iter=groupSet.find(att_B3);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.88",__FILE__,__LINE__);
	groupSet = groupA.getAtts("att_C1",NcGroup::All);
	if(groupSet.size() != 1)                                   throw NcException("NcException","Error in test 10.89",__FILE__,__LINE__);
	iter=groupSet.find(att_C1);  if( iter == groupSet.end()) throw NcException("NcException","Error in test 10.90",__FILE__,__LINE__);

	cout <<"    -----------   passed\n";

	return 0;
      }
    







    }
  catch (NcException& e)
    {
      cout << "unknown error"<<endl;
      e.what();
    }
}
