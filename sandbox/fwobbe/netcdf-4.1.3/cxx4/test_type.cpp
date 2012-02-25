// Purpose: Converts ida3 format xma data to netcdf4
// Usage:   xma2netcdf <shot number>


#include <iostream>
#include <iomanip>
#include <string>
#include <cstddef>
#include <netcdf>

#include <stdio.h>
#include <stddef.h>
#include "test_utilities.h"
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

struct struct3{
  int mem1;
  double mem2;
  short mem3[3];
};

int main()
{
try
  {
    cout<<"Opening file \"firstFile.cdf\" with NcFile::replace"<<endl;
    NcFile ncFile("firstFile.cdf",NcFile::replace);
    
    cout<<left<<std::setw(57)<<"Testing addGroup(\"groupName\")";
    NcGroup groupA(ncFile.addGroup("groupA"));
    NcGroup groupA0(ncFile.addGroup("groupA0"));
    NcGroup groupB(groupA.addGroup("groupB"));
    NcGroup groupC(groupA.addGroup("groupC"));
    cout <<"    -----------   passed\n";
    
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


    cout <<left<<setw(57)<<"Testing getName()";
    if( ncByte.getName()   !=  "byte")   throw NcException("NcException","Error in test 1.1",__FILE__,__LINE__);
    if( ncUbyte.getName()  !=  "ubyte")  throw NcException("NcException","Error in test 1.2",__FILE__,__LINE__);
    if( ncChar.getName()   !=  "char")   throw NcException("NcException","Error in test 1.3",__FILE__,__LINE__);
    if( ncShort.getName()  !=  "short")  throw NcException("NcException","Error in test 1.4",__FILE__,__LINE__);
    if( ncUshort.getName() !=  "ushort") throw NcException("NcException","Error in test 1.5",__FILE__,__LINE__);
    if( ncInt.getName()    !=  "int")    throw NcException("NcException","Error in test 1.6",__FILE__,__LINE__);
    if( ncUint.getName()   !=  "uint")   throw NcException("NcException","Error in test 1.7",__FILE__,__LINE__);
    if( ncInt64.getName()  !=  "int64")  throw NcException("NcException","Error in test 1.8",__FILE__,__LINE__);
    if( ncUint64.getName() !=  "uint64") throw NcException("NcException","Error in test 1.9",__FILE__,__LINE__);
    if( ncFloat.getName()  !=  "float")  throw NcException("NcException","Error in test 1.10",__FILE__,__LINE__);
    if( ncDouble.getName() !=  "double") throw NcException("NcException","Error in test 1.11",__FILE__,__LINE__);
    if( ncString.getName() !=  "string") throw NcException("NcException","Error in test 1.12",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing getParentGroup() and isNull()";
    if( !ncByte.getParentGroup().isNull())       throw NcException("NcException","Error in test 2.1",__FILE__,__LINE__);
    if( !ncUbyte.getParentGroup().isNull())      throw NcException("NcException","Error in test 2.2",__FILE__,__LINE__);
    if( !ncChar.getParentGroup().isNull())       throw NcException("NcException","Error in test 2.3",__FILE__,__LINE__);
    if( !ncShort.getParentGroup().isNull())      throw NcException("NcException","Error in test 2.4",__FILE__,__LINE__);
    if( !ncUshort.getParentGroup().isNull())     throw NcException("NcException","Error in test 2.5",__FILE__,__LINE__);
    if( !ncInt.getParentGroup().isNull())        throw NcException("NcException","Error in test 2.6",__FILE__,__LINE__);
    if( !ncUint.getParentGroup().isNull())       throw NcException("NcException","Error in test 2.7",__FILE__,__LINE__);
    if( !ncInt64.getParentGroup().isNull())      throw NcException("NcException","Error in test 2.8",__FILE__,__LINE__);
    if( !ncUint64.getParentGroup().isNull())     throw NcException("NcException","Error in test 2.9",__FILE__,__LINE__);
    if( !ncFloat.getParentGroup().isNull())      throw NcException("NcException","Error in test 2.10",__FILE__,__LINE__);
    if( !ncDouble.getParentGroup().isNull())     throw NcException("NcException","Error in test 2.11",__FILE__,__LINE__);
    if( !ncString.getParentGroup().isNull())     throw NcException("NcException","Error in test 2.12",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";


    cout <<left<<setw(57)<<"Testing getSize()";
    if( ncByte.getSize() !=  1)                  throw NcException("NcException","Error in test 3.1",__FILE__,__LINE__);
    if( ncUbyte.getSize() !=  1)                 throw NcException("NcException","Error in test 3.2",__FILE__,__LINE__);
    if( ncChar.getSize() !=  1)                  throw NcException("NcException","Error in test 3.3",__FILE__,__LINE__);
    if( ncShort.getSize() !=  2)                 throw NcException("NcException","Error in test 3.4",__FILE__,__LINE__);
    if( ncUshort.getSize() !=  2)                throw NcException("NcException","Error in test 3.5",__FILE__,__LINE__);
    if( ncInt.getSize() !=  4)                   throw NcException("NcException","Error in test 3.6",__FILE__,__LINE__);
    if( ncUint.getSize() !=  4)                  throw NcException("NcException","Error in test 3.7",__FILE__,__LINE__);
    if( ncInt64.getSize() !=  8)                 throw NcException("NcException","Error in test 3.8",__FILE__,__LINE__);
    if( ncUint64.getSize() !=  8)                throw NcException("NcException","Error in test 3.9",__FILE__,__LINE__);
    if( ncFloat.getSize() !=  4)                 throw NcException("NcException","Error in test 3.10",__FILE__,__LINE__);
    if( ncDouble.getSize() !=  8)                throw NcException("NcException","Error in test 3.11",__FILE__,__LINE__);
    if( ncString.getSize() !=  sizeof(char *))   throw NcException("NcException","Error in test 3.12",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing getTypeClass()";
    if( ncByte.getTypeClass() !=  NcType::nc_BYTE)     throw NcException("NcException","Error in test 4.1",__FILE__,__LINE__);
    if( ncUbyte.getTypeClass() !=  NcType::nc_UBYTE)   throw NcException("NcException","Error in test 4.2",__FILE__,__LINE__);
    if( ncChar.getTypeClass() !=  NcType::nc_CHAR)     throw NcException("NcException","Error in test 4.3",__FILE__,__LINE__);
    if( ncShort.getTypeClass() !=  NcType::nc_SHORT)   throw NcException("NcException","Error in test 4.4",__FILE__,__LINE__);
    if( ncUshort.getTypeClass() !=  NcType::nc_USHORT) throw NcException("NcException","Error in test 4.5",__FILE__,__LINE__);
    if( ncInt.getTypeClass() !=  NcType::nc_INT)       throw NcException("NcException","Error in test 4.6",__FILE__,__LINE__);
    if( ncUint.getTypeClass() !=  NcType::nc_UINT)     throw NcException("NcException","Error in test 4.7",__FILE__,__LINE__);
    if( ncInt64.getTypeClass() !=  NcType::nc_INT64)   throw NcException("NcException","Error in test 4.8",__FILE__,__LINE__);
    if( ncUint64.getTypeClass() !=  NcType::nc_UINT64) throw NcException("NcException","Error in test 4.9",__FILE__,__LINE__);
    if( ncFloat.getTypeClass() !=  NcType::nc_FLOAT)   throw NcException("NcException","Error in test 4.10",__FILE__,__LINE__);
    if( ncDouble.getTypeClass() !=  NcType::nc_DOUBLE) throw NcException("NcException","Error in test 4.11",__FILE__,__LINE__);
    if( ncString.getTypeClass() !=  NcType::nc_STRING) throw NcException("NcException","Error in test 4.12",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing getTypeClassName()";
    if( ncByte.getTypeClassName() !=  "nc_BYTE")     throw NcException("NcException","Error in test 5.1",__FILE__,__LINE__);
    if( ncUbyte.getTypeClassName() !=  "nc_UBYTE")   throw NcException("NcException","Error in test 5.2",__FILE__,__LINE__);
    if( ncChar.getTypeClassName() !=  "nc_CHAR")     throw NcException("NcException","Error in test 5.3",__FILE__,__LINE__);
    if( ncShort.getTypeClassName() !=  "nc_SHORT")   throw NcException("NcException","Error in test 5.4",__FILE__,__LINE__);
    if( ncUshort.getTypeClassName() !=  "nc_USHORT") throw NcException("NcException","Error in test 5.5",__FILE__,__LINE__);
    if( ncInt.getTypeClassName() !=  "nc_INT")       throw NcException("NcException","Error in test 5.6",__FILE__,__LINE__);
    if( ncUint.getTypeClassName() !=  "nc_UINT")     throw NcException("NcException","Error in test 5.7",__FILE__,__LINE__);
    if( ncInt64.getTypeClassName() !=  "nc_INT64")   throw NcException("NcException","Error in test 5.8",__FILE__,__LINE__);
    if( ncUint64.getTypeClassName() !=  "nc_UINT64") throw NcException("NcException","Error in test 5.9",__FILE__,__LINE__);
    if( ncFloat.getTypeClassName() !=  "nc_FLOAT")   throw NcException("NcException","Error in test 5.10",__FILE__,__LINE__);
    if( ncDouble.getTypeClassName() !=  "nc_DOUBLE") throw NcException("NcException","Error in test 5.11",__FILE__,__LINE__);
    if( ncString.getTypeClassName() !=  "nc_STRING") throw NcException("NcException","Error in test 5.12",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    

    cout <<left<<setw(57)<<"Testing creating new Compound Type";
    struct struct1{
      char mem1;
      char mem2;
      short mem3;
      double mem4;
    };
    
    struct struct2{
      char mem1;
      double mem2;
      short mem3[18];
    };
    vector<int> vecSize(2); vecSize[0]=6,vecSize[1]=3;

    NcCompoundType compoundType1(ncFile.addCompoundType("compoundType_1",sizeof(struct1)));
    NcCompoundType compoundType2(ncFile.addCompoundType("compoundType_2",sizeof(struct2)));

    cout <<"    -----------   passed\n";


    cout <<left<<setw(57)<<"Testing NcCompoundType::addMember()";

    compoundType1.addMember("member1",ncByte,offsetof(struct1,mem1));
    compoundType1.addMember("member2",ncByte,offsetof(struct1,mem2));
    compoundType1.addMember("member3",ncShort,offsetof(struct1,mem3));
    compoundType1.addMember("member4",ncDouble,offsetof(struct1,mem4));


    compoundType2.addMember("memberA",ncChar,offsetof(struct2,mem1));
    compoundType2.addMember("memberB",ncDouble,offsetof(struct2,mem2));
    compoundType2.addMember("memberC",ncShort,offsetof(struct2,mem3),vecSize);

    cout <<"    -----------   passed\n";


    cout <<left<<setw(57)<<"Testing NcCompoundType==>NcType && NcType::getTypeClass()";
    NcType dummyType(compoundType2);
    if(compoundType2.getTypeClass() != NcType::nc_COMPOUND) throw NcException("NcException","Error in test 8.1",__FILE__,__LINE__);
    if(dummyType.getTypeClass() != NcType::nc_COMPOUND)     throw NcException("NcException","Error in test 8.2",__FILE__,__LINE__);
    if(compoundType2.getTypeClass() != NcCompoundType::nc_COMPOUND) throw NcException("NcException","Error in test 8.3",__FILE__,__LINE__);
    if(dummyType.getTypeClass() != NcCompoundType::nc_COMPOUND)     throw NcException("NcException","Error in test 8.4",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    
    cout <<left<<setw(57)<<"Testing compoundClass.getMemberCount()";
    if(compoundType2.getMemberCount() != 3)throw NcException("NcException","Error in test 9.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcCompoundType::getMemberDimCount()";
    if(compoundType2.getMemberDimCount(0) != 0)throw NcException("NcException","Error in test 10.1",__FILE__,__LINE__);
    if(compoundType2.getMemberDimCount(1) != 0)throw NcException("NcException","Error in test 10.2",__FILE__,__LINE__);
    if(compoundType2.getMemberDimCount(2) != 2)throw NcException("NcException","Error in test 10.3",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    

    cout <<left<<setw(57)<<"Testing NcCompoundType::getMemberShape(index)";
    vector<int> memberShape;
    memberShape =compoundType2.getMemberShape(0);
    if(memberShape.size() != 0)throw NcException("NcException","Error in test 11.1",__FILE__,__LINE__);

    memberShape =compoundType2.getMemberShape(1);
    if(memberShape.size() != 0)throw NcException("NcException","Error in test 11.2",__FILE__,__LINE__);

    memberShape =compoundType2.getMemberShape(2);
    if(memberShape.size() != 2)throw NcException("NcException","Error in test 11.3",__FILE__,__LINE__);
    if(memberShape[0] != 6)    throw NcException("NcException","Error in test 11.4",__FILE__,__LINE__);
    if(memberShape[1] != 3)    throw NcException("NcException","Error in test 11.5",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    

    cout <<left<<setw(57)<<"Testing NcCompoundType::getMember(index).getName()";
    if(compoundType2.getMember(0).getName() !=  "char")   throw NcException("NcException","Error in test 12.1",__FILE__,__LINE__);
    if(compoundType2.getMember(1).getName() !=  "double") throw NcException("NcException","Error in test 12.2",__FILE__,__LINE__);
    if(compoundType2.getMember(2).getName() !=  "short")  throw NcException("NcException","Error in test 12.3",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    

    cout <<left<<setw(57)<<"Testing NcCompoundType == NcType";
    NcType dummyType2(compoundType2);
    if(!(dummyType2 == compoundType2)) throw NcException("NcException","Error in test 13.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcCompoundType == NcCompoundType";
    NcCompoundType dummy3(compoundType2);
    if(!(dummy3 == compoundType2)) throw NcException("NcException","Error in test 14.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    

    ////////////////////////////


    cout <<left<<setw(57)<<"Testing creating new Vlen Type";
    NcVlenType vlenType1(ncFile.addVlenType("vlenType_1",ncShort));
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcVlenType::getTypeClass()";
    if(vlenType1.getTypeClass() != NcType::nc_VLEN) throw NcException("NcException","Error in test 15.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    
    cout <<left<<setw(57)<<"Testing NcVlenType::getTypeClassName()";
    if(vlenType1.getTypeClassName() != "nc_VLEN") throw NcException("NcException","Error in test 16.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    
    cout <<left<<setw(57)<<"Testing NcVlenType::getName()";
    if(vlenType1.getName() != "vlenType_1") throw NcException("NcException","Error in test 17.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    
    cout <<left<<setw(57)<<"Testing NcVlenType::getBaseType(); == and !=";
    if(!(vlenType1.getBaseType() == ncShort)) throw NcException("NcException","Error in test 18.1",__FILE__,__LINE__);
    if(vlenType1.getBaseType() != ncShort) throw NcException("NcException","Error in test 18.2",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";


    cout <<left<<setw(57)<<"Testing NcVlenType constructors";
    NcType typevlen(vlenType1);
    NcVlenType vlenvlenType(typevlen);
    if(vlenType1 != vlenvlenType) throw NcException("NcException","Error in test 19.1",__FILE__,__LINE__);
    if(vlenType1 != typevlen) throw NcException("NcException","Error in test 19.2",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";
    



    ////////////////////////////


    cout <<left<<setw(57)<<"Testing creating new Enum Type";
    NcEnumType enumType1(ncFile.addEnumType("enumType_1",NcEnumType::nc_SHORT));
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::addMember()";
    enumType1.addMember("Monday",1);
    enumType1.addMember("Tuesday",7);
    enumType1.addMember("Wednesday",-20);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::getBaseType() == and !=";
    if(enumType1.getBaseType() != ncShort) throw NcException("NcException","Error in test 20.1",__FILE__,__LINE__);
    if(!(enumType1.getBaseType() == ncShort)) throw NcException("NcException","Error in test 20.2",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::getMemberCount()";
    if(enumType1.getMemberCount() != 3) throw NcException("NcException","Error in test 21.1",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::getMemberNameFromIndex(index)";
    if(enumType1.getMemberNameFromIndex(0) != "Monday") throw NcException("NcException","Error in test 22.1",__FILE__,__LINE__);
    if(enumType1.getMemberNameFromIndex(1) != "Tuesday") throw NcException("NcException","Error in test 22.2",__FILE__,__LINE__);
    if(enumType1.getMemberNameFromIndex(2) != "Wednesday") throw NcException("NcException","Error in test 22.3",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::getMemberNameFromValue(index)";
    if(enumType1.getMemberNameFromValue(1) != "Monday") throw NcException("NcException","Error in test 23.1",__FILE__,__LINE__);
    if(enumType1.getMemberNameFromValue(7) != "Tuesday") throw NcException("NcException","Error in test 23.2",__FILE__,__LINE__);
    if(enumType1.getMemberNameFromValue(-20) != "Wednesday") throw NcException("NcException","Error in test 23.3",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType::getMemberValue(index,value)";
    short value;
    enumType1.getMemberValue(0,value);
    if(value != 1) throw NcException("NcException","Error in test 24.1",__FILE__,__LINE__);
    enumType1.getMemberValue(1,value);
    if(value != 7) throw NcException("NcException","Error in test 24.2",__FILE__,__LINE__);
    enumType1.getMemberValue(2,value);
    if(value != -20) throw NcException("NcException","Error in test 24.3",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    cout <<left<<setw(57)<<"Testing NcEnumType constructors == and !=";
    NcType typeEnum(enumType1);
    NcEnumType EnumEnumType(typeEnum);
    if(enumType1 != EnumEnumType) throw NcException("NcException","Error in test 25.1",__FILE__,__LINE__);
    if(enumType1 != typeEnum) throw NcException("NcException","Error in test 25.2",__FILE__,__LINE__);
    if(!(enumType1 == EnumEnumType)) throw NcException("NcException","Error in test 25.3",__FILE__,__LINE__);
       if(!(enumType1 == typeEnum)) throw NcException("NcException","Error in test 25.4",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";




    cout <<left<<setw(57)<<"Testing ncFile::getType() (overloaded)";
    NcType typ1(ncFile.getType("enumType_1"));
    if(ncFile.getTypeCount()!= 4) throw NcException("NcException","Error in test 26.1",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_BYTE)!= 0) throw NcException("NcException","Error in test 26.3",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_CHAR)!= 0) throw NcException("NcException","Error in test 26.4",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_SHORT)!= 0) throw NcException("NcException","Error in test 26.5",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_INT)!= 0) throw NcException("NcException","Error in test 26.6",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_FLOAT)!= 0) throw NcException("NcException","Error in test 26.7",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_DOUBLE)!= 0) throw NcException("NcException","Error in test 26.8",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_UBYTE)!= 0) throw NcException("NcException","Error in test 26.9",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_USHORT)!= 0) throw NcException("NcException","Error in test 26.10",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_UINT)!= 0) throw NcException("NcException","Error in test 26.11",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_INT64)!= 0) throw NcException("NcException","Error in test 26.12",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_UINT64)!= 0) throw NcException("NcException","Error in test 26.13",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_STRING)!= 0) throw NcException("NcException","Error in test 26.14",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_VLEN)!= 1) throw NcException("NcException","Error in test 26.15",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_OPAQUE)!= 0) throw NcException("NcException","Error in test 26.16",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_ENUM)!= 1) throw NcException("NcException","Error in test 26.17",__FILE__,__LINE__);
    if(ncFile.getTypeCount(NcType::nc_COMPOUND)!= 2) throw NcException("NcException","Error in test 26.18",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";

    ncFile.getTypes("enumType_1",NcType::nc_ENUM);
    ncFile.getTypes(NcType::nc_ENUM);
    ncFile.getTypes("enumType_1");
    ncFile.getTypes();
    


    cout <<left<<setw(57)<<"Testing ENUM ncFile::putVar() and NcFile::getVar()";
    {
      NcDim dim1 = ncFile.addDim("dim1",10);
      NcVar var_1   = ncFile.addVar("var_1", enumType1,dim1);
      short enumValues[10];
      enumValues[0] =  1;
      enumValues[1] =  1;
      enumValues[2] =  1;
      enumValues[3] =  1;
      enumValues[4] =  1;
      enumValues[5] =  1;
      enumValues[6] =  1;
      enumValues[7] =  1;
      enumValues[8] =  -20;
      enumValues[9] =  7;
      var_1.putVar(enumValues);  

      NcVar var_2(ncFile.getVar("var_1"));
      
      if(var_2.isNull()) throw NcException("NcException","Error in test 27.1",__FILE__,__LINE__);
      short enumValues2[10];
      var_2.getVar(enumValues2);
      if(enumValues[0] != 1)  throw NcException("NcException","Error in test 27.2",__FILE__,__LINE__);
      if(enumValues[1] != 1)  throw NcException("NcException","Error in test 27.3",__FILE__,__LINE__);
      if(enumValues[2] != 1)  throw NcException("NcException","Error in test 27.4",__FILE__,__LINE__);
      if(enumValues[3] != 1)  throw NcException("NcException","Error in test 27.5",__FILE__,__LINE__);
      if(enumValues[4] != 1)  throw NcException("NcException","Error in test 27.6",__FILE__,__LINE__);
      if(enumValues[5] != 1)  throw NcException("NcException","Error in test 27.7",__FILE__,__LINE__);
      if(enumValues[6] != 1)  throw NcException("NcException","Error in test 27.8",__FILE__,__LINE__);
      if(enumValues[7] != 1)  throw NcException("NcException","Error in test 27.9",__FILE__,__LINE__);
      if(enumValues[8] != -20)throw NcException("NcException","Error in test 27.10",__FILE__,__LINE__);
      if(enumValues[9] != 7)  throw NcException("NcException","Error in test 27.11",__FILE__,__LINE__);
    }
    cout <<"    -----------   passed\n";
 
  



    cout <<left<<setw(57)<<"Testing COMPOUND ncFile::putVar() and NcFile::getVar()";
    {
      vector<int> vecSize(2); vecSize[0]=6,vecSize[1]=3;
      
      NcCompoundType compoundType3(ncFile.addCompoundType("compoundType_3",sizeof(struct3)));
      compoundType3.addMember("member1",ncInt,offsetof(struct3,mem1));
      compoundType3.addMember("member2",ncDouble,offsetof(struct3,mem2));
      vector<int> mem3Shape(1);mem3Shape[0]=3;
      compoundType3.addMember("member3",ncShort,offsetof(struct3,mem3),mem3Shape);

      NcDim dim3 = ncFile.addDim("dim3",2);
      NcVar var_3   = ncFile.addVar("var_3", compoundType3,dim3);
      
      struct3 dummyStruct;
      dummyStruct.mem1=1;
      dummyStruct.mem2=-1.23456;
      dummyStruct.mem3[0]=1;
      dummyStruct.mem3[1]=-6;
      dummyStruct.mem3[2]=20;
      
      struct3 dummyFill   ;
      dummyFill.mem1=94;
      dummyFill.mem2=95;
      dummyFill.mem3[0]=96;
      dummyFill.mem3[1]=97;
      dummyFill.mem3[2]=98;
      
      struct3 dummyStruct2[2];
      dummyStruct2[0].mem1=1;
      dummyStruct2[0].mem2=-1.23456;
      dummyStruct2[0].mem3[0]=1;
      dummyStruct2[0].mem3[1]=-6;
      dummyStruct2[0].mem3[2]=20;
      
      var_3.setFill(true,&dummyFill);

      vector<size_t> index(1);index[0]=1;
      //var_3.putVar(&dummyStruct2);  
      var_3.putVar(index,&dummyStruct);  
      
      NcVar var_4(ncFile.getVar("var_3"));

      if(var_4.isNull()) throw NcException("NcException","Error in test 28.1",__FILE__,__LINE__);
      struct3 dummyStruct3[2];
      var_4.getVar(dummyStruct3);

      if(dummyStruct3[1].mem1 != 1)  throw NcException("NcException","Error in test 28.2",__FILE__,__LINE__);
      if(dummyStruct3[1].mem2 != -1.23456)  throw NcException("NcException","Error in test 28.3",__FILE__,__LINE__);
      if(dummyStruct3[1].mem3[0] != 1)  throw NcException("NcException","Error in test 28.4",__FILE__,__LINE__);
      if(dummyStruct3[1].mem3[1] != -6)  throw NcException("NcException","Error in test 28.5",__FILE__,__LINE__);
      if(dummyStruct3[1].mem3[2] != 20)  throw NcException("NcException","Error in test 28.6",__FILE__,__LINE__);
      
      if(dummyStruct3[0].mem1 != 94)  throw NcException("NcException","Error in test 28.7",__FILE__,__LINE__);
      if(dummyStruct3[0].mem2 != 95)  throw NcException("NcException","Error in test 28.8",__FILE__,__LINE__);
      if(dummyStruct3[0].mem3[0] != 96)  throw NcException("NcException","Error in test 28.9",__FILE__,__LINE__);
      if(dummyStruct3[0].mem3[1] != 97)  throw NcException("NcException","Error in test 28.10",__FILE__,__LINE__);
      if(dummyStruct3[0].mem3[2] != 98)  throw NcException("NcException","Error in test 28.11",__FILE__,__LINE__);
      
    }
    cout <<"    -----------   passed\n";








    cout <<left<<setw(57)<<"Testing VLEN ncFile::putVar() and NcFile::getVar()";
    {
      NcVlenType vlenType3(ncFile.addVlenType("vlenType_3",ncShort));
      NcDim dim3(ncFile.getDim("dim3"));
      NcVar var_7   = ncFile.addVar("var_7", vlenType3,dim3);

      //  nc_vlen_t* dummyData = new nc_vlen_t[2];
      nc_vlen_t dummyData[2];
      //short int *vlenPointer = new short int[2];
      short int* vlenPointer;
      vlenPointer = (short int*) malloc(3*sizeof(short int));
      vlenPointer[0] = 1;
      vlenPointer[1] = 31;
      vlenPointer[2] = -20;
      dummyData[0].p = vlenPointer;
      dummyData[0].len = 3;
      vlenPointer = (short int*) malloc(2*sizeof(short int));
      vlenPointer[0] = 73;
      vlenPointer[1] = 64;
      dummyData[1].p = vlenPointer;
      dummyData[1].len = 2;

      var_7.putVar(dummyData);  

       nc_free_vlen(&dummyData[0]);
       nc_free_vlen(&dummyData[1]);
       // delete [] (short int*) dummyData[0].p;
       // delete [] (short int*) dummyData[1].p;

      NcVar var_8(ncFile.getVar("var_7"));

      if(var_8.isNull()) throw NcException("NcException","Error in test 29.1",__FILE__,__LINE__);

      // read data back
      nc_vlen_t dummyData2[2];
      var_8.getVar(dummyData2);
      if(dummyData2[0].len != 3)  throw NcException("NcException","Error in test 29.2",__FILE__,__LINE__);
      if(((short int *)dummyData2[0].p)[0] != 1)  throw NcException("NcException","Error in test 29.3",__FILE__,__LINE__);
      if(((short int *)dummyData2[0].p)[1] != 31)  throw NcException("NcException","Error in test 29.4",__FILE__,__LINE__);
      if(((short int *)dummyData2[0].p)[2] != -20)  throw NcException("NcException","Error in test 29.5",__FILE__,__LINE__);
      if(dummyData2[1].len != 2)  throw NcException("NcException","Error in test 29.6",__FILE__,__LINE__);
      if(((short int *)dummyData2[1].p)[0] != 73)  throw NcException("NcException","Error in test 29.7",__FILE__,__LINE__);
      if(((short int *)dummyData2[1].p)[1] != 64)  throw NcException("NcException","Error in test 29.8",__FILE__,__LINE__);
      nc_free_vlen(&dummyData2[0]);
      nc_free_vlen(&dummyData2[1]);

    }
    cout <<"    -----------   passed\n";



    cout <<left<<setw(57)<<"Testing COMPOUND with VLEN ncFile::putVar() and NcFile::getVar()";
    {
      // The following structure is the "definition" of the compound type:
      struct struct10{
	nc_vlen_t mem1[2];
	double mem2;
      };
      // define the Vlen type that is part of the compound type.
      NcVlenType vlenType13(ncFile.addVlenType("vlenType_13",ncShort));
      NcDim dim12 = ncFile.addDim("dim12",2);

      // now define the compound type
      NcCompoundType compoundType4(ncFile.addCompoundType("compoundType_4",sizeof(struct10)));
      vector<int> mem1Shape(1);mem1Shape[0]=2;
      compoundType4.addMember("member1",vlenType13,offsetof(struct10,mem1),mem1Shape);
      compoundType4.addMember("member2",ncDouble,offsetof(struct10,mem2));

      // finally define a variable containing this new compound type
      NcDim dim13 = ncFile.addDim("dim13",2);
      NcVar var_13   = ncFile.addVar("var_13", compoundType4,dim13);
      
      // here we populate a single entry.
      struct10 dummyData2;
      short int* vlenPointer;
      //      vlenPointer = new short int[3];
      vlenPointer = (short int*) malloc(3*sizeof(short int));
      vlenPointer[0] = 1;
      vlenPointer[1] = 31;
      vlenPointer[2] = -20;
      dummyData2.mem1[0];
      dummyData2.mem1[0].p = vlenPointer;
      dummyData2.mem1[0].len=3;
      //      vlenPointer = new short int[2];
      vlenPointer = (short int*) malloc(2*sizeof(short int));
      vlenPointer[0] = 73;
      vlenPointer[1] = 64;
      dummyData2.mem1[1].p=vlenPointer;
      dummyData2.mem1[1].len=2;
      
      dummyData2.mem2=20.1234;

      // ...and put the data into the netCDF file
      vector<size_t> index(1);index[0]=1;
      var_13.putVar(index,&dummyData2);  

      nc_free_vlen(&dummyData2.mem1[0]);
      nc_free_vlen(&dummyData2.mem1[1]);
      // delete [] dummyData;

    }
    cout <<"    -----------   passed\n";
      

 
}
catch (NcException& e)
  {
    cout << "unknown error"<<endl;
    e.what();
  }
}
