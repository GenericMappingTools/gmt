#ifndef NCSTREAMX_H
#define NCSTREAMX_H



typedef enum DataType {
    CHAR=0,
    BYTE=1,
    SHORT=2,
    INT=3,
    INT64=4,
    FLOAT=5,
    DOUBLE=6,
    STRING=7,
    STRUCTURE=8,
    SEQUENCE=9,
    ENUM1=10,
    ENUM2=11,
    ENUM4=12,
    OPAQUE=13,
    UBYTE=14,
    USHORT=15,
    UINT=16,
    UINT64=17
} DataType;


typedef enum Compress {
    NONE=0,
    DEFLATE=1
} Compress;

/* Forward definitions */
typedef struct Attribute Attribute;
typedef struct Dimension Dimension;
typedef struct Variable Variable;
typedef struct Structure Structure;
typedef struct EnumTypedef EnumTypedef;
typedef struct EnumType EnumType;
typedef struct Group Group;
typedef struct Header Header;
typedef struct Data Data;
typedef struct Range Range;
typedef struct Section Section;
typedef struct StructureData StructureData;
typedef struct Error Error;

struct Attribute {
    CRnode node;
    char* name;
    DataType type;
    uint32_t len;
    struct {int defined; bytes_t value;} data;
    struct {size_t count; char** values;} sdata;
};


extern ast_err Attribute_write(ast_runtime*,Attribute*);
extern ast_err Attribute_read(ast_runtime*,Attribute**);
extern ast_err Attribute_reclaim(ast_runtime*,Attribute*);
extern size_t Attribute_get_size(ast_runtime*,Attribute*);

struct Dimension {
    CRnode node;
    struct {int defined; char* value;} name;
    struct {int defined; uint64_t value;} length;
    struct {int defined; bool_t value;} isUnlimited;
    struct {int defined; bool_t value;} isVlen;
    struct {int defined; bool_t value;} isPrivate;
};


extern ast_err Dimension_write(ast_runtime*,Dimension*);
extern ast_err Dimension_read(ast_runtime*,Dimension**);
extern ast_err Dimension_reclaim(ast_runtime*,Dimension*);
extern size_t Dimension_get_size(ast_runtime*,Dimension*);

struct Variable {
    CRnode node;
    char* name;
    DataType dataType;
    struct {size_t count; Dimension** values;} shape;
    struct {size_t count; Attribute** values;} atts;
    struct {int defined; bool_t value;} unsigned_;
    struct {int defined; bytes_t value;} data;
    struct {int defined; char* value;} enumType;
    struct {size_t count; uint32_t* values;} dimIndex;
};


extern ast_err Variable_write(ast_runtime*,Variable*);
extern ast_err Variable_read(ast_runtime*,Variable**);
extern ast_err Variable_reclaim(ast_runtime*,Variable*);
extern size_t Variable_get_size(ast_runtime*,Variable*);

struct Structure {
    CRnode node;
    char* name;
    DataType dataType;
    struct {size_t count; Dimension** values;} shape;
    struct {size_t count; Attribute** values;} atts;
    struct {size_t count; Variable** values;} vars;
    struct {size_t count; Structure** values;} structs;
};


extern ast_err Structure_write(ast_runtime*,Structure*);
extern ast_err Structure_read(ast_runtime*,Structure**);
extern ast_err Structure_reclaim(ast_runtime*,Structure*);
extern size_t Structure_get_size(ast_runtime*,Structure*);

struct EnumTypedef {
    CRnode node;
    char* name;
    struct {size_t count; EnumType** values;} map;
};


extern ast_err EnumTypedef_write(ast_runtime*,EnumTypedef*);
extern ast_err EnumTypedef_read(ast_runtime*,EnumTypedef**);
extern ast_err EnumTypedef_reclaim(ast_runtime*,EnumTypedef*);
extern size_t EnumTypedef_get_size(ast_runtime*,EnumTypedef*);

struct EnumType {
    uint32_t code;
    char* value;
};


extern ast_err EnumType_write(ast_runtime*,EnumType*);
extern ast_err EnumType_read(ast_runtime*,EnumType**);
extern ast_err EnumType_reclaim(ast_runtime*,EnumType*);
extern size_t EnumType_get_size(ast_runtime*,EnumType*);

struct Group {
    CRnode node;
    char* name;
    struct {size_t count; Dimension** values;} dims;
    struct {size_t count; Variable** values;} vars;
    struct {size_t count; Structure** values;} structs;
    struct {size_t count; Attribute** values;} atts;
    struct {size_t count; Group** values;} groups;
    struct {size_t count; EnumTypedef** values;} enumTypes;
};


extern ast_err Group_write(ast_runtime*,Group*);
extern ast_err Group_read(ast_runtime*,Group**);
extern ast_err Group_reclaim(ast_runtime*,Group*);
extern size_t Group_get_size(ast_runtime*,Group*);

struct Header {
    CRnode node;
    struct {int defined; char* value;} location;
    struct {int defined; char* value;} title;
    struct {int defined; char* value;} id;
    Group* root;
    struct {int defined; uint32_t value;} version;
};


extern ast_err Header_write(ast_runtime*,Header*);
extern ast_err Header_read(ast_runtime*,Header**);
extern ast_err Header_reclaim(ast_runtime*,Header*);
extern size_t Header_get_size(ast_runtime*,Header*);

struct Data {
    CRnode node;
    char* varName;
    DataType dataType;
    struct {int defined; Section* value;} section;
    struct {int defined; bool_t value;} bigend;
    struct {int defined; uint32_t value;} version;
    struct {int defined; Compress value;} compress;
    struct {int defined; uint32_t value;} crc32;
};


extern ast_err Data_write(ast_runtime*,Data*);
extern ast_err Data_read(ast_runtime*,Data**);
extern ast_err Data_reclaim(ast_runtime*,Data*);
extern size_t Data_get_size(ast_runtime*,Data*);

struct Range {
    CRnode node;
    struct {int defined; uint64_t value;} start;
    uint64_t size;
    struct {int defined; uint64_t value;} stride;
};


extern ast_err Range_write(ast_runtime*,Range*);
extern ast_err Range_read(ast_runtime*,Range**);
extern ast_err Range_reclaim(ast_runtime*,Range*);
extern size_t Range_get_size(ast_runtime*,Range*);

struct Section {
    CRnode node;
    struct {size_t count; Range** values;} range;
};


extern ast_err Section_write(ast_runtime*,Section*);
extern ast_err Section_read(ast_runtime*,Section**);
extern ast_err Section_reclaim(ast_runtime*,Section*);
extern size_t Section_get_size(ast_runtime*,Section*);

struct StructureData {
    CRnode node;
    struct {size_t count; uint32_t* values;} member;
    bytes_t data;
    struct {size_t count; uint32_t* values;} heapCount;
    struct {size_t count; char** values;} sdata;
    struct {int defined; uint64_t value;} nrows;
};


extern ast_err StructureData_write(ast_runtime*,StructureData*);
extern ast_err StructureData_read(ast_runtime*,StructureData**);
extern ast_err StructureData_reclaim(ast_runtime*,StructureData*);
extern size_t StructureData_get_size(ast_runtime*,StructureData*);

struct Error {
    CRnode node;
    char* message;
};


extern ast_err Error_write(ast_runtime*,Error*);
extern ast_err Error_read(ast_runtime*,Error**);
extern ast_err Error_reclaim(ast_runtime*,Error*);
extern size_t Error_get_size(ast_runtime*,Error*);

#endif /*NCSTREAMX_H*/
