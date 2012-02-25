#include <exception>
#include <string>
#include <iostream>

#ifndef NcExceptionClasses
#define NcExceptionClasses

namespace netCDF
{

  //!  Exception classes.
  /*!
    These exceptions are thrown if the netCDF-4 API encounters an error.
  */
  namespace exceptions
  {

    /*! 
      Default object thrown if a netCDF exception is encountered.
      An unsatisfactory return from a call to one of the netCDF c-routines 
      generates an exception using an object inheriting this class.  All other netCDF-related
      errors  including those originating in the C++ binding, generates an NcException.
    */
    class NcException : public std::exception
    {
    public:
      NcException(const std::string& exceptionName,const std::string& complaint,const char* file,int line);
      virtual ~NcException() throw();
      void what()
      {
	//	std::cout<<fileName<<":"<<lnumber<<" "<<message<<std::endl;
	std::cout<<fileName<<":"<<lnumber<<" thrown exception "<<exceptionName<<".\n"<<message<<std::endl;
      }
    private:
      std::string message;      //error mesasge
      std::string exceptionName;//name of the exception
      std::string fileName;     //file containing the error
      int lnumber;              //line number where the error occurs
    };


    /*! Thrown if the specified netCDF ID does not refer to an open netCDF dataset. */
    class NcBadId : public NcException
    {
    public:
      NcBadId(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if too many netcdf files are open. */
    class NcNFile : public NcException
    {
    public:
      NcNFile(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if, having set NC_NOCLOBBER, the specified dataset already exists. */
    class NcExist : public NcException
    {
    public:
      NcExist(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if not a netCDF id.  */
    class NcInvalidArg : public NcException
    {
    public:
      NcInvalidArg(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if invalid argument. */
    class NcInvalidWrite : public NcException
    {
    public:
      NcInvalidWrite(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if operation not allowed in data mode. */
    class NcNotInDefineMode : public NcException
    {
    public:
      NcNotInDefineMode(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if operation not allowed in defined mode. */
    class NcInDefineMode : public NcException
    {
    public:
      NcInDefineMode(const std::string& complaint,const char* file,int line);
    };

    /*! 
      Index exceeds dimension bound.
      Exception may  be generated during operations to get or put  netCDF variable data.
      The exception is thrown if the specified indices were out of range for the rank of the 
      specified variable. For example, a negative index or an index that is larger than 
      the corresponding dimension length will cause an error.
    */
    class NcInvalidCoords : public NcException
    {
    public:
      NcInvalidCoords(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if NC_MAX_DIMS is exceeded. */
    class NcMaxDims : public NcException
    {
    public:
      NcMaxDims(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if string match to name is in use. */
    class NcNameInUse : public NcException
    {
    public:
      NcNameInUse(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attribute is not found. */
    class NcNotAtt : public NcException
    {
    public:
      NcNotAtt(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if Nc_MAX_ATTRS is exceeded. */
    class NcMaxAtts : public NcException
    {
    public:
      NcMaxAtts(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if not a valid netCDF data type. */
    class NcBadType : public NcException
    {
    public:
      NcBadType(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if an invalid dimension id or name. */
    class NcBadDim : public NcException
    {
    public:
      NcBadDim(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if Nc_UNLIMITED is in the wrong index. */
    class NcUnlimPos : public NcException
    {
    public:
      NcUnlimPos(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if NC_MAX_VARS is exceeded. */
    class NcMaxVars : public NcException
    {
    public:
      NcMaxVars(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if variable is not found. */
    class NcNotVar : public NcException
    {
    public:
      NcNotVar(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if the action is prohibited on the NC_GLOBAL varid. */
    class NcGlobal : public NcException
    {
    public:
      NcGlobal(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if not a netCDF file. */
    class NcNotNCF : public NcException
    {
    public:
      NcNotNCF(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if in FORTRAN, string is too short. */
    class NcSts : public NcException
    {
    public:
      NcSts(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if NC_MAX_NAME is exceeded. */
    class NcMaxName : public NcException
    {
    public:
      NcMaxName(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if NC_UNLIMITED size is already in use. */
    class NcUnlimit : public NcException
    {
    public:
      NcUnlimit(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if nc_rec op when there are no record vars. */
    class NcNoRecVars : public NcException
    {
    public:
      NcNoRecVars(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attempt to convert between text and numbers. */
    class NcChar : public NcException
    {
    public:
      NcChar(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if edge+start exceeds dimension bound. */
    class NcEdge : public NcException
    {
    public:
      NcEdge(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if illegal stride. */
    class NcStride : public NcException
    {
    public:
      NcStride(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attribute or variable name contains illegal characters. */
    class NcBadName : public NcException
    {
    public:
      NcBadName(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if math result not representable. */
    class NcRange : public NcException
    {
    public:
      NcRange(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if memory allocation (malloc) failure. */
    class NcNoMem : public NcException
    {
    public:
      NcNoMem(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if one or more variable sizes violate format constraints */
    class NcVarSize : public NcException
    {
    public:
      NcVarSize(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if invalid dimension size. */
    class NcDimSize : public NcException
    {
    public:
      NcDimSize(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if file likely truncated or possibly corrupted. */
    class NcTrunc : public NcException
    {
    public:
      NcTrunc(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if an error was reported by the HDF5 layer. */
    class NcHdfErr : public NcException
    {
    public:
      NcHdfErr(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if cannot read. */
    class NcCantRead : public NcException
    {
    public:
      NcCantRead(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if cannot write. */
    class NcCantWrite : public NcException
    {
    public:
      NcCantWrite(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if cannot create. */
    class NcCantCreate : public NcException
    {
    public:
      NcCantCreate(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if file meta. */
    class NcFileMeta : public NcException
    {
    public:
      NcFileMeta(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if dim meta. */
    class NcDimMeta : public NcException
    {
    public:
      NcDimMeta(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attribute meta. */
    class NcAttMeta : public NcException
    {
    public:
      NcAttMeta(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if variable meta. */
    class NcVarMeta : public NcException
    {
    public:
      NcVarMeta(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if no compound. */
    class NcNoCompound : public NcException
    {
    public:
      NcNoCompound(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attribute exists. */
    class NcAttExists : public NcException
    {
    public:
      NcAttExists(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attempting netcdf-4 operation on netcdf-3 file. */
    class NcNotNc4 : public NcException
    {
    public:
      NcNotNc4(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if attempting netcdf-4 operation on strict nc3 netcdf-4 file. */
    class NcStrictNc3 : public NcException
    {
    public:
      NcStrictNc3(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if bad group id. */
    class NcBadGroupId : public NcException
    {
    public:
      NcBadGroupId(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if bad type id. */
    class NcBadTypeId : public NcException
    {
    public:
      NcBadTypeId(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if bad field id. */
    class NcBadFieldId : public NcException
    {
    public:
      NcBadFieldId(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if cannot find the field id. */
    class NcUnknownName : public NcException
    {
    public:
      NcUnknownName(const std::string& complaint,const char* file,int line);
    };

    /*! Thrown if cannot return a netCDF group. */
    class NcEnoGrp : public NcException
    {
    public:
      NcEnoGrp(const std::string& complaint,const char* file,int line);
    };

    /*! 
      Thrown if the requested operation is on a NULL group.
    
      This exception is thrown if an operation on a NcGroup object is requested which is empty. To test if the object is empty used NcGroup::isNull()
     */
    class NcNullGrp : public NcException
    {
    public:
      NcNullGrp(const std::string& complaint,const char* file,int line);
    };

    /*! 
      Thrown if the requested operation is on a NULL type.
    
      This exception is thrown if an operation on a NcType object is requested which is empty. To test if the object is empty used NcType::isNull()
     */
    class NcNullType : public NcException
    {
    public:
      NcNullType(const std::string& complaint,const char* file,int line);
    };

    /*! 
      Thrown if the requested operation is on a NULL dimension.
    
      This exception is thrown if an operation on a NcDim object is requested which is empty. To test if the object is empty used NcDim::isNull()
     */
    class NcNullDim : public NcException
    {
    public:
      NcNullDim(const std::string& complaint,const char* file,int line);
    };

    /*! 
      Thrown if an operation to set the chunking, endianness, fill of a NcVar object is issued after a 
      call to NcVar::getVar or NcVar::putVar has been made.
    */
    class NcElateDef : public NcException
    {
    public:
      NcElateDef(const std::string& complaint,const char* file,int line);
    };

  }

}

#endif
