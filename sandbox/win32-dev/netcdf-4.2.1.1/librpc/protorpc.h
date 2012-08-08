#ifndef PROTORPC_H
#define PROTORPC_H



typedef enum RPC_Call {
    NCRPC_CREATE=0,
    NCRPC_OPEN=1,
    NCRPC_NEW_NC=2,
    NCRPC_FREE_NC=3,
    NCRPC_REDEF=4,
    NCRPC__ENDDEF=5,
    NCRPC_SYNC=6,
    NCRPC_ABORT=7,
    NCRPC_CLOSE=8,
    NCRPC_SET_FILL=9,
    NCRPC_SET_BASE_PE=10,
    NCRPC_INQ_BASE_PE=11,
    NCRPC_INQ_FORMAT=12,
    NCRPC_INQ=13,
    NCRPC_INQ_TYPE=14,
    NCRPC_DEF_DIM=15,
    NCRPC_INQ_DIMID=16,
    NCRPC_INQ_DIM=17,
    NCRPC_INQ_UNLIMDIM=18,
    NCRPC_RENAME_DIM=19,
    NCRPC_INQ_ATT=20,
    NCRPC_INQ_ATTID=21,
    NCRPC_INQ_ATTNAME=22,
    NCRPC_RENAME_ATT=23,
    NCRPC_DEL_ATT=24,
    NCRPC_GET_ATT=25,
    NCRPC_PUT_ATT=26,
    NCRPC_DEF_VAR=27,
    NCRPC_INQ_VAR_ALL=28,
    NCRPC_INQ_VARID=29,
    NCRPC_RENAME_VAR=30,
    NCRPC_PUT_VARA=31,
    NCRPC_GET_VARA=32,
    NCRPC_VAR_PAR_ACCESS=33,
    NCRPC_INQ_NCID=34,
    NCRPC_INQ_GRPS=35,
    NCRPC_INQ_GRPNAME=36,
    NCRPC_INQ_GRPNAME_FULL=37,
    NCRPC_INQ_GRP_PARENT=38,
    NCRPC_INQ_GRP_FULL_NCID=39,
    NCRPC_INQ_VARIDS=40,
    NCRPC_INQ_DIMIDS=41,
    NCRPC_INQ_TYPEIDS=42,
    NCRPC_INQ_TYPE_EQUAL=43,
    NCRPC_DEF_GRP=44,
    NCRPC_INQ_USER_TYPE=45,
    NCRPC_DEF_COMPOUND=46,
    NCRPC_INSERT_COMPOUND=47,
    NCRPC_INSERT_ARRAY_COMPOUND=48,
    NCRPC_INQ_TYPEID=49,
    NCRPC_INQ_COMPOUND_FIELD=50,
    NCRPC_INQ_COMPOUND_FIELDINDEX=51,
    NCRPC_DEF_VLEN=52,
    NCRPC_PUT_VLEN_ELEMENT=53,
    NCRPC_GET_VLEN_ELEMENT=54,
    NCRPC_DEF_ENUM=55,
    NCRPC_INSERT_ENUM=56,
    NCRPC_INQ_ENUM_MEMBER=57,
    NCRPC_INQ_ENUM_IDENT=58,
    NCRPC_DEF_OPAQUE=59,
    NCRPC_DEF_VAR_DEFLATE=60,
    NCRPC_DEF_VAR_FLETCHER32=61,
    NCRPC_DEF_VAR_CHUNKING=62,
    NCRPC_DEF_VAR_FILL=63,
    NCRPC_DEF_VAR_ENDIAN=64,
    NCRPC_SET_VAR_CHUNK_CACHE=65,
    NCRPC_GET_VAR_CHUNK_CACHE=66,
    NCRPC_INQ_UNLIMDIMS=67,
    NCRPC_SHOW_METADATA=68,
    NCRPC_INITIALIZE=69,
    NCRPC_GETMETADATA=70
} RPC_Call;


typedef enum nc_meta {
    META_NAT=0,
    META_BYTE=1,
    META_CHAR=2,
    META_SHORT=3,
    META_INT=4,
    META_FLOAT=5,
    META_DOUBLE=6,
    META_UBYTE=7,
    META_USHORT=8,
    META_UINT=9,
    META_INT64=10,
    META_UINT64=11,
    META_STRING=12,
    META_VLEN=13,
    META_OPAQUE=14,
    META_ENUM=15,
    META_COMPOUND=16,
    META_ATOMIC=64,
    META_FIELD=65,
    META_GRAPH=66,
    META_GROUP=67,
    META_VAR=68,
    META_DIM=69
} nc_meta;

/* Forward definitions */
typedef struct NCCreate NCCreate;
typedef struct NCCreate_Return NCCreate_Return;
typedef struct NCOpen NCOpen;
typedef struct NCOpen_Return NCOpen_Return;
typedef struct NCRedef NCRedef;
typedef struct NCRedef_Return NCRedef_Return;
typedef struct NC_Enddef NC_Enddef;
typedef struct NC_Enddef_Return NC_Enddef_Return;
typedef struct NCSync NCSync;
typedef struct NCSync_Return NCSync_Return;
typedef struct NCAbort NCAbort;
typedef struct NCAbort_Return NCAbort_Return;
typedef struct NCClose NCClose;
typedef struct NCClose_Return NCClose_Return;
typedef struct NCSet_Fill NCSet_Fill;
typedef struct NCSet_Fill_Return NCSet_Fill_Return;
typedef struct NCInq_Base_PE NCInq_Base_PE;
typedef struct NCInq_Base_PE_Return NCInq_Base_PE_Return;
typedef struct NCSet_base_pe NCSet_base_pe;
typedef struct NCSet_base_pe_Return NCSet_base_pe_Return;
typedef struct NCInq_format NCInq_format;
typedef struct NCInq_format_Return NCInq_format_Return;
typedef struct NCInq NCInq;
typedef struct NCInq_Return NCInq_Return;
typedef struct NCInq_Type NCInq_Type;
typedef struct NCInq_Type_Return NCInq_Type_Return;
typedef struct NCDef_Dim NCDef_Dim;
typedef struct NCDef_Dim_Return NCDef_Dim_Return;
typedef struct NCInq_dimid NCInq_dimid;
typedef struct NCInq_dimid_Return NCInq_dimid_Return;
typedef struct NCInq_dim NCInq_dim;
typedef struct NCInq_dim_Return NCInq_dim_Return;
typedef struct NCInq_unlimdim NCInq_unlimdim;
typedef struct NCInq_unlimdim_Return NCInq_unlimdim_Return;
typedef struct NCRename_dim NCRename_dim;
typedef struct NCRename_dim_Return NCRename_dim_Return;
typedef struct NCInq_att NCInq_att;
typedef struct NCInq_att_Return NCInq_att_Return;
typedef struct NCInq_attid NCInq_attid;
typedef struct NCInq_attid_Return NCInq_attid_Return;
typedef struct NCInq_attname NCInq_attname;
typedef struct NCInq_attname_Return NCInq_attname_Return;
typedef struct NCRename_att NCRename_att;
typedef struct NCRename_att_Return NCRename_att_Return;
typedef struct NCDel_att NCDel_att;
typedef struct NCDel_att_Return NCDel_att_Return;
typedef struct NCGet_att NCGet_att;
typedef struct NCGet_att_Return NCGet_att_Return;
typedef struct NCPut_att NCPut_att;
typedef struct NCPut_att_Return NCPut_att_Return;
typedef struct NCDef_Var NCDef_Var;
typedef struct NCDef_Var_Return NCDef_Var_Return;
typedef struct NCInq_varid NCInq_varid;
typedef struct NCInq_varid_Return NCInq_varid_Return;
typedef struct NCRename_var NCRename_var;
typedef struct NCRename_var_Return NCRename_var_Return;
typedef struct NCGet_vara NCGet_vara;
typedef struct NCGet_vara_Return NCGet_vara_Return;
typedef struct NCPut_vara NCPut_vara;
typedef struct NCPut_vara_Return NCPut_vara_Return;
typedef struct NCGet_vars NCGet_vars;
typedef struct NCGet_vars_Return NCGet_vars_Return;
typedef struct NCPut_vars NCPut_vars;
typedef struct NCPut_vars_Return NCPut_vars_Return;
typedef struct NCGet_varm NCGet_varm;
typedef struct NCGet_varm_Return NCGet_varm_Return;
typedef struct NCPut_varm NCPut_varm;
typedef struct NCPut_varm_Return NCPut_varm_Return;
typedef struct NCInq_var_all NCInq_var_all;
typedef struct NCInq_var_all_Return NCInq_var_all_Return;
typedef struct NCShow_metadata NCShow_metadata;
typedef struct NCShow_metadata_Return NCShow_metadata_Return;
typedef struct NCInq_unlimdims NCInq_unlimdims;
typedef struct NCInq_unlimdims_Return NCInq_unlimdims_Return;
typedef struct NCVar_par_access NCVar_par_access;
typedef struct NCVar_par_access_Return NCVar_par_access_Return;
typedef struct NCInq_ncid NCInq_ncid;
typedef struct NCInq_ncid_Return NCInq_ncid_Return;
typedef struct NCInq_grps NCInq_grps;
typedef struct NCInq_grps_Return NCInq_grps_Return;
typedef struct NCInq_grpname NCInq_grpname;
typedef struct NCInq_grpname_Return NCInq_grpname_Return;
typedef struct NCInq_grpname_full NCInq_grpname_full;
typedef struct NCInq_grpname_full_Return NCInq_grpname_full_Return;
typedef struct NCInq_grp_parent NCInq_grp_parent;
typedef struct NCInq_grp_parent_Return NCInq_grp_parent_Return;
typedef struct NCInq_grp_full_ncid NCInq_grp_full_ncid;
typedef struct NCInq_grp_full_ncid_Return NCInq_grp_full_ncid_Return;
typedef struct NCInq_varids NCInq_varids;
typedef struct NCInq_varids_Return NCInq_varids_Return;
typedef struct NCInq_dimids NCInq_dimids;
typedef struct NCInq_dimids_Return NCInq_dimids_Return;
typedef struct NCInq_typeids NCInq_typeids;
typedef struct NCInq_typeids_Return NCInq_typeids_Return;
typedef struct NCInq_type_equal NCInq_type_equal;
typedef struct NCInq_type_equal_Return NCInq_type_equal_Return;
typedef struct NCDef_Grp NCDef_Grp;
typedef struct NCDef_Grp_Return NCDef_Grp_Return;
typedef struct NCInq_user_type NCInq_user_type;
typedef struct NCInq_user_type_Return NCInq_user_type_Return;
typedef struct NCInq_typeid NCInq_typeid;
typedef struct NCInq_typeid_Return NCInq_typeid_Return;
typedef struct NCDef_Compound NCDef_Compound;
typedef struct NCDef_Compound_Return NCDef_Compound_Return;
typedef struct NCInsert_compound NCInsert_compound;
typedef struct NCInsert_compound_Return NCInsert_compound_Return;
typedef struct NCInsert_array_compound NCInsert_array_compound;
typedef struct NCInsert_array_compound_Return NCInsert_array_compound_Return;
typedef struct NCInq_compound_field NCInq_compound_field;
typedef struct NCInq_compound_field_Return NCInq_compound_field_Return;
typedef struct NCInq_compound_fieldindex NCInq_compound_fieldindex;
typedef struct NCInq_compound_fieldindex_Return NCInq_compound_fieldindex_Return;
typedef struct NCDef_Vlen NCDef_Vlen;
typedef struct NCDef_Vlen_Return NCDef_Vlen_Return;
typedef struct NCPut_vlen_element NCPut_vlen_element;
typedef struct NCPut_vlen_element_Return NCPut_vlen_element_Return;
typedef struct NCGet_vlen_element NCGet_vlen_element;
typedef struct NCGet_vlen_element_Return NCGet_vlen_element_Return;
typedef struct NCDef_Enum NCDef_Enum;
typedef struct NCDef_Enum_Return NCDef_Enum_Return;
typedef struct NCInsert_enum NCInsert_enum;
typedef struct NCInsert_enum_Return NCInsert_enum_Return;
typedef struct NCInq_enum_member NCInq_enum_member;
typedef struct NCInq_enum_member_Return NCInq_enum_member_Return;
typedef struct NCInq_enum_ident NCInq_enum_ident;
typedef struct NCInq_enum_ident_Return NCInq_enum_ident_Return;
typedef struct NCDef_Opaque NCDef_Opaque;
typedef struct NCDef_Opaque_Return NCDef_Opaque_Return;
typedef struct NCDef_var_deflate NCDef_var_deflate;
typedef struct NCDef_var_deflate_Return NCDef_var_deflate_Return;
typedef struct NCDef_Var_Fletcher32 NCDef_Var_Fletcher32;
typedef struct NCDef_Var_Fletcher32_Return NCDef_Var_Fletcher32_Return;
typedef struct NCDef_Var_Chunking NCDef_Var_Chunking;
typedef struct NCDef_Var_Chunking_Return NCDef_Var_Chunking_Return;
typedef struct NCDef_Var_Fill NCDef_Var_Fill;
typedef struct NCDef_Var_Fill_Return NCDef_Var_Fill_Return;
typedef struct NCDef_Var_endian NCDef_Var_endian;
typedef struct NCDef_Var_endian_Return NCDef_Var_endian_Return;
typedef struct NCSet_var_chunk_cache NCSet_var_chunk_cache;
typedef struct NCSet_var_chunk_cache_Return NCSet_var_chunk_cache_Return;
typedef struct NCGet_var_chunk_cache NCGet_var_chunk_cache;
typedef struct NCGet_var_chunk_cache_Return NCGet_var_chunk_cache_Return;
typedef struct NCNC_set_log_level NCNC_set_log_level;
typedef struct NCNC_set_log_level_Return NCNC_set_log_level_Return;
typedef struct NCNC_inq_libvers NCNC_inq_libvers;
typedef struct NCNC_inq_libvers_Return NCNC_inq_libvers_Return;
typedef struct NCNC_delete_mp NCNC_delete_mp;
typedef struct NCNC_delete_mp_Return NCNC_delete_mp_Return;
typedef struct MetaNode MetaNode;
typedef struct MetaGraph MetaGraph;
typedef struct MetaGroup MetaGroup;
typedef struct MetaVar MetaVar;
typedef struct MetaDim MetaDim;
typedef struct MetaCompound MetaCompound;
typedef struct MetaField MetaField;
typedef struct MetaEnum MetaEnum;
typedef struct MetaEconst MetaEconst;

struct NCCreate {
    char* path;
    int32_t cmode;
    uint64_t initialsz;
    int32_t basepe;
    int32_t use_parallel;
};


extern ast_err NCCreate_write(ast_runtime*,NCCreate*);
extern ast_err NCCreate_read(ast_runtime*,NCCreate**);
extern ast_err NCCreate_reclaim(ast_runtime*,NCCreate*);
extern size_t NCCreate_get_size(ast_runtime*,NCCreate*);

struct NCCreate_Return {
    int32_t ncstatus;
    int32_t ncid;
};


extern ast_err NCCreate_Return_write(ast_runtime*,NCCreate_Return*);
extern ast_err NCCreate_Return_read(ast_runtime*,NCCreate_Return**);
extern ast_err NCCreate_Return_reclaim(ast_runtime*,NCCreate_Return*);
extern size_t NCCreate_Return_get_size(ast_runtime*,NCCreate_Return*);

struct NCOpen {
    char* path;
    int32_t cmode;
    int32_t basepe;
    struct {size_t count; uint64_t* values;} chunksizehint;
    int32_t use_parallel;
    bytes_t parameters;
};


extern ast_err NCOpen_write(ast_runtime*,NCOpen*);
extern ast_err NCOpen_read(ast_runtime*,NCOpen**);
extern ast_err NCOpen_reclaim(ast_runtime*,NCOpen*);
extern size_t NCOpen_get_size(ast_runtime*,NCOpen*);

struct NCOpen_Return {
    int32_t ncstatus;
    int32_t ncid;
};


extern ast_err NCOpen_Return_write(ast_runtime*,NCOpen_Return*);
extern ast_err NCOpen_Return_read(ast_runtime*,NCOpen_Return**);
extern ast_err NCOpen_Return_reclaim(ast_runtime*,NCOpen_Return*);
extern size_t NCOpen_Return_get_size(ast_runtime*,NCOpen_Return*);

struct NCRedef {
    int32_t ncid;
};


extern ast_err NCRedef_write(ast_runtime*,NCRedef*);
extern ast_err NCRedef_read(ast_runtime*,NCRedef**);
extern ast_err NCRedef_reclaim(ast_runtime*,NCRedef*);
extern size_t NCRedef_get_size(ast_runtime*,NCRedef*);

struct NCRedef_Return {
    int32_t ncstatus;
};


extern ast_err NCRedef_Return_write(ast_runtime*,NCRedef_Return*);
extern ast_err NCRedef_Return_read(ast_runtime*,NCRedef_Return**);
extern ast_err NCRedef_Return_reclaim(ast_runtime*,NCRedef_Return*);
extern size_t NCRedef_Return_get_size(ast_runtime*,NCRedef_Return*);

struct NC_Enddef {
    int32_t ncid;
    uint64_t minfree;
    uint64_t v_align;
    uint64_t v_minfree;
    uint64_t r_align;
};


extern ast_err NC_Enddef_write(ast_runtime*,NC_Enddef*);
extern ast_err NC_Enddef_read(ast_runtime*,NC_Enddef**);
extern ast_err NC_Enddef_reclaim(ast_runtime*,NC_Enddef*);
extern size_t NC_Enddef_get_size(ast_runtime*,NC_Enddef*);

struct NC_Enddef_Return {
    int32_t ncstatus;
};


extern ast_err NC_Enddef_Return_write(ast_runtime*,NC_Enddef_Return*);
extern ast_err NC_Enddef_Return_read(ast_runtime*,NC_Enddef_Return**);
extern ast_err NC_Enddef_Return_reclaim(ast_runtime*,NC_Enddef_Return*);
extern size_t NC_Enddef_Return_get_size(ast_runtime*,NC_Enddef_Return*);

struct NCSync {
    int32_t ncid;
};


extern ast_err NCSync_write(ast_runtime*,NCSync*);
extern ast_err NCSync_read(ast_runtime*,NCSync**);
extern ast_err NCSync_reclaim(ast_runtime*,NCSync*);
extern size_t NCSync_get_size(ast_runtime*,NCSync*);

struct NCSync_Return {
    int32_t ncstatus;
};


extern ast_err NCSync_Return_write(ast_runtime*,NCSync_Return*);
extern ast_err NCSync_Return_read(ast_runtime*,NCSync_Return**);
extern ast_err NCSync_Return_reclaim(ast_runtime*,NCSync_Return*);
extern size_t NCSync_Return_get_size(ast_runtime*,NCSync_Return*);

struct NCAbort {
    int32_t ncid;
};


extern ast_err NCAbort_write(ast_runtime*,NCAbort*);
extern ast_err NCAbort_read(ast_runtime*,NCAbort**);
extern ast_err NCAbort_reclaim(ast_runtime*,NCAbort*);
extern size_t NCAbort_get_size(ast_runtime*,NCAbort*);

struct NCAbort_Return {
    int32_t ncstatus;
};


extern ast_err NCAbort_Return_write(ast_runtime*,NCAbort_Return*);
extern ast_err NCAbort_Return_read(ast_runtime*,NCAbort_Return**);
extern ast_err NCAbort_Return_reclaim(ast_runtime*,NCAbort_Return*);
extern size_t NCAbort_Return_get_size(ast_runtime*,NCAbort_Return*);

struct NCClose {
    int32_t ncid;
};


extern ast_err NCClose_write(ast_runtime*,NCClose*);
extern ast_err NCClose_read(ast_runtime*,NCClose**);
extern ast_err NCClose_reclaim(ast_runtime*,NCClose*);
extern size_t NCClose_get_size(ast_runtime*,NCClose*);

struct NCClose_Return {
    int32_t ncstatus;
};


extern ast_err NCClose_Return_write(ast_runtime*,NCClose_Return*);
extern ast_err NCClose_Return_read(ast_runtime*,NCClose_Return**);
extern ast_err NCClose_Return_reclaim(ast_runtime*,NCClose_Return*);
extern size_t NCClose_Return_get_size(ast_runtime*,NCClose_Return*);

struct NCSet_Fill {
    int32_t ncid;
    int32_t fillmode;
};


extern ast_err NCSet_Fill_write(ast_runtime*,NCSet_Fill*);
extern ast_err NCSet_Fill_read(ast_runtime*,NCSet_Fill**);
extern ast_err NCSet_Fill_reclaim(ast_runtime*,NCSet_Fill*);
extern size_t NCSet_Fill_get_size(ast_runtime*,NCSet_Fill*);

struct NCSet_Fill_Return {
    int32_t ncstatus;
    int32_t oldmode;
};


extern ast_err NCSet_Fill_Return_write(ast_runtime*,NCSet_Fill_Return*);
extern ast_err NCSet_Fill_Return_read(ast_runtime*,NCSet_Fill_Return**);
extern ast_err NCSet_Fill_Return_reclaim(ast_runtime*,NCSet_Fill_Return*);
extern size_t NCSet_Fill_Return_get_size(ast_runtime*,NCSet_Fill_Return*);

struct NCInq_Base_PE {
    int32_t ncid;
};


extern ast_err NCInq_Base_PE_write(ast_runtime*,NCInq_Base_PE*);
extern ast_err NCInq_Base_PE_read(ast_runtime*,NCInq_Base_PE**);
extern ast_err NCInq_Base_PE_reclaim(ast_runtime*,NCInq_Base_PE*);
extern size_t NCInq_Base_PE_get_size(ast_runtime*,NCInq_Base_PE*);

struct NCInq_Base_PE_Return {
    int32_t ncstatus;
    int32_t pe;
};


extern ast_err NCInq_Base_PE_Return_write(ast_runtime*,NCInq_Base_PE_Return*);
extern ast_err NCInq_Base_PE_Return_read(ast_runtime*,NCInq_Base_PE_Return**);
extern ast_err NCInq_Base_PE_Return_reclaim(ast_runtime*,NCInq_Base_PE_Return*);
extern size_t NCInq_Base_PE_Return_get_size(ast_runtime*,NCInq_Base_PE_Return*);

struct NCSet_base_pe {
    int32_t ncid;
    int32_t pe;
};


extern ast_err NCSet_base_pe_write(ast_runtime*,NCSet_base_pe*);
extern ast_err NCSet_base_pe_read(ast_runtime*,NCSet_base_pe**);
extern ast_err NCSet_base_pe_reclaim(ast_runtime*,NCSet_base_pe*);
extern size_t NCSet_base_pe_get_size(ast_runtime*,NCSet_base_pe*);

struct NCSet_base_pe_Return {
    int32_t ncstatus;
};


extern ast_err NCSet_base_pe_Return_write(ast_runtime*,NCSet_base_pe_Return*);
extern ast_err NCSet_base_pe_Return_read(ast_runtime*,NCSet_base_pe_Return**);
extern ast_err NCSet_base_pe_Return_reclaim(ast_runtime*,NCSet_base_pe_Return*);
extern size_t NCSet_base_pe_Return_get_size(ast_runtime*,NCSet_base_pe_Return*);

struct NCInq_format {
    int32_t ncid;
};


extern ast_err NCInq_format_write(ast_runtime*,NCInq_format*);
extern ast_err NCInq_format_read(ast_runtime*,NCInq_format**);
extern ast_err NCInq_format_reclaim(ast_runtime*,NCInq_format*);
extern size_t NCInq_format_get_size(ast_runtime*,NCInq_format*);

struct NCInq_format_Return {
    int32_t ncstatus;
    int32_t format;
};


extern ast_err NCInq_format_Return_write(ast_runtime*,NCInq_format_Return*);
extern ast_err NCInq_format_Return_read(ast_runtime*,NCInq_format_Return**);
extern ast_err NCInq_format_Return_reclaim(ast_runtime*,NCInq_format_Return*);
extern size_t NCInq_format_Return_get_size(ast_runtime*,NCInq_format_Return*);

struct NCInq {
    int32_t ncid;
};


extern ast_err NCInq_write(ast_runtime*,NCInq*);
extern ast_err NCInq_read(ast_runtime*,NCInq**);
extern ast_err NCInq_reclaim(ast_runtime*,NCInq*);
extern size_t NCInq_get_size(ast_runtime*,NCInq*);

struct NCInq_Return {
    int32_t ncstatus;
    int32_t ndims;
    int32_t nvars;
    int32_t natts;
    int32_t unlimdimid;
};


extern ast_err NCInq_Return_write(ast_runtime*,NCInq_Return*);
extern ast_err NCInq_Return_read(ast_runtime*,NCInq_Return**);
extern ast_err NCInq_Return_reclaim(ast_runtime*,NCInq_Return*);
extern size_t NCInq_Return_get_size(ast_runtime*,NCInq_Return*);

struct NCInq_Type {
    int32_t ncid;
    int32_t xtype;
};


extern ast_err NCInq_Type_write(ast_runtime*,NCInq_Type*);
extern ast_err NCInq_Type_read(ast_runtime*,NCInq_Type**);
extern ast_err NCInq_Type_reclaim(ast_runtime*,NCInq_Type*);
extern size_t NCInq_Type_get_size(ast_runtime*,NCInq_Type*);

struct NCInq_Type_Return {
    int32_t ncstatus;
    char* name;
    uint64_t size;
};


extern ast_err NCInq_Type_Return_write(ast_runtime*,NCInq_Type_Return*);
extern ast_err NCInq_Type_Return_read(ast_runtime*,NCInq_Type_Return**);
extern ast_err NCInq_Type_Return_reclaim(ast_runtime*,NCInq_Type_Return*);
extern size_t NCInq_Type_Return_get_size(ast_runtime*,NCInq_Type_Return*);

struct NCDef_Dim {
    int32_t ncid;
    char* name;
    uint64_t len;
};


extern ast_err NCDef_Dim_write(ast_runtime*,NCDef_Dim*);
extern ast_err NCDef_Dim_read(ast_runtime*,NCDef_Dim**);
extern ast_err NCDef_Dim_reclaim(ast_runtime*,NCDef_Dim*);
extern size_t NCDef_Dim_get_size(ast_runtime*,NCDef_Dim*);

struct NCDef_Dim_Return {
    int32_t ncstatus;
    int32_t dimid;
};


extern ast_err NCDef_Dim_Return_write(ast_runtime*,NCDef_Dim_Return*);
extern ast_err NCDef_Dim_Return_read(ast_runtime*,NCDef_Dim_Return**);
extern ast_err NCDef_Dim_Return_reclaim(ast_runtime*,NCDef_Dim_Return*);
extern size_t NCDef_Dim_Return_get_size(ast_runtime*,NCDef_Dim_Return*);

struct NCInq_dimid {
    int32_t ncid;
    char* name;
};


extern ast_err NCInq_dimid_write(ast_runtime*,NCInq_dimid*);
extern ast_err NCInq_dimid_read(ast_runtime*,NCInq_dimid**);
extern ast_err NCInq_dimid_reclaim(ast_runtime*,NCInq_dimid*);
extern size_t NCInq_dimid_get_size(ast_runtime*,NCInq_dimid*);

struct NCInq_dimid_Return {
    int32_t ncstatus;
    int32_t dimid;
};


extern ast_err NCInq_dimid_Return_write(ast_runtime*,NCInq_dimid_Return*);
extern ast_err NCInq_dimid_Return_read(ast_runtime*,NCInq_dimid_Return**);
extern ast_err NCInq_dimid_Return_reclaim(ast_runtime*,NCInq_dimid_Return*);
extern size_t NCInq_dimid_Return_get_size(ast_runtime*,NCInq_dimid_Return*);

struct NCInq_dim {
    int32_t ncid;
    int32_t dimid;
};


extern ast_err NCInq_dim_write(ast_runtime*,NCInq_dim*);
extern ast_err NCInq_dim_read(ast_runtime*,NCInq_dim**);
extern ast_err NCInq_dim_reclaim(ast_runtime*,NCInq_dim*);
extern size_t NCInq_dim_get_size(ast_runtime*,NCInq_dim*);

struct NCInq_dim_Return {
    int32_t ncstatus;
    char* name;
    uint64_t len;
};


extern ast_err NCInq_dim_Return_write(ast_runtime*,NCInq_dim_Return*);
extern ast_err NCInq_dim_Return_read(ast_runtime*,NCInq_dim_Return**);
extern ast_err NCInq_dim_Return_reclaim(ast_runtime*,NCInq_dim_Return*);
extern size_t NCInq_dim_Return_get_size(ast_runtime*,NCInq_dim_Return*);

struct NCInq_unlimdim {
    int32_t ncid;
};


extern ast_err NCInq_unlimdim_write(ast_runtime*,NCInq_unlimdim*);
extern ast_err NCInq_unlimdim_read(ast_runtime*,NCInq_unlimdim**);
extern ast_err NCInq_unlimdim_reclaim(ast_runtime*,NCInq_unlimdim*);
extern size_t NCInq_unlimdim_get_size(ast_runtime*,NCInq_unlimdim*);

struct NCInq_unlimdim_Return {
    int32_t ncstatus;
    int32_t unlimdimid;
};


extern ast_err NCInq_unlimdim_Return_write(ast_runtime*,NCInq_unlimdim_Return*);
extern ast_err NCInq_unlimdim_Return_read(ast_runtime*,NCInq_unlimdim_Return**);
extern ast_err NCInq_unlimdim_Return_reclaim(ast_runtime*,NCInq_unlimdim_Return*);
extern size_t NCInq_unlimdim_Return_get_size(ast_runtime*,NCInq_unlimdim_Return*);

struct NCRename_dim {
    int32_t ncid;
    int32_t dimid;
    char* name;
};


extern ast_err NCRename_dim_write(ast_runtime*,NCRename_dim*);
extern ast_err NCRename_dim_read(ast_runtime*,NCRename_dim**);
extern ast_err NCRename_dim_reclaim(ast_runtime*,NCRename_dim*);
extern size_t NCRename_dim_get_size(ast_runtime*,NCRename_dim*);

struct NCRename_dim_Return {
    int32_t ncstatus;
};


extern ast_err NCRename_dim_Return_write(ast_runtime*,NCRename_dim_Return*);
extern ast_err NCRename_dim_Return_read(ast_runtime*,NCRename_dim_Return**);
extern ast_err NCRename_dim_Return_reclaim(ast_runtime*,NCRename_dim_Return*);
extern size_t NCRename_dim_Return_get_size(ast_runtime*,NCRename_dim_Return*);

struct NCInq_att {
    int32_t ncid;
    int32_t varid;
    char* name;
};


extern ast_err NCInq_att_write(ast_runtime*,NCInq_att*);
extern ast_err NCInq_att_read(ast_runtime*,NCInq_att**);
extern ast_err NCInq_att_reclaim(ast_runtime*,NCInq_att*);
extern size_t NCInq_att_get_size(ast_runtime*,NCInq_att*);

struct NCInq_att_Return {
    int32_t ncstatus;
    int32_t xtype;
    uint64_t len;
};


extern ast_err NCInq_att_Return_write(ast_runtime*,NCInq_att_Return*);
extern ast_err NCInq_att_Return_read(ast_runtime*,NCInq_att_Return**);
extern ast_err NCInq_att_Return_reclaim(ast_runtime*,NCInq_att_Return*);
extern size_t NCInq_att_Return_get_size(ast_runtime*,NCInq_att_Return*);

struct NCInq_attid {
    int32_t ncid;
    int32_t varid;
    char* name;
};


extern ast_err NCInq_attid_write(ast_runtime*,NCInq_attid*);
extern ast_err NCInq_attid_read(ast_runtime*,NCInq_attid**);
extern ast_err NCInq_attid_reclaim(ast_runtime*,NCInq_attid*);
extern size_t NCInq_attid_get_size(ast_runtime*,NCInq_attid*);

struct NCInq_attid_Return {
    int32_t ncstatus;
    int32_t attid;
};


extern ast_err NCInq_attid_Return_write(ast_runtime*,NCInq_attid_Return*);
extern ast_err NCInq_attid_Return_read(ast_runtime*,NCInq_attid_Return**);
extern ast_err NCInq_attid_Return_reclaim(ast_runtime*,NCInq_attid_Return*);
extern size_t NCInq_attid_Return_get_size(ast_runtime*,NCInq_attid_Return*);

struct NCInq_attname {
    int32_t ncid;
    int32_t varid;
    int32_t attnum;
};


extern ast_err NCInq_attname_write(ast_runtime*,NCInq_attname*);
extern ast_err NCInq_attname_read(ast_runtime*,NCInq_attname**);
extern ast_err NCInq_attname_reclaim(ast_runtime*,NCInq_attname*);
extern size_t NCInq_attname_get_size(ast_runtime*,NCInq_attname*);

struct NCInq_attname_Return {
    int32_t ncstatus;
    char* name;
};


extern ast_err NCInq_attname_Return_write(ast_runtime*,NCInq_attname_Return*);
extern ast_err NCInq_attname_Return_read(ast_runtime*,NCInq_attname_Return**);
extern ast_err NCInq_attname_Return_reclaim(ast_runtime*,NCInq_attname_Return*);
extern size_t NCInq_attname_Return_get_size(ast_runtime*,NCInq_attname_Return*);

struct NCRename_att {
    int32_t ncid;
    int32_t varid;
    char* name;
    char* newname;
};


extern ast_err NCRename_att_write(ast_runtime*,NCRename_att*);
extern ast_err NCRename_att_read(ast_runtime*,NCRename_att**);
extern ast_err NCRename_att_reclaim(ast_runtime*,NCRename_att*);
extern size_t NCRename_att_get_size(ast_runtime*,NCRename_att*);

struct NCRename_att_Return {
    int32_t ncstatus;
};


extern ast_err NCRename_att_Return_write(ast_runtime*,NCRename_att_Return*);
extern ast_err NCRename_att_Return_read(ast_runtime*,NCRename_att_Return**);
extern ast_err NCRename_att_Return_reclaim(ast_runtime*,NCRename_att_Return*);
extern size_t NCRename_att_Return_get_size(ast_runtime*,NCRename_att_Return*);

struct NCDel_att {
    int32_t ncid;
    int32_t varid;
    char* name;
};


extern ast_err NCDel_att_write(ast_runtime*,NCDel_att*);
extern ast_err NCDel_att_read(ast_runtime*,NCDel_att**);
extern ast_err NCDel_att_reclaim(ast_runtime*,NCDel_att*);
extern size_t NCDel_att_get_size(ast_runtime*,NCDel_att*);

struct NCDel_att_Return {
    int32_t ncstatus;
};


extern ast_err NCDel_att_Return_write(ast_runtime*,NCDel_att_Return*);
extern ast_err NCDel_att_Return_read(ast_runtime*,NCDel_att_Return**);
extern ast_err NCDel_att_Return_reclaim(ast_runtime*,NCDel_att_Return*);
extern size_t NCDel_att_Return_get_size(ast_runtime*,NCDel_att_Return*);

struct NCGet_att {
    int32_t ncid;
    int32_t varid;
    char* name;
    int32_t xtype;
};


extern ast_err NCGet_att_write(ast_runtime*,NCGet_att*);
extern ast_err NCGet_att_read(ast_runtime*,NCGet_att**);
extern ast_err NCGet_att_reclaim(ast_runtime*,NCGet_att*);
extern size_t NCGet_att_get_size(ast_runtime*,NCGet_att*);

struct NCGet_att_Return {
    int32_t ncstatus;
    bytes_t values;
};


extern ast_err NCGet_att_Return_write(ast_runtime*,NCGet_att_Return*);
extern ast_err NCGet_att_Return_read(ast_runtime*,NCGet_att_Return**);
extern ast_err NCGet_att_Return_reclaim(ast_runtime*,NCGet_att_Return*);
extern size_t NCGet_att_Return_get_size(ast_runtime*,NCGet_att_Return*);

struct NCPut_att {
    int32_t ncid;
    int32_t varid;
    char* name;
    int32_t vtype;
    uint64_t nelems;
    bytes_t value;
    int32_t atype;
};


extern ast_err NCPut_att_write(ast_runtime*,NCPut_att*);
extern ast_err NCPut_att_read(ast_runtime*,NCPut_att**);
extern ast_err NCPut_att_reclaim(ast_runtime*,NCPut_att*);
extern size_t NCPut_att_get_size(ast_runtime*,NCPut_att*);

struct NCPut_att_Return {
    int32_t ncstatus;
};


extern ast_err NCPut_att_Return_write(ast_runtime*,NCPut_att_Return*);
extern ast_err NCPut_att_Return_read(ast_runtime*,NCPut_att_Return**);
extern ast_err NCPut_att_Return_reclaim(ast_runtime*,NCPut_att_Return*);
extern size_t NCPut_att_Return_get_size(ast_runtime*,NCPut_att_Return*);

struct NCDef_Var {
    int32_t ncid;
    char* name;
    int32_t xtype;
    int32_t ndims;
    struct {size_t count; int32_t* values;} dimids;
};


extern ast_err NCDef_Var_write(ast_runtime*,NCDef_Var*);
extern ast_err NCDef_Var_read(ast_runtime*,NCDef_Var**);
extern ast_err NCDef_Var_reclaim(ast_runtime*,NCDef_Var*);
extern size_t NCDef_Var_get_size(ast_runtime*,NCDef_Var*);

struct NCDef_Var_Return {
    int32_t ncstatus;
    int32_t varid;
};


extern ast_err NCDef_Var_Return_write(ast_runtime*,NCDef_Var_Return*);
extern ast_err NCDef_Var_Return_read(ast_runtime*,NCDef_Var_Return**);
extern ast_err NCDef_Var_Return_reclaim(ast_runtime*,NCDef_Var_Return*);
extern size_t NCDef_Var_Return_get_size(ast_runtime*,NCDef_Var_Return*);

struct NCInq_varid {
    int32_t ncid;
    char* name;
};


extern ast_err NCInq_varid_write(ast_runtime*,NCInq_varid*);
extern ast_err NCInq_varid_read(ast_runtime*,NCInq_varid**);
extern ast_err NCInq_varid_reclaim(ast_runtime*,NCInq_varid*);
extern size_t NCInq_varid_get_size(ast_runtime*,NCInq_varid*);

struct NCInq_varid_Return {
    int32_t ncstatus;
    int32_t varid;
};


extern ast_err NCInq_varid_Return_write(ast_runtime*,NCInq_varid_Return*);
extern ast_err NCInq_varid_Return_read(ast_runtime*,NCInq_varid_Return**);
extern ast_err NCInq_varid_Return_reclaim(ast_runtime*,NCInq_varid_Return*);
extern size_t NCInq_varid_Return_get_size(ast_runtime*,NCInq_varid_Return*);

struct NCRename_var {
    int32_t ncid;
    int32_t varid;
    char* name;
};


extern ast_err NCRename_var_write(ast_runtime*,NCRename_var*);
extern ast_err NCRename_var_read(ast_runtime*,NCRename_var**);
extern ast_err NCRename_var_reclaim(ast_runtime*,NCRename_var*);
extern size_t NCRename_var_get_size(ast_runtime*,NCRename_var*);

struct NCRename_var_Return {
    int32_t ncstatus;
};


extern ast_err NCRename_var_Return_write(ast_runtime*,NCRename_var_Return*);
extern ast_err NCRename_var_Return_read(ast_runtime*,NCRename_var_Return**);
extern ast_err NCRename_var_Return_reclaim(ast_runtime*,NCRename_var_Return*);
extern size_t NCRename_var_Return_get_size(ast_runtime*,NCRename_var_Return*);

struct NCGet_vara {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    int32_t memtype;
};


extern ast_err NCGet_vara_write(ast_runtime*,NCGet_vara*);
extern ast_err NCGet_vara_read(ast_runtime*,NCGet_vara**);
extern ast_err NCGet_vara_reclaim(ast_runtime*,NCGet_vara*);
extern size_t NCGet_vara_get_size(ast_runtime*,NCGet_vara*);

struct NCGet_vara_Return {
    int32_t ncstatus;
    bytes_t value;
};


extern ast_err NCGet_vara_Return_write(ast_runtime*,NCGet_vara_Return*);
extern ast_err NCGet_vara_Return_read(ast_runtime*,NCGet_vara_Return**);
extern ast_err NCGet_vara_Return_reclaim(ast_runtime*,NCGet_vara_Return*);
extern size_t NCGet_vara_Return_get_size(ast_runtime*,NCGet_vara_Return*);

struct NCPut_vara {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    bytes_t value;
    int32_t memtype;
};


extern ast_err NCPut_vara_write(ast_runtime*,NCPut_vara*);
extern ast_err NCPut_vara_read(ast_runtime*,NCPut_vara**);
extern ast_err NCPut_vara_reclaim(ast_runtime*,NCPut_vara*);
extern size_t NCPut_vara_get_size(ast_runtime*,NCPut_vara*);

struct NCPut_vara_Return {
    int32_t ncstatus;
};


extern ast_err NCPut_vara_Return_write(ast_runtime*,NCPut_vara_Return*);
extern ast_err NCPut_vara_Return_read(ast_runtime*,NCPut_vara_Return**);
extern ast_err NCPut_vara_Return_reclaim(ast_runtime*,NCPut_vara_Return*);
extern size_t NCPut_vara_Return_get_size(ast_runtime*,NCPut_vara_Return*);

struct NCGet_vars {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    struct {size_t count; uint64_t* values;} stride;
    int32_t memtype;
};


extern ast_err NCGet_vars_write(ast_runtime*,NCGet_vars*);
extern ast_err NCGet_vars_read(ast_runtime*,NCGet_vars**);
extern ast_err NCGet_vars_reclaim(ast_runtime*,NCGet_vars*);
extern size_t NCGet_vars_get_size(ast_runtime*,NCGet_vars*);

struct NCGet_vars_Return {
    int32_t ncstatus;
    bytes_t value;
};


extern ast_err NCGet_vars_Return_write(ast_runtime*,NCGet_vars_Return*);
extern ast_err NCGet_vars_Return_read(ast_runtime*,NCGet_vars_Return**);
extern ast_err NCGet_vars_Return_reclaim(ast_runtime*,NCGet_vars_Return*);
extern size_t NCGet_vars_Return_get_size(ast_runtime*,NCGet_vars_Return*);

struct NCPut_vars {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    struct {size_t count; uint64_t* values;} stride;
    bytes_t value;
    int32_t memtype;
};


extern ast_err NCPut_vars_write(ast_runtime*,NCPut_vars*);
extern ast_err NCPut_vars_read(ast_runtime*,NCPut_vars**);
extern ast_err NCPut_vars_reclaim(ast_runtime*,NCPut_vars*);
extern size_t NCPut_vars_get_size(ast_runtime*,NCPut_vars*);

struct NCPut_vars_Return {
    int32_t ncstatus;
};


extern ast_err NCPut_vars_Return_write(ast_runtime*,NCPut_vars_Return*);
extern ast_err NCPut_vars_Return_read(ast_runtime*,NCPut_vars_Return**);
extern ast_err NCPut_vars_Return_reclaim(ast_runtime*,NCPut_vars_Return*);
extern size_t NCPut_vars_Return_get_size(ast_runtime*,NCPut_vars_Return*);

struct NCGet_varm {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    struct {size_t count; uint64_t* values;} stride;
    struct {size_t count; uint64_t* values;} imap;
    int32_t memtype;
};


extern ast_err NCGet_varm_write(ast_runtime*,NCGet_varm*);
extern ast_err NCGet_varm_read(ast_runtime*,NCGet_varm**);
extern ast_err NCGet_varm_reclaim(ast_runtime*,NCGet_varm*);
extern size_t NCGet_varm_get_size(ast_runtime*,NCGet_varm*);

struct NCGet_varm_Return {
    int32_t ncstatus;
    bytes_t value;
};


extern ast_err NCGet_varm_Return_write(ast_runtime*,NCGet_varm_Return*);
extern ast_err NCGet_varm_Return_read(ast_runtime*,NCGet_varm_Return**);
extern ast_err NCGet_varm_Return_reclaim(ast_runtime*,NCGet_varm_Return*);
extern size_t NCGet_varm_Return_get_size(ast_runtime*,NCGet_varm_Return*);

struct NCPut_varm {
    int32_t ncid;
    int32_t varid;
    struct {size_t count; uint64_t* values;} start;
    struct {size_t count; uint64_t* values;} edges;
    struct {size_t count; uint64_t* values;} stride;
    struct {size_t count; uint64_t* values;} imap;
    bytes_t value;
    int32_t memtype;
};


extern ast_err NCPut_varm_write(ast_runtime*,NCPut_varm*);
extern ast_err NCPut_varm_read(ast_runtime*,NCPut_varm**);
extern ast_err NCPut_varm_reclaim(ast_runtime*,NCPut_varm*);
extern size_t NCPut_varm_get_size(ast_runtime*,NCPut_varm*);

struct NCPut_varm_Return {
    int32_t ncstatus;
};


extern ast_err NCPut_varm_Return_write(ast_runtime*,NCPut_varm_Return*);
extern ast_err NCPut_varm_Return_read(ast_runtime*,NCPut_varm_Return**);
extern ast_err NCPut_varm_Return_reclaim(ast_runtime*,NCPut_varm_Return*);
extern size_t NCPut_varm_Return_get_size(ast_runtime*,NCPut_varm_Return*);

struct NCInq_var_all {
    int32_t ncid;
    int32_t varid;
    char* name;
};


extern ast_err NCInq_var_all_write(ast_runtime*,NCInq_var_all*);
extern ast_err NCInq_var_all_read(ast_runtime*,NCInq_var_all**);
extern ast_err NCInq_var_all_reclaim(ast_runtime*,NCInq_var_all*);
extern size_t NCInq_var_all_get_size(ast_runtime*,NCInq_var_all*);

struct NCInq_var_all_Return {
    int32_t ncstatus;
    int32_t xtype;
    int32_t ndims;
    struct {size_t count; int32_t* values;} dimids;
    int32_t natts;
    bool_t shuffle;
    bool_t deflate;
    int32_t deflate_level;
    bool_t fletcher32;
    bool_t contiguous;
    struct {size_t count; uint64_t* values;} chunksizes;
    bool_t no_fill;
    bytes_t fill_value;
    bool_t endianness;
    int32_t options_mask;
    int32_t pixels_per_block;
};


extern ast_err NCInq_var_all_Return_write(ast_runtime*,NCInq_var_all_Return*);
extern ast_err NCInq_var_all_Return_read(ast_runtime*,NCInq_var_all_Return**);
extern ast_err NCInq_var_all_Return_reclaim(ast_runtime*,NCInq_var_all_Return*);
extern size_t NCInq_var_all_Return_get_size(ast_runtime*,NCInq_var_all_Return*);

struct NCShow_metadata {
    int32_t ncid;
};


extern ast_err NCShow_metadata_write(ast_runtime*,NCShow_metadata*);
extern ast_err NCShow_metadata_read(ast_runtime*,NCShow_metadata**);
extern ast_err NCShow_metadata_reclaim(ast_runtime*,NCShow_metadata*);
extern size_t NCShow_metadata_get_size(ast_runtime*,NCShow_metadata*);

struct NCShow_metadata_Return {
    int32_t ncstatus;
};


extern ast_err NCShow_metadata_Return_write(ast_runtime*,NCShow_metadata_Return*);
extern ast_err NCShow_metadata_Return_read(ast_runtime*,NCShow_metadata_Return**);
extern ast_err NCShow_metadata_Return_reclaim(ast_runtime*,NCShow_metadata_Return*);
extern size_t NCShow_metadata_Return_get_size(ast_runtime*,NCShow_metadata_Return*);

struct NCInq_unlimdims {
    int32_t ncid;
};


extern ast_err NCInq_unlimdims_write(ast_runtime*,NCInq_unlimdims*);
extern ast_err NCInq_unlimdims_read(ast_runtime*,NCInq_unlimdims**);
extern ast_err NCInq_unlimdims_reclaim(ast_runtime*,NCInq_unlimdims*);
extern size_t NCInq_unlimdims_get_size(ast_runtime*,NCInq_unlimdims*);

struct NCInq_unlimdims_Return {
    int32_t ncstatus;
    int32_t nunlimdims;
    struct {size_t count; int32_t* values;} unlimdimids;
};


extern ast_err NCInq_unlimdims_Return_write(ast_runtime*,NCInq_unlimdims_Return*);
extern ast_err NCInq_unlimdims_Return_read(ast_runtime*,NCInq_unlimdims_Return**);
extern ast_err NCInq_unlimdims_Return_reclaim(ast_runtime*,NCInq_unlimdims_Return*);
extern size_t NCInq_unlimdims_Return_get_size(ast_runtime*,NCInq_unlimdims_Return*);

struct NCVar_par_access {
    int32_t ncid;
    int32_t varid;
    bool_t par_access;
};


extern ast_err NCVar_par_access_write(ast_runtime*,NCVar_par_access*);
extern ast_err NCVar_par_access_read(ast_runtime*,NCVar_par_access**);
extern ast_err NCVar_par_access_reclaim(ast_runtime*,NCVar_par_access*);
extern size_t NCVar_par_access_get_size(ast_runtime*,NCVar_par_access*);

struct NCVar_par_access_Return {
    int32_t ncstatus;
};


extern ast_err NCVar_par_access_Return_write(ast_runtime*,NCVar_par_access_Return*);
extern ast_err NCVar_par_access_Return_read(ast_runtime*,NCVar_par_access_Return**);
extern ast_err NCVar_par_access_Return_reclaim(ast_runtime*,NCVar_par_access_Return*);
extern size_t NCVar_par_access_Return_get_size(ast_runtime*,NCVar_par_access_Return*);

struct NCInq_ncid {
    int32_t ncid;
    char* group;
};


extern ast_err NCInq_ncid_write(ast_runtime*,NCInq_ncid*);
extern ast_err NCInq_ncid_read(ast_runtime*,NCInq_ncid**);
extern ast_err NCInq_ncid_reclaim(ast_runtime*,NCInq_ncid*);
extern size_t NCInq_ncid_get_size(ast_runtime*,NCInq_ncid*);

struct NCInq_ncid_Return {
    int32_t ncstatus;
    int32_t grp_ncid;
};


extern ast_err NCInq_ncid_Return_write(ast_runtime*,NCInq_ncid_Return*);
extern ast_err NCInq_ncid_Return_read(ast_runtime*,NCInq_ncid_Return**);
extern ast_err NCInq_ncid_Return_reclaim(ast_runtime*,NCInq_ncid_Return*);
extern size_t NCInq_ncid_Return_get_size(ast_runtime*,NCInq_ncid_Return*);

struct NCInq_grps {
    int32_t ncid;
};


extern ast_err NCInq_grps_write(ast_runtime*,NCInq_grps*);
extern ast_err NCInq_grps_read(ast_runtime*,NCInq_grps**);
extern ast_err NCInq_grps_reclaim(ast_runtime*,NCInq_grps*);
extern size_t NCInq_grps_get_size(ast_runtime*,NCInq_grps*);

struct NCInq_grps_Return {
    int32_t ncstatus;
    int32_t ngroups;
    struct {size_t count; int32_t* values;} ncids;
};


extern ast_err NCInq_grps_Return_write(ast_runtime*,NCInq_grps_Return*);
extern ast_err NCInq_grps_Return_read(ast_runtime*,NCInq_grps_Return**);
extern ast_err NCInq_grps_Return_reclaim(ast_runtime*,NCInq_grps_Return*);
extern size_t NCInq_grps_Return_get_size(ast_runtime*,NCInq_grps_Return*);

struct NCInq_grpname {
    int32_t ncid;
};


extern ast_err NCInq_grpname_write(ast_runtime*,NCInq_grpname*);
extern ast_err NCInq_grpname_read(ast_runtime*,NCInq_grpname**);
extern ast_err NCInq_grpname_reclaim(ast_runtime*,NCInq_grpname*);
extern size_t NCInq_grpname_get_size(ast_runtime*,NCInq_grpname*);

struct NCInq_grpname_Return {
    int32_t ncstatus;
    char* name;
};


extern ast_err NCInq_grpname_Return_write(ast_runtime*,NCInq_grpname_Return*);
extern ast_err NCInq_grpname_Return_read(ast_runtime*,NCInq_grpname_Return**);
extern ast_err NCInq_grpname_Return_reclaim(ast_runtime*,NCInq_grpname_Return*);
extern size_t NCInq_grpname_Return_get_size(ast_runtime*,NCInq_grpname_Return*);

struct NCInq_grpname_full {
    int32_t ncid;
};


extern ast_err NCInq_grpname_full_write(ast_runtime*,NCInq_grpname_full*);
extern ast_err NCInq_grpname_full_read(ast_runtime*,NCInq_grpname_full**);
extern ast_err NCInq_grpname_full_reclaim(ast_runtime*,NCInq_grpname_full*);
extern size_t NCInq_grpname_full_get_size(ast_runtime*,NCInq_grpname_full*);

struct NCInq_grpname_full_Return {
    int32_t ncstatus;
    struct {size_t count; uint64_t* values;} len;
    char* fullname;
};


extern ast_err NCInq_grpname_full_Return_write(ast_runtime*,NCInq_grpname_full_Return*);
extern ast_err NCInq_grpname_full_Return_read(ast_runtime*,NCInq_grpname_full_Return**);
extern ast_err NCInq_grpname_full_Return_reclaim(ast_runtime*,NCInq_grpname_full_Return*);
extern size_t NCInq_grpname_full_Return_get_size(ast_runtime*,NCInq_grpname_full_Return*);

struct NCInq_grp_parent {
    int32_t ncid;
};


extern ast_err NCInq_grp_parent_write(ast_runtime*,NCInq_grp_parent*);
extern ast_err NCInq_grp_parent_read(ast_runtime*,NCInq_grp_parent**);
extern ast_err NCInq_grp_parent_reclaim(ast_runtime*,NCInq_grp_parent*);
extern size_t NCInq_grp_parent_get_size(ast_runtime*,NCInq_grp_parent*);

struct NCInq_grp_parent_Return {
    int32_t ncstatus;
    int32_t parentncid;
};


extern ast_err NCInq_grp_parent_Return_write(ast_runtime*,NCInq_grp_parent_Return*);
extern ast_err NCInq_grp_parent_Return_read(ast_runtime*,NCInq_grp_parent_Return**);
extern ast_err NCInq_grp_parent_Return_reclaim(ast_runtime*,NCInq_grp_parent_Return*);
extern size_t NCInq_grp_parent_Return_get_size(ast_runtime*,NCInq_grp_parent_Return*);

struct NCInq_grp_full_ncid {
    int32_t ncid;
    char* fullname;
};


extern ast_err NCInq_grp_full_ncid_write(ast_runtime*,NCInq_grp_full_ncid*);
extern ast_err NCInq_grp_full_ncid_read(ast_runtime*,NCInq_grp_full_ncid**);
extern ast_err NCInq_grp_full_ncid_reclaim(ast_runtime*,NCInq_grp_full_ncid*);
extern size_t NCInq_grp_full_ncid_get_size(ast_runtime*,NCInq_grp_full_ncid*);

struct NCInq_grp_full_ncid_Return {
    int32_t ncstatus;
    int32_t groupncid;
};


extern ast_err NCInq_grp_full_ncid_Return_write(ast_runtime*,NCInq_grp_full_ncid_Return*);
extern ast_err NCInq_grp_full_ncid_Return_read(ast_runtime*,NCInq_grp_full_ncid_Return**);
extern ast_err NCInq_grp_full_ncid_Return_reclaim(ast_runtime*,NCInq_grp_full_ncid_Return*);
extern size_t NCInq_grp_full_ncid_Return_get_size(ast_runtime*,NCInq_grp_full_ncid_Return*);

struct NCInq_varids {
    int32_t ncid;
};


extern ast_err NCInq_varids_write(ast_runtime*,NCInq_varids*);
extern ast_err NCInq_varids_read(ast_runtime*,NCInq_varids**);
extern ast_err NCInq_varids_reclaim(ast_runtime*,NCInq_varids*);
extern size_t NCInq_varids_get_size(ast_runtime*,NCInq_varids*);

struct NCInq_varids_Return {
    int32_t ncstatus;
    int32_t nvars;
    struct {size_t count; int32_t* values;} varids;
};


extern ast_err NCInq_varids_Return_write(ast_runtime*,NCInq_varids_Return*);
extern ast_err NCInq_varids_Return_read(ast_runtime*,NCInq_varids_Return**);
extern ast_err NCInq_varids_Return_reclaim(ast_runtime*,NCInq_varids_Return*);
extern size_t NCInq_varids_Return_get_size(ast_runtime*,NCInq_varids_Return*);

struct NCInq_dimids {
    int32_t ncid;
    bool_t includeparents;
};


extern ast_err NCInq_dimids_write(ast_runtime*,NCInq_dimids*);
extern ast_err NCInq_dimids_read(ast_runtime*,NCInq_dimids**);
extern ast_err NCInq_dimids_reclaim(ast_runtime*,NCInq_dimids*);
extern size_t NCInq_dimids_get_size(ast_runtime*,NCInq_dimids*);

struct NCInq_dimids_Return {
    int32_t ncstatus;
    int32_t ndims;
    struct {size_t count; int32_t* values;} dimids;
};


extern ast_err NCInq_dimids_Return_write(ast_runtime*,NCInq_dimids_Return*);
extern ast_err NCInq_dimids_Return_read(ast_runtime*,NCInq_dimids_Return**);
extern ast_err NCInq_dimids_Return_reclaim(ast_runtime*,NCInq_dimids_Return*);
extern size_t NCInq_dimids_Return_get_size(ast_runtime*,NCInq_dimids_Return*);

struct NCInq_typeids {
    int32_t ncid;
};


extern ast_err NCInq_typeids_write(ast_runtime*,NCInq_typeids*);
extern ast_err NCInq_typeids_read(ast_runtime*,NCInq_typeids**);
extern ast_err NCInq_typeids_reclaim(ast_runtime*,NCInq_typeids*);
extern size_t NCInq_typeids_get_size(ast_runtime*,NCInq_typeids*);

struct NCInq_typeids_Return {
    int32_t ncstatus;
    int32_t ntypes;
    struct {size_t count; int32_t* values;} typeids;
};


extern ast_err NCInq_typeids_Return_write(ast_runtime*,NCInq_typeids_Return*);
extern ast_err NCInq_typeids_Return_read(ast_runtime*,NCInq_typeids_Return**);
extern ast_err NCInq_typeids_Return_reclaim(ast_runtime*,NCInq_typeids_Return*);
extern size_t NCInq_typeids_Return_get_size(ast_runtime*,NCInq_typeids_Return*);

struct NCInq_type_equal {
    int32_t ncid1;
    int32_t typeid1;
    int32_t ncid2;
    int32_t typeid2;
};


extern ast_err NCInq_type_equal_write(ast_runtime*,NCInq_type_equal*);
extern ast_err NCInq_type_equal_read(ast_runtime*,NCInq_type_equal**);
extern ast_err NCInq_type_equal_reclaim(ast_runtime*,NCInq_type_equal*);
extern size_t NCInq_type_equal_get_size(ast_runtime*,NCInq_type_equal*);

struct NCInq_type_equal_Return {
    int32_t ncstatus;
    bool_t equal;
};


extern ast_err NCInq_type_equal_Return_write(ast_runtime*,NCInq_type_equal_Return*);
extern ast_err NCInq_type_equal_Return_read(ast_runtime*,NCInq_type_equal_Return**);
extern ast_err NCInq_type_equal_Return_reclaim(ast_runtime*,NCInq_type_equal_Return*);
extern size_t NCInq_type_equal_Return_get_size(ast_runtime*,NCInq_type_equal_Return*);

struct NCDef_Grp {
    int32_t ncid;
    char* name;
};


extern ast_err NCDef_Grp_write(ast_runtime*,NCDef_Grp*);
extern ast_err NCDef_Grp_read(ast_runtime*,NCDef_Grp**);
extern ast_err NCDef_Grp_reclaim(ast_runtime*,NCDef_Grp*);
extern size_t NCDef_Grp_get_size(ast_runtime*,NCDef_Grp*);

struct NCDef_Grp_Return {
    int32_t ncstatus;
    int32_t grpncid;
};


extern ast_err NCDef_Grp_Return_write(ast_runtime*,NCDef_Grp_Return*);
extern ast_err NCDef_Grp_Return_read(ast_runtime*,NCDef_Grp_Return**);
extern ast_err NCDef_Grp_Return_reclaim(ast_runtime*,NCDef_Grp_Return*);
extern size_t NCDef_Grp_Return_get_size(ast_runtime*,NCDef_Grp_Return*);

struct NCInq_user_type {
    int32_t ncid;
    int32_t typeid;
};


extern ast_err NCInq_user_type_write(ast_runtime*,NCInq_user_type*);
extern ast_err NCInq_user_type_read(ast_runtime*,NCInq_user_type**);
extern ast_err NCInq_user_type_reclaim(ast_runtime*,NCInq_user_type*);
extern size_t NCInq_user_type_get_size(ast_runtime*,NCInq_user_type*);

struct NCInq_user_type_Return {
    int32_t ncstatus;
    char* name;
    uint64_t size;
    int32_t basetype;
    uint64_t nfields;
    int32_t typeclass;
};


extern ast_err NCInq_user_type_Return_write(ast_runtime*,NCInq_user_type_Return*);
extern ast_err NCInq_user_type_Return_read(ast_runtime*,NCInq_user_type_Return**);
extern ast_err NCInq_user_type_Return_reclaim(ast_runtime*,NCInq_user_type_Return*);
extern size_t NCInq_user_type_Return_get_size(ast_runtime*,NCInq_user_type_Return*);

struct NCInq_typeid {
    int32_t ncid;
    char* name;
};


extern ast_err NCInq_typeid_write(ast_runtime*,NCInq_typeid*);
extern ast_err NCInq_typeid_read(ast_runtime*,NCInq_typeid**);
extern ast_err NCInq_typeid_reclaim(ast_runtime*,NCInq_typeid*);
extern size_t NCInq_typeid_get_size(ast_runtime*,NCInq_typeid*);

struct NCInq_typeid_Return {
    int32_t ncstatus;
    int32_t typeid;
};


extern ast_err NCInq_typeid_Return_write(ast_runtime*,NCInq_typeid_Return*);
extern ast_err NCInq_typeid_Return_read(ast_runtime*,NCInq_typeid_Return**);
extern ast_err NCInq_typeid_Return_reclaim(ast_runtime*,NCInq_typeid_Return*);
extern size_t NCInq_typeid_Return_get_size(ast_runtime*,NCInq_typeid_Return*);

struct NCDef_Compound {
    int32_t ncid;
    uint64_t size;
    char* name;
};


extern ast_err NCDef_Compound_write(ast_runtime*,NCDef_Compound*);
extern ast_err NCDef_Compound_read(ast_runtime*,NCDef_Compound**);
extern ast_err NCDef_Compound_reclaim(ast_runtime*,NCDef_Compound*);
extern size_t NCDef_Compound_get_size(ast_runtime*,NCDef_Compound*);

struct NCDef_Compound_Return {
    int32_t ncstatus;
    int32_t typeid;
};


extern ast_err NCDef_Compound_Return_write(ast_runtime*,NCDef_Compound_Return*);
extern ast_err NCDef_Compound_Return_read(ast_runtime*,NCDef_Compound_Return**);
extern ast_err NCDef_Compound_Return_reclaim(ast_runtime*,NCDef_Compound_Return*);
extern size_t NCDef_Compound_Return_get_size(ast_runtime*,NCDef_Compound_Return*);

struct NCInsert_compound {
    int32_t ncid;
    int32_t typeid;
    char* name;
    uint64_t offset;
    int32_t fieldtypeid;
};


extern ast_err NCInsert_compound_write(ast_runtime*,NCInsert_compound*);
extern ast_err NCInsert_compound_read(ast_runtime*,NCInsert_compound**);
extern ast_err NCInsert_compound_reclaim(ast_runtime*,NCInsert_compound*);
extern size_t NCInsert_compound_get_size(ast_runtime*,NCInsert_compound*);

struct NCInsert_compound_Return {
    int32_t ncstatus;
};


extern ast_err NCInsert_compound_Return_write(ast_runtime*,NCInsert_compound_Return*);
extern ast_err NCInsert_compound_Return_read(ast_runtime*,NCInsert_compound_Return**);
extern ast_err NCInsert_compound_Return_reclaim(ast_runtime*,NCInsert_compound_Return*);
extern size_t NCInsert_compound_Return_get_size(ast_runtime*,NCInsert_compound_Return*);

struct NCInsert_array_compound {
    int32_t ncid;
    int32_t typeid;
    char* name;
    uint64_t offset;
    int32_t fieldtypeid;
    int32_t ndims;
    struct {size_t count; int32_t* values;} dimsizes;
};


extern ast_err NCInsert_array_compound_write(ast_runtime*,NCInsert_array_compound*);
extern ast_err NCInsert_array_compound_read(ast_runtime*,NCInsert_array_compound**);
extern ast_err NCInsert_array_compound_reclaim(ast_runtime*,NCInsert_array_compound*);
extern size_t NCInsert_array_compound_get_size(ast_runtime*,NCInsert_array_compound*);

struct NCInsert_array_compound_Return {
    int32_t ncstatus;
};


extern ast_err NCInsert_array_compound_Return_write(ast_runtime*,NCInsert_array_compound_Return*);
extern ast_err NCInsert_array_compound_Return_read(ast_runtime*,NCInsert_array_compound_Return**);
extern ast_err NCInsert_array_compound_Return_reclaim(ast_runtime*,NCInsert_array_compound_Return*);
extern size_t NCInsert_array_compound_Return_get_size(ast_runtime*,NCInsert_array_compound_Return*);

struct NCInq_compound_field {
    int32_t ncid;
    int32_t typeid;
    int32_t fieldid;
};


extern ast_err NCInq_compound_field_write(ast_runtime*,NCInq_compound_field*);
extern ast_err NCInq_compound_field_read(ast_runtime*,NCInq_compound_field**);
extern ast_err NCInq_compound_field_reclaim(ast_runtime*,NCInq_compound_field*);
extern size_t NCInq_compound_field_get_size(ast_runtime*,NCInq_compound_field*);

struct NCInq_compound_field_Return {
    int32_t ncstatus;
    char* name;
    uint64_t offset;
    int32_t fieldtypeid;
    int32_t ndims;
    struct {size_t count; int32_t* values;} dimsizes;
};


extern ast_err NCInq_compound_field_Return_write(ast_runtime*,NCInq_compound_field_Return*);
extern ast_err NCInq_compound_field_Return_read(ast_runtime*,NCInq_compound_field_Return**);
extern ast_err NCInq_compound_field_Return_reclaim(ast_runtime*,NCInq_compound_field_Return*);
extern size_t NCInq_compound_field_Return_get_size(ast_runtime*,NCInq_compound_field_Return*);

struct NCInq_compound_fieldindex {
    int32_t ncid;
    int32_t typeid;
    char* name;
};


extern ast_err NCInq_compound_fieldindex_write(ast_runtime*,NCInq_compound_fieldindex*);
extern ast_err NCInq_compound_fieldindex_read(ast_runtime*,NCInq_compound_fieldindex**);
extern ast_err NCInq_compound_fieldindex_reclaim(ast_runtime*,NCInq_compound_fieldindex*);
extern size_t NCInq_compound_fieldindex_get_size(ast_runtime*,NCInq_compound_fieldindex*);

struct NCInq_compound_fieldindex_Return {
    int32_t ncstatus;
    int32_t fieldid;
};


extern ast_err NCInq_compound_fieldindex_Return_write(ast_runtime*,NCInq_compound_fieldindex_Return*);
extern ast_err NCInq_compound_fieldindex_Return_read(ast_runtime*,NCInq_compound_fieldindex_Return**);
extern ast_err NCInq_compound_fieldindex_Return_reclaim(ast_runtime*,NCInq_compound_fieldindex_Return*);
extern size_t NCInq_compound_fieldindex_Return_get_size(ast_runtime*,NCInq_compound_fieldindex_Return*);

struct NCDef_Vlen {
    int32_t ncid;
    char* name;
    int32_t base_typeid;
};


extern ast_err NCDef_Vlen_write(ast_runtime*,NCDef_Vlen*);
extern ast_err NCDef_Vlen_read(ast_runtime*,NCDef_Vlen**);
extern ast_err NCDef_Vlen_reclaim(ast_runtime*,NCDef_Vlen*);
extern size_t NCDef_Vlen_get_size(ast_runtime*,NCDef_Vlen*);

struct NCDef_Vlen_Return {
    int32_t ncstatus;
    int32_t typeid;
};


extern ast_err NCDef_Vlen_Return_write(ast_runtime*,NCDef_Vlen_Return*);
extern ast_err NCDef_Vlen_Return_read(ast_runtime*,NCDef_Vlen_Return**);
extern ast_err NCDef_Vlen_Return_reclaim(ast_runtime*,NCDef_Vlen_Return*);
extern size_t NCDef_Vlen_Return_get_size(ast_runtime*,NCDef_Vlen_Return*);

struct NCPut_vlen_element {
    int32_t ncid;
    int32_t typeid;
    bytes_t element;
    uint64_t len;
    bytes_t data;
};


extern ast_err NCPut_vlen_element_write(ast_runtime*,NCPut_vlen_element*);
extern ast_err NCPut_vlen_element_read(ast_runtime*,NCPut_vlen_element**);
extern ast_err NCPut_vlen_element_reclaim(ast_runtime*,NCPut_vlen_element*);
extern size_t NCPut_vlen_element_get_size(ast_runtime*,NCPut_vlen_element*);

struct NCPut_vlen_element_Return {
    int32_t ncstatus;
};


extern ast_err NCPut_vlen_element_Return_write(ast_runtime*,NCPut_vlen_element_Return*);
extern ast_err NCPut_vlen_element_Return_read(ast_runtime*,NCPut_vlen_element_Return**);
extern ast_err NCPut_vlen_element_Return_reclaim(ast_runtime*,NCPut_vlen_element_Return*);
extern size_t NCPut_vlen_element_Return_get_size(ast_runtime*,NCPut_vlen_element_Return*);

struct NCGet_vlen_element {
    int32_t ncid;
    int32_t typeid;
};


extern ast_err NCGet_vlen_element_write(ast_runtime*,NCGet_vlen_element*);
extern ast_err NCGet_vlen_element_read(ast_runtime*,NCGet_vlen_element**);
extern ast_err NCGet_vlen_element_reclaim(ast_runtime*,NCGet_vlen_element*);
extern size_t NCGet_vlen_element_get_size(ast_runtime*,NCGet_vlen_element*);

struct NCGet_vlen_element_Return {
    int32_t ncstatus;
    bytes_t element;
    uint64_t len;
    bytes_t data;
};


extern ast_err NCGet_vlen_element_Return_write(ast_runtime*,NCGet_vlen_element_Return*);
extern ast_err NCGet_vlen_element_Return_read(ast_runtime*,NCGet_vlen_element_Return**);
extern ast_err NCGet_vlen_element_Return_reclaim(ast_runtime*,NCGet_vlen_element_Return*);
extern size_t NCGet_vlen_element_Return_get_size(ast_runtime*,NCGet_vlen_element_Return*);

struct NCDef_Enum {
    int32_t ncid;
    int32_t basetypeid;
    char* name;
};


extern ast_err NCDef_Enum_write(ast_runtime*,NCDef_Enum*);
extern ast_err NCDef_Enum_read(ast_runtime*,NCDef_Enum**);
extern ast_err NCDef_Enum_reclaim(ast_runtime*,NCDef_Enum*);
extern size_t NCDef_Enum_get_size(ast_runtime*,NCDef_Enum*);

struct NCDef_Enum_Return {
    int32_t ncstatus;
    int32_t typeid;
};


extern ast_err NCDef_Enum_Return_write(ast_runtime*,NCDef_Enum_Return*);
extern ast_err NCDef_Enum_Return_read(ast_runtime*,NCDef_Enum_Return**);
extern ast_err NCDef_Enum_Return_reclaim(ast_runtime*,NCDef_Enum_Return*);
extern size_t NCDef_Enum_Return_get_size(ast_runtime*,NCDef_Enum_Return*);

struct NCInsert_enum {
    int32_t ncid;
    int32_t typeid;
    char* name;
    bytes_t value;
};


extern ast_err NCInsert_enum_write(ast_runtime*,NCInsert_enum*);
extern ast_err NCInsert_enum_read(ast_runtime*,NCInsert_enum**);
extern ast_err NCInsert_enum_reclaim(ast_runtime*,NCInsert_enum*);
extern size_t NCInsert_enum_get_size(ast_runtime*,NCInsert_enum*);

struct NCInsert_enum_Return {
    int32_t ncstatus;
};


extern ast_err NCInsert_enum_Return_write(ast_runtime*,NCInsert_enum_Return*);
extern ast_err NCInsert_enum_Return_read(ast_runtime*,NCInsert_enum_Return**);
extern ast_err NCInsert_enum_Return_reclaim(ast_runtime*,NCInsert_enum_Return*);
extern size_t NCInsert_enum_Return_get_size(ast_runtime*,NCInsert_enum_Return*);

struct NCInq_enum_member {
    int32_t ncid;
    int32_t typeid;
    int32_t index;
};


extern ast_err NCInq_enum_member_write(ast_runtime*,NCInq_enum_member*);
extern ast_err NCInq_enum_member_read(ast_runtime*,NCInq_enum_member**);
extern ast_err NCInq_enum_member_reclaim(ast_runtime*,NCInq_enum_member*);
extern size_t NCInq_enum_member_get_size(ast_runtime*,NCInq_enum_member*);

struct NCInq_enum_member_Return {
    int32_t ncstatus;
    char* name;
    bytes_t value;
};


extern ast_err NCInq_enum_member_Return_write(ast_runtime*,NCInq_enum_member_Return*);
extern ast_err NCInq_enum_member_Return_read(ast_runtime*,NCInq_enum_member_Return**);
extern ast_err NCInq_enum_member_Return_reclaim(ast_runtime*,NCInq_enum_member_Return*);
extern size_t NCInq_enum_member_Return_get_size(ast_runtime*,NCInq_enum_member_Return*);

struct NCInq_enum_ident {
    int32_t ncid;
    int32_t typeid;
    uint64_t value;
};


extern ast_err NCInq_enum_ident_write(ast_runtime*,NCInq_enum_ident*);
extern ast_err NCInq_enum_ident_read(ast_runtime*,NCInq_enum_ident**);
extern ast_err NCInq_enum_ident_reclaim(ast_runtime*,NCInq_enum_ident*);
extern size_t NCInq_enum_ident_get_size(ast_runtime*,NCInq_enum_ident*);

struct NCInq_enum_ident_Return {
    int32_t ncstatus;
    char* name;
};


extern ast_err NCInq_enum_ident_Return_write(ast_runtime*,NCInq_enum_ident_Return*);
extern ast_err NCInq_enum_ident_Return_read(ast_runtime*,NCInq_enum_ident_Return**);
extern ast_err NCInq_enum_ident_Return_reclaim(ast_runtime*,NCInq_enum_ident_Return*);
extern size_t NCInq_enum_ident_Return_get_size(ast_runtime*,NCInq_enum_ident_Return*);

struct NCDef_Opaque {
    int32_t ncid;
    uint64_t size;
    char* name;
};


extern ast_err NCDef_Opaque_write(ast_runtime*,NCDef_Opaque*);
extern ast_err NCDef_Opaque_read(ast_runtime*,NCDef_Opaque**);
extern ast_err NCDef_Opaque_reclaim(ast_runtime*,NCDef_Opaque*);
extern size_t NCDef_Opaque_get_size(ast_runtime*,NCDef_Opaque*);

struct NCDef_Opaque_Return {
    int32_t ncstatus;
    int32_t typeid;
};


extern ast_err NCDef_Opaque_Return_write(ast_runtime*,NCDef_Opaque_Return*);
extern ast_err NCDef_Opaque_Return_read(ast_runtime*,NCDef_Opaque_Return**);
extern ast_err NCDef_Opaque_Return_reclaim(ast_runtime*,NCDef_Opaque_Return*);
extern size_t NCDef_Opaque_Return_get_size(ast_runtime*,NCDef_Opaque_Return*);

struct NCDef_var_deflate {
    int32_t ncid;
    int32_t varid;
    bool_t shuffle;
    bool_t deflate;
    int32_t deflatelevel;
};


extern ast_err NCDef_var_deflate_write(ast_runtime*,NCDef_var_deflate*);
extern ast_err NCDef_var_deflate_read(ast_runtime*,NCDef_var_deflate**);
extern ast_err NCDef_var_deflate_reclaim(ast_runtime*,NCDef_var_deflate*);
extern size_t NCDef_var_deflate_get_size(ast_runtime*,NCDef_var_deflate*);

struct NCDef_var_deflate_Return {
    int32_t ncstatus;
};


extern ast_err NCDef_var_deflate_Return_write(ast_runtime*,NCDef_var_deflate_Return*);
extern ast_err NCDef_var_deflate_Return_read(ast_runtime*,NCDef_var_deflate_Return**);
extern ast_err NCDef_var_deflate_Return_reclaim(ast_runtime*,NCDef_var_deflate_Return*);
extern size_t NCDef_var_deflate_Return_get_size(ast_runtime*,NCDef_var_deflate_Return*);

struct NCDef_Var_Fletcher32 {
    int32_t ncid;
    int32_t varid;
    bool_t fletcher32;
};


extern ast_err NCDef_Var_Fletcher32_write(ast_runtime*,NCDef_Var_Fletcher32*);
extern ast_err NCDef_Var_Fletcher32_read(ast_runtime*,NCDef_Var_Fletcher32**);
extern ast_err NCDef_Var_Fletcher32_reclaim(ast_runtime*,NCDef_Var_Fletcher32*);
extern size_t NCDef_Var_Fletcher32_get_size(ast_runtime*,NCDef_Var_Fletcher32*);

struct NCDef_Var_Fletcher32_Return {
    int32_t ncstatus;
};


extern ast_err NCDef_Var_Fletcher32_Return_write(ast_runtime*,NCDef_Var_Fletcher32_Return*);
extern ast_err NCDef_Var_Fletcher32_Return_read(ast_runtime*,NCDef_Var_Fletcher32_Return**);
extern ast_err NCDef_Var_Fletcher32_Return_reclaim(ast_runtime*,NCDef_Var_Fletcher32_Return*);
extern size_t NCDef_Var_Fletcher32_Return_get_size(ast_runtime*,NCDef_Var_Fletcher32_Return*);

struct NCDef_Var_Chunking {
    int32_t ncid;
    int32_t varid;
    bool_t contiguous;
    struct {size_t count; uint64_t* values;} chunksizes;
};


extern ast_err NCDef_Var_Chunking_write(ast_runtime*,NCDef_Var_Chunking*);
extern ast_err NCDef_Var_Chunking_read(ast_runtime*,NCDef_Var_Chunking**);
extern ast_err NCDef_Var_Chunking_reclaim(ast_runtime*,NCDef_Var_Chunking*);
extern size_t NCDef_Var_Chunking_get_size(ast_runtime*,NCDef_Var_Chunking*);

struct NCDef_Var_Chunking_Return {
    int32_t ncstatus;
};


extern ast_err NCDef_Var_Chunking_Return_write(ast_runtime*,NCDef_Var_Chunking_Return*);
extern ast_err NCDef_Var_Chunking_Return_read(ast_runtime*,NCDef_Var_Chunking_Return**);
extern ast_err NCDef_Var_Chunking_Return_reclaim(ast_runtime*,NCDef_Var_Chunking_Return*);
extern size_t NCDef_Var_Chunking_Return_get_size(ast_runtime*,NCDef_Var_Chunking_Return*);

struct NCDef_Var_Fill {
    int32_t ncid;
    int32_t varid;
    bool_t nofill;
    bytes_t fill_value;
};


extern ast_err NCDef_Var_Fill_write(ast_runtime*,NCDef_Var_Fill*);
extern ast_err NCDef_Var_Fill_read(ast_runtime*,NCDef_Var_Fill**);
extern ast_err NCDef_Var_Fill_reclaim(ast_runtime*,NCDef_Var_Fill*);
extern size_t NCDef_Var_Fill_get_size(ast_runtime*,NCDef_Var_Fill*);

struct NCDef_Var_Fill_Return {
    int32_t ncstatus;
};


extern ast_err NCDef_Var_Fill_Return_write(ast_runtime*,NCDef_Var_Fill_Return*);
extern ast_err NCDef_Var_Fill_Return_read(ast_runtime*,NCDef_Var_Fill_Return**);
extern ast_err NCDef_Var_Fill_Return_reclaim(ast_runtime*,NCDef_Var_Fill_Return*);
extern size_t NCDef_Var_Fill_Return_get_size(ast_runtime*,NCDef_Var_Fill_Return*);

struct NCDef_Var_endian {
    int32_t ncid;
    int32_t varid;
    bool_t bigendian;
};


extern ast_err NCDef_Var_endian_write(ast_runtime*,NCDef_Var_endian*);
extern ast_err NCDef_Var_endian_read(ast_runtime*,NCDef_Var_endian**);
extern ast_err NCDef_Var_endian_reclaim(ast_runtime*,NCDef_Var_endian*);
extern size_t NCDef_Var_endian_get_size(ast_runtime*,NCDef_Var_endian*);

struct NCDef_Var_endian_Return {
    int32_t ncstatus;
};


extern ast_err NCDef_Var_endian_Return_write(ast_runtime*,NCDef_Var_endian_Return*);
extern ast_err NCDef_Var_endian_Return_read(ast_runtime*,NCDef_Var_endian_Return**);
extern ast_err NCDef_Var_endian_Return_reclaim(ast_runtime*,NCDef_Var_endian_Return*);
extern size_t NCDef_Var_endian_Return_get_size(ast_runtime*,NCDef_Var_endian_Return*);

struct NCSet_var_chunk_cache {
    int32_t ncid;
    int32_t varid;
    uint64_t size;
    uint64_t nelems;
    float preemption;
};


extern ast_err NCSet_var_chunk_cache_write(ast_runtime*,NCSet_var_chunk_cache*);
extern ast_err NCSet_var_chunk_cache_read(ast_runtime*,NCSet_var_chunk_cache**);
extern ast_err NCSet_var_chunk_cache_reclaim(ast_runtime*,NCSet_var_chunk_cache*);
extern size_t NCSet_var_chunk_cache_get_size(ast_runtime*,NCSet_var_chunk_cache*);

struct NCSet_var_chunk_cache_Return {
    int32_t ncstatus;
};


extern ast_err NCSet_var_chunk_cache_Return_write(ast_runtime*,NCSet_var_chunk_cache_Return*);
extern ast_err NCSet_var_chunk_cache_Return_read(ast_runtime*,NCSet_var_chunk_cache_Return**);
extern ast_err NCSet_var_chunk_cache_Return_reclaim(ast_runtime*,NCSet_var_chunk_cache_Return*);
extern size_t NCSet_var_chunk_cache_Return_get_size(ast_runtime*,NCSet_var_chunk_cache_Return*);

struct NCGet_var_chunk_cache {
    int32_t ncid;
    int32_t varid;
};


extern ast_err NCGet_var_chunk_cache_write(ast_runtime*,NCGet_var_chunk_cache*);
extern ast_err NCGet_var_chunk_cache_read(ast_runtime*,NCGet_var_chunk_cache**);
extern ast_err NCGet_var_chunk_cache_reclaim(ast_runtime*,NCGet_var_chunk_cache*);
extern size_t NCGet_var_chunk_cache_get_size(ast_runtime*,NCGet_var_chunk_cache*);

struct NCGet_var_chunk_cache_Return {
    int32_t ncstatus;
    uint64_t size;
    uint64_t nelems;
    float preemption;
};


extern ast_err NCGet_var_chunk_cache_Return_write(ast_runtime*,NCGet_var_chunk_cache_Return*);
extern ast_err NCGet_var_chunk_cache_Return_read(ast_runtime*,NCGet_var_chunk_cache_Return**);
extern ast_err NCGet_var_chunk_cache_Return_reclaim(ast_runtime*,NCGet_var_chunk_cache_Return*);
extern size_t NCGet_var_chunk_cache_Return_get_size(ast_runtime*,NCGet_var_chunk_cache_Return*);

struct NCNC_set_log_level {
    int32_t newlevel;
};


extern ast_err NCNC_set_log_level_write(ast_runtime*,NCNC_set_log_level*);
extern ast_err NCNC_set_log_level_read(ast_runtime*,NCNC_set_log_level**);
extern ast_err NCNC_set_log_level_reclaim(ast_runtime*,NCNC_set_log_level*);
extern size_t NCNC_set_log_level_get_size(ast_runtime*,NCNC_set_log_level*);

struct NCNC_set_log_level_Return {
    int32_t ncstatus;
};


extern ast_err NCNC_set_log_level_Return_write(ast_runtime*,NCNC_set_log_level_Return*);
extern ast_err NCNC_set_log_level_Return_read(ast_runtime*,NCNC_set_log_level_Return**);
extern ast_err NCNC_set_log_level_Return_reclaim(ast_runtime*,NCNC_set_log_level_Return*);
extern size_t NCNC_set_log_level_Return_get_size(ast_runtime*,NCNC_set_log_level_Return*);

struct NCNC_inq_libvers {
};


extern ast_err NCNC_inq_libvers_write(ast_runtime*,NCNC_inq_libvers*);
extern ast_err NCNC_inq_libvers_read(ast_runtime*,NCNC_inq_libvers**);
extern ast_err NCNC_inq_libvers_reclaim(ast_runtime*,NCNC_inq_libvers*);
extern size_t NCNC_inq_libvers_get_size(ast_runtime*,NCNC_inq_libvers*);

struct NCNC_inq_libvers_Return {
    char* version;
};


extern ast_err NCNC_inq_libvers_Return_write(ast_runtime*,NCNC_inq_libvers_Return*);
extern ast_err NCNC_inq_libvers_Return_read(ast_runtime*,NCNC_inq_libvers_Return**);
extern ast_err NCNC_inq_libvers_Return_reclaim(ast_runtime*,NCNC_inq_libvers_Return*);
extern size_t NCNC_inq_libvers_Return_get_size(ast_runtime*,NCNC_inq_libvers_Return*);

struct NCNC_delete_mp {
    char* path;
    int32_t basepe;
};


extern ast_err NCNC_delete_mp_write(ast_runtime*,NCNC_delete_mp*);
extern ast_err NCNC_delete_mp_read(ast_runtime*,NCNC_delete_mp**);
extern ast_err NCNC_delete_mp_reclaim(ast_runtime*,NCNC_delete_mp*);
extern size_t NCNC_delete_mp_get_size(ast_runtime*,NCNC_delete_mp*);

struct NCNC_delete_mp_Return {
    int32_t ncstatus;
};


extern ast_err NCNC_delete_mp_Return_write(ast_runtime*,NCNC_delete_mp_Return*);
extern ast_err NCNC_delete_mp_Return_read(ast_runtime*,NCNC_delete_mp_Return**);
extern ast_err NCNC_delete_mp_Return_reclaim(ast_runtime*,NCNC_delete_mp_Return*);
extern size_t NCNC_delete_mp_Return_get_size(ast_runtime*,NCNC_delete_mp_Return*);

struct MetaNode {
    MetaNode* root;
    nc_meta nodeclass;
    nc_meta subclass;
    struct {int defined; int32_t value;} ncid;
    struct {int defined; int32_t value;} typeid;
    struct {int defined; char* value;} name;
    struct {int defined; uint64_t value;} size;
    struct {int defined; MetaNode* value;} basetype;
    struct {int defined; MetaGraph* value;} graph;
    struct {int defined; MetaGroup* value;} group;
    struct {int defined; MetaVar* value;} var;
    struct {int defined; MetaDim* value;} dim;
    struct {int defined; MetaCompound* value;} compound_t;
    struct {int defined; MetaEnum* value;} enum_t;
};


extern ast_err MetaNode_write(ast_runtime*,MetaNode*);
extern ast_err MetaNode_read(ast_runtime*,MetaNode**);
extern ast_err MetaNode_reclaim(ast_runtime*,MetaNode*);
extern size_t MetaNode_get_size(ast_runtime*,MetaNode*);

struct MetaGraph {
    struct {size_t count; MetaNode** values;} nodeset;
    MetaNode* rootgroup;
};


extern ast_err MetaGraph_write(ast_runtime*,MetaGraph*);
extern ast_err MetaGraph_read(ast_runtime*,MetaGraph**);
extern ast_err MetaGraph_reclaim(ast_runtime*,MetaGraph*);
extern size_t MetaGraph_get_size(ast_runtime*,MetaGraph*);

struct MetaGroup {
    struct {size_t count; MetaNode** values;} typeset;
    struct {size_t count; MetaNode** values;} varset;
    struct {size_t count; MetaNode** values;} dimrset;
    struct {size_t count; MetaNode** values;} groups;
};


extern ast_err MetaGroup_write(ast_runtime*,MetaGroup*);
extern ast_err MetaGroup_read(ast_runtime*,MetaGroup**);
extern ast_err MetaGroup_reclaim(ast_runtime*,MetaGroup*);
extern size_t MetaGroup_get_size(ast_runtime*,MetaGroup*);

struct MetaVar {
    struct {size_t count; MetaNode** values;} dims;
};


extern ast_err MetaVar_write(ast_runtime*,MetaVar*);
extern ast_err MetaVar_read(ast_runtime*,MetaVar**);
extern ast_err MetaVar_reclaim(ast_runtime*,MetaVar*);
extern size_t MetaVar_get_size(ast_runtime*,MetaVar*);

struct MetaDim {
    uint64_t actualsize;
};


extern ast_err MetaDim_write(ast_runtime*,MetaDim*);
extern ast_err MetaDim_read(ast_runtime*,MetaDim**);
extern ast_err MetaDim_reclaim(ast_runtime*,MetaDim*);
extern size_t MetaDim_get_size(ast_runtime*,MetaDim*);

struct MetaCompound {
    struct {size_t count; MetaNode** values;} fields;
};


extern ast_err MetaCompound_write(ast_runtime*,MetaCompound*);
extern ast_err MetaCompound_read(ast_runtime*,MetaCompound**);
extern ast_err MetaCompound_reclaim(ast_runtime*,MetaCompound*);
extern size_t MetaCompound_get_size(ast_runtime*,MetaCompound*);

struct MetaField {
    struct {size_t count; uint64_t* values;} dims;
    uint64_t offset;
    uint64_t alignment;
};


extern ast_err MetaField_write(ast_runtime*,MetaField*);
extern ast_err MetaField_read(ast_runtime*,MetaField**);
extern ast_err MetaField_reclaim(ast_runtime*,MetaField*);
extern size_t MetaField_get_size(ast_runtime*,MetaField*);

struct MetaEnum {
    struct {size_t count; MetaEconst** values;} econsts;
};


extern ast_err MetaEnum_write(ast_runtime*,MetaEnum*);
extern ast_err MetaEnum_read(ast_runtime*,MetaEnum**);
extern ast_err MetaEnum_reclaim(ast_runtime*,MetaEnum*);
extern size_t MetaEnum_get_size(ast_runtime*,MetaEnum*);

struct MetaEconst {
    char* name;
    bytes_t value;
};


extern ast_err MetaEconst_write(ast_runtime*,MetaEconst*);
extern ast_err MetaEconst_read(ast_runtime*,MetaEconst**);
extern ast_err MetaEconst_reclaim(ast_runtime*,MetaEconst*);
extern size_t MetaEconst_get_size(ast_runtime*,MetaEconst*);

#endif /*PROTORPC_H*/
