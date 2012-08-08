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
      NcFile ncFile("firstFile.cdf",NcFile::replace);
      
      NcGroup groupA(ncFile.addGroup("groupA"));
      NcGroup groupA0(ncFile.addGroup("groupA0"));
      NcGroup groupB(groupA.addGroup("groupB"));
      NcGroup groupC(groupA.addGroup("groupC"));
    
      NcDim dim1 = ncFile.addDim("dim1",10);
      NcDim dim2 = ncFile.addDim("dim2");
      NcDim dim3 = ncFile.addDim("dim3",13);
      NcDim dim4 = groupB.addDim("dim4",14);
      NcDim dim5 = groupB.addDim("dim5",15);
      NcDim dim6 = groupB.addDim("dim6",16);
      NcDim dim7 = groupB.addDim("dim7",17);


      NcVar var_1   = ncFile.addVar("var_1",   ncInt,dim1);
      NcVar var_2   = ncFile.addVar("var_2",   ncInt,dim1);
      NcVar var_3   = ncFile.addVar("var_3",   ncInt,dim1);
      NcVar var_4   = ncFile.addVar("var_4",   ncInt,dim1);
      NcVar var_5   = ncFile.addVar("var_5",   ncInt,dim1);
      NcVar var_6   = ncFile.addVar("var_6",   ncInt,dim1);
      NcVar var_7   = ncFile.addVar("var_7",   ncInt,dim1);
      NcVar var_8   = ncFile.addVar("var_8",   ncInt,dim1);
      NcVar var_9   = ncFile.addVar("var_9",   ncInt,dim1);
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
      

      vector<short>  a1(10),b1(10);
      vector<int>  a2(10),b2(10);
      vector<float>  a3(10),b3(10);
      vector<double>  a4(10),b4(10);
      vector<unsigned short>  a5(10),b5(10);
      vector<unsigned int>  a6(10),b6(10);
      vector<long long>  a7(10),b7(10);
      vector<unsigned long long>  a8(10),b8(10);
      vector<double>  a9(10),b9(10);
      initializeVector(a1);
      initializeVector(a2);
      initializeVector(a3);
      initializeVector(a4);
      initializeVector(a5);
      initializeVector(a6);
      initializeVector(a7);
      initializeVector(a8);
      initializeVector(a9);
      for(int  i=0; i<a1.size(); i++) {
	a1[i] *= 1;
	a2[i] *= 2;
	a3[i] *= 3;
	a4[i] *= 4;
	a5[i] *= 5;
	a6[i] *= 6;
	a7[i] *= 7;
	a8[i] *= 0.8;
	a9[i] *= 9;
      }

      bool fillMode;
      int bFill=13;
      var_2.setFill(true,bFill);
      var_2.setChecksum(NcVar::nc_NOCHECKSUM);
      vector<size_t> bb(1); bb[0]=5;
      var_2.setChunking(NcVar::nc_CHUNKED,bb);
      var_2.setEndianness(NcVar::nc_ENDIAN_LITTLE);
      var_2.setCompression(false,true,9);

      // put variables of different type into the same variable type: requires conversion.
      var_1.putVar(&a1[0]);  
      var_2.putVar(&a2[0]);  
      var_3.putVar(&a3[0]);  
      var_4.putVar(&a4[0]);  
      var_5.putVar(&a5[0]);  
      var_6.putVar(&a6[0]);  
      var_7.putVar(&a7[0]);  
      var_8.putVar(&a8[0]);  
      var_9.putVar(&a9[0]);  

      // get variable out
      vector<size_t> index(1);index[0]=5;
      vector<size_t> index2(1);index2[0]=4;
      var_1.getVar(&b1[0]);  
      //      var_2.getVar(&b2[0]);  
      //      var_2.getVar(index,&b2[0]);  
      var_2.getVar(index,index2,&b2[0]);  
      for(int  i=0; i<b2.size(); i++) {
	cout << "i ="<<i<<" vector="<<b2[i]<<endl;
      }

      int bFill2;
      var_2.getFillModeParameters(fillMode,(void*)&bFill2);
      netCDF::NcVar::ChecksumMode  aa=var_2.getChecksum();
      vector<size_t> cc2(1);
      netCDF::NcVar::ChunkMode cc1;
      var_2.getChunkingParameters(cc1,cc2);
      cout<< fillMode<<endl;
      bool shuffleFilterEnabled,deflateFilterEnabled;
      int deflateLevel;
      var_2.getCompressionParameters(shuffleFilterEnabled,deflateFilterEnabled,deflateLevel);
      cout << "fill parameter ="<<bFill2<<endl;
      cout << "checksum  ="<<aa<<endl;
      cout << "chunking parameters: chunk mode="<<cc1<<"    Chunk sizes="<<cc2[0]<<endl;
      cout << "endianness ="<<var_2.getEndianness()<<endl;
      cout << "shuffleFilterEnabled = "<<shuffleFilterEnabled <<endl;
      cout << "deflateFilterEnabled = "<<deflateFilterEnabled <<endl;
      cout << "deflateLevel = "<<deflateLevel <<endl;
      cout << "parentGroup ="<<var_2.getParentGroup().getName()<<endl;
      cout << "name ="<<var_2.getName()<<endl;
      cout << "dimCount ="<<var_2.getDimCount()<<endl;
      cout << "dimName ="<<var_2.getDim(0).getName()<<endl;
      cout << "nctypeName ="<<var_2.getType().getName()<<endl;



      const string b("abc");
      //      NcVarAtt att4  = var_2.putAtt("att4",ncChar,size_t(2), b.c_str());
      const char* c(b.c_str());
      //NcVarAtt att12 = var_2.putAtt("att12",ncString,size_t(1), &c);
      vector<short>  a1x(10);
      initializeVector(a1x);
      NcVarAtt attA7_1  = var_2.putAtt("att7_1",ncUint,size_t(10),    &a1x[0]);

      cout << "varAtt variable name ="<<attA7_1.getParentVar().getName()<<endl;



      var_3.getVar(&b3[0]);  
      var_4.getVar(&b4[0]);  
      var_5.getVar(&b5[0]);  
      var_6.getVar(&b6[0]);  
      var_7.getVar(&b7[0]);  
      var_8.getVar(&b8[0]);  
      var_9.getVar(&b9[0]);  
      
      /*
	if(a1 != b1)  throw NcException("NcException","Error in test 1.1",__FILE__,__LINE__);
	if(a2 != b2)  throw NcException("NcException","Error in test 1.2",__FILE__,__LINE__);
	if(a3 != b3)  throw NcException("NcException","Error in test 1.3",__FILE__,__LINE__);
	if(a4 != b4)  throw NcException("NcException","Error in test 1.4",__FILE__,__LINE__);
	if(a5 != b5)  throw NcException("NcException","Error in test 1.5",__FILE__,__LINE__);
	if(a6 != b6)  throw NcException("NcException","Error in test 1.6",__FILE__,__LINE__);
	if(a7 != b7)  throw NcException("NcException","Error in test 1.7",__FILE__,__LINE__);
	if(a8 != b8)  throw NcException("NcException","Error in test 1.8",__FILE__,__LINE__);
	if(a9 != b9)  throw NcException("NcException","Error in test 1.9",__FILE__,__LINE__);
      */

      //   check writing out too much or too little data!!!!
      // put in documentation that for the put() and get() you must be sure that      // you get the variables correctly sized
      //   implement....
      //  putVars(variable, numbr of elements)
      //  putVar(variable)

    }
  catch (NcException& e)
    {
      cout << "unknown error"<<endl;
      e.what();
    }
}
