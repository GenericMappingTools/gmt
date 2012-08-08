#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include <ast.h>

#include "protorpc.h"

ast_err
NCCreate_write(ast_runtime* rt, NCCreate* nccreate_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&nccreate_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&nccreate_v->cmode);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&nccreate_v->initialsz);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&nccreate_v->basepe);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,6,&nccreate_v->use_parallel);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCCreate_write*/

ast_err
NCCreate_read(ast_runtime* rt, NCCreate** nccreate_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCCreate* nccreate_v;
    unsigned long pos;

    nccreate_v = (NCCreate*)ast_alloc(rt,sizeof(NCCreate));
    if(nccreate_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCCreate|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&nccreate_v->path);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&nccreate_v->cmode);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&nccreate_v->initialsz);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&nccreate_v->basepe);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_int32,6,&nccreate_v->use_parallel);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(nccreate_vp) *nccreate_vp = nccreate_v;
done:
    return ACATCH(status);
} /*NCCreate_read*/

ast_err
NCCreate_reclaim(ast_runtime* rt, NCCreate* nccreate_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,nccreate_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)nccreate_v);
    goto done;

done:
    return ACATCH(status);

} /*NCCreate_reclaim*/

size_t
NCCreate_get_size(ast_runtime* rt, NCCreate* nccreate_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&nccreate_v->path);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&nccreate_v->cmode);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&nccreate_v->initialsz);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&nccreate_v->basepe);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_int32,&nccreate_v->use_parallel);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCCreate_get_size*/

ast_err
NCCreate_Return_write(ast_runtime* rt, NCCreate_Return* nccreate_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&nccreate_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&nccreate_return_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCCreate_Return_write*/

ast_err
NCCreate_Return_read(ast_runtime* rt, NCCreate_Return** nccreate_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCCreate_Return* nccreate_return_v;
    unsigned long pos;

    nccreate_return_v = (NCCreate_Return*)ast_alloc(rt,sizeof(NCCreate_Return));
    if(nccreate_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCCreate_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&nccreate_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&nccreate_return_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(nccreate_return_vp) *nccreate_return_vp = nccreate_return_v;
done:
    return ACATCH(status);
} /*NCCreate_Return_read*/

ast_err
NCCreate_Return_reclaim(ast_runtime* rt, NCCreate_Return* nccreate_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)nccreate_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCCreate_Return_reclaim*/

size_t
NCCreate_Return_get_size(ast_runtime* rt, NCCreate_Return* nccreate_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&nccreate_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&nccreate_return_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCCreate_Return_get_size*/

ast_err
NCOpen_write(ast_runtime* rt, NCOpen* ncopen_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&ncopen_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncopen_v->cmode);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncopen_v->basepe);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncopen_v->chunksizehint.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncopen_v->chunksizehint.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncopen_v->use_parallel);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,6,&ncopen_v->parameters);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCOpen_write*/

ast_err
NCOpen_read(ast_runtime* rt, NCOpen** ncopen_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCOpen* ncopen_v;
    unsigned long pos;

    ncopen_v = (NCOpen*)ast_alloc(rt,sizeof(NCOpen));
    if(ncopen_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCOpen|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&ncopen_v->path);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncopen_v->cmode);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncopen_v->basepe);
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncopen_v->chunksizehint,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncopen_v->use_parallel);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_bytes,6,&ncopen_v->parameters);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncopen_vp) *ncopen_vp = ncopen_v;
done:
    return ACATCH(status);
} /*NCOpen_read*/

ast_err
NCOpen_reclaim(ast_runtime* rt, NCOpen* ncopen_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncopen_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncopen_v->parameters);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncopen_v);
    goto done;

done:
    return ACATCH(status);

} /*NCOpen_reclaim*/

size_t
NCOpen_get_size(ast_runtime* rt, NCOpen* ncopen_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&ncopen_v->path);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncopen_v->cmode);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncopen_v->basepe);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncopen_v->chunksizehint.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncopen_v->chunksizehint.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncopen_v->use_parallel);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_bytes,&ncopen_v->parameters);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCOpen_get_size*/

ast_err
NCOpen_Return_write(ast_runtime* rt, NCOpen_Return* ncopen_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncopen_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncopen_return_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCOpen_Return_write*/

ast_err
NCOpen_Return_read(ast_runtime* rt, NCOpen_Return** ncopen_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCOpen_Return* ncopen_return_v;
    unsigned long pos;

    ncopen_return_v = (NCOpen_Return*)ast_alloc(rt,sizeof(NCOpen_Return));
    if(ncopen_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCOpen_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncopen_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncopen_return_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncopen_return_vp) *ncopen_return_vp = ncopen_return_v;
done:
    return ACATCH(status);
} /*NCOpen_Return_read*/

ast_err
NCOpen_Return_reclaim(ast_runtime* rt, NCOpen_Return* ncopen_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncopen_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCOpen_Return_reclaim*/

size_t
NCOpen_Return_get_size(ast_runtime* rt, NCOpen_Return* ncopen_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncopen_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncopen_return_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCOpen_Return_get_size*/

ast_err
NCRedef_write(ast_runtime* rt, NCRedef* ncredef_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncredef_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRedef_write*/

ast_err
NCRedef_read(ast_runtime* rt, NCRedef** ncredef_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRedef* ncredef_v;
    unsigned long pos;

    ncredef_v = (NCRedef*)ast_alloc(rt,sizeof(NCRedef));
    if(ncredef_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRedef|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncredef_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncredef_vp) *ncredef_vp = ncredef_v;
done:
    return ACATCH(status);
} /*NCRedef_read*/

ast_err
NCRedef_reclaim(ast_runtime* rt, NCRedef* ncredef_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncredef_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRedef_reclaim*/

size_t
NCRedef_get_size(ast_runtime* rt, NCRedef* ncredef_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncredef_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRedef_get_size*/

ast_err
NCRedef_Return_write(ast_runtime* rt, NCRedef_Return* ncredef_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncredef_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRedef_Return_write*/

ast_err
NCRedef_Return_read(ast_runtime* rt, NCRedef_Return** ncredef_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRedef_Return* ncredef_return_v;
    unsigned long pos;

    ncredef_return_v = (NCRedef_Return*)ast_alloc(rt,sizeof(NCRedef_Return));
    if(ncredef_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRedef_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncredef_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncredef_return_vp) *ncredef_return_vp = ncredef_return_v;
done:
    return ACATCH(status);
} /*NCRedef_Return_read*/

ast_err
NCRedef_Return_reclaim(ast_runtime* rt, NCRedef_Return* ncredef_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncredef_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRedef_Return_reclaim*/

size_t
NCRedef_Return_get_size(ast_runtime* rt, NCRedef_Return* ncredef_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncredef_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRedef_Return_get_size*/

ast_err
NC_Enddef_write(ast_runtime* rt, NC_Enddef* nc_enddef_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&nc_enddef_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&nc_enddef_v->minfree);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&nc_enddef_v->v_align);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&nc_enddef_v->v_minfree);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,5,&nc_enddef_v->r_align);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NC_Enddef_write*/

ast_err
NC_Enddef_read(ast_runtime* rt, NC_Enddef** nc_enddef_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NC_Enddef* nc_enddef_v;
    unsigned long pos;

    nc_enddef_v = (NC_Enddef*)ast_alloc(rt,sizeof(NC_Enddef));
    if(nc_enddef_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NC_Enddef|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&nc_enddef_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&nc_enddef_v->minfree);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&nc_enddef_v->v_align);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&nc_enddef_v->v_minfree);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_uint64,5,&nc_enddef_v->r_align);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(nc_enddef_vp) *nc_enddef_vp = nc_enddef_v;
done:
    return ACATCH(status);
} /*NC_Enddef_read*/

ast_err
NC_Enddef_reclaim(ast_runtime* rt, NC_Enddef* nc_enddef_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)nc_enddef_v);
    goto done;

done:
    return ACATCH(status);

} /*NC_Enddef_reclaim*/

size_t
NC_Enddef_get_size(ast_runtime* rt, NC_Enddef* nc_enddef_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&nc_enddef_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&nc_enddef_v->minfree);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&nc_enddef_v->v_align);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&nc_enddef_v->v_minfree);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_uint64,&nc_enddef_v->r_align);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NC_Enddef_get_size*/

ast_err
NC_Enddef_Return_write(ast_runtime* rt, NC_Enddef_Return* nc_enddef_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&nc_enddef_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NC_Enddef_Return_write*/

ast_err
NC_Enddef_Return_read(ast_runtime* rt, NC_Enddef_Return** nc_enddef_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NC_Enddef_Return* nc_enddef_return_v;
    unsigned long pos;

    nc_enddef_return_v = (NC_Enddef_Return*)ast_alloc(rt,sizeof(NC_Enddef_Return));
    if(nc_enddef_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NC_Enddef_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&nc_enddef_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(nc_enddef_return_vp) *nc_enddef_return_vp = nc_enddef_return_v;
done:
    return ACATCH(status);
} /*NC_Enddef_Return_read*/

ast_err
NC_Enddef_Return_reclaim(ast_runtime* rt, NC_Enddef_Return* nc_enddef_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)nc_enddef_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NC_Enddef_Return_reclaim*/

size_t
NC_Enddef_Return_get_size(ast_runtime* rt, NC_Enddef_Return* nc_enddef_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&nc_enddef_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NC_Enddef_Return_get_size*/

ast_err
NCSync_write(ast_runtime* rt, NCSync* ncsync_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncsync_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSync_write*/

ast_err
NCSync_read(ast_runtime* rt, NCSync** ncsync_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSync* ncsync_v;
    unsigned long pos;

    ncsync_v = (NCSync*)ast_alloc(rt,sizeof(NCSync));
    if(ncsync_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSync|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncsync_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncsync_vp) *ncsync_vp = ncsync_v;
done:
    return ACATCH(status);
} /*NCSync_read*/

ast_err
NCSync_reclaim(ast_runtime* rt, NCSync* ncsync_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncsync_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSync_reclaim*/

size_t
NCSync_get_size(ast_runtime* rt, NCSync* ncsync_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncsync_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSync_get_size*/

ast_err
NCSync_Return_write(ast_runtime* rt, NCSync_Return* ncsync_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncsync_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSync_Return_write*/

ast_err
NCSync_Return_read(ast_runtime* rt, NCSync_Return** ncsync_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSync_Return* ncsync_return_v;
    unsigned long pos;

    ncsync_return_v = (NCSync_Return*)ast_alloc(rt,sizeof(NCSync_Return));
    if(ncsync_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSync_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncsync_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncsync_return_vp) *ncsync_return_vp = ncsync_return_v;
done:
    return ACATCH(status);
} /*NCSync_Return_read*/

ast_err
NCSync_Return_reclaim(ast_runtime* rt, NCSync_Return* ncsync_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncsync_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSync_Return_reclaim*/

size_t
NCSync_Return_get_size(ast_runtime* rt, NCSync_Return* ncsync_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncsync_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSync_Return_get_size*/

ast_err
NCAbort_write(ast_runtime* rt, NCAbort* ncabort_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncabort_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCAbort_write*/

ast_err
NCAbort_read(ast_runtime* rt, NCAbort** ncabort_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCAbort* ncabort_v;
    unsigned long pos;

    ncabort_v = (NCAbort*)ast_alloc(rt,sizeof(NCAbort));
    if(ncabort_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCAbort|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncabort_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncabort_vp) *ncabort_vp = ncabort_v;
done:
    return ACATCH(status);
} /*NCAbort_read*/

ast_err
NCAbort_reclaim(ast_runtime* rt, NCAbort* ncabort_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncabort_v);
    goto done;

done:
    return ACATCH(status);

} /*NCAbort_reclaim*/

size_t
NCAbort_get_size(ast_runtime* rt, NCAbort* ncabort_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncabort_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCAbort_get_size*/

ast_err
NCAbort_Return_write(ast_runtime* rt, NCAbort_Return* ncabort_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncabort_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCAbort_Return_write*/

ast_err
NCAbort_Return_read(ast_runtime* rt, NCAbort_Return** ncabort_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCAbort_Return* ncabort_return_v;
    unsigned long pos;

    ncabort_return_v = (NCAbort_Return*)ast_alloc(rt,sizeof(NCAbort_Return));
    if(ncabort_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCAbort_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncabort_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncabort_return_vp) *ncabort_return_vp = ncabort_return_v;
done:
    return ACATCH(status);
} /*NCAbort_Return_read*/

ast_err
NCAbort_Return_reclaim(ast_runtime* rt, NCAbort_Return* ncabort_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncabort_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCAbort_Return_reclaim*/

size_t
NCAbort_Return_get_size(ast_runtime* rt, NCAbort_Return* ncabort_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncabort_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCAbort_Return_get_size*/

ast_err
NCClose_write(ast_runtime* rt, NCClose* ncclose_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncclose_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCClose_write*/

ast_err
NCClose_read(ast_runtime* rt, NCClose** ncclose_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCClose* ncclose_v;
    unsigned long pos;

    ncclose_v = (NCClose*)ast_alloc(rt,sizeof(NCClose));
    if(ncclose_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCClose|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncclose_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncclose_vp) *ncclose_vp = ncclose_v;
done:
    return ACATCH(status);
} /*NCClose_read*/

ast_err
NCClose_reclaim(ast_runtime* rt, NCClose* ncclose_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncclose_v);
    goto done;

done:
    return ACATCH(status);

} /*NCClose_reclaim*/

size_t
NCClose_get_size(ast_runtime* rt, NCClose* ncclose_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncclose_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCClose_get_size*/

ast_err
NCClose_Return_write(ast_runtime* rt, NCClose_Return* ncclose_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncclose_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCClose_Return_write*/

ast_err
NCClose_Return_read(ast_runtime* rt, NCClose_Return** ncclose_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCClose_Return* ncclose_return_v;
    unsigned long pos;

    ncclose_return_v = (NCClose_Return*)ast_alloc(rt,sizeof(NCClose_Return));
    if(ncclose_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCClose_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncclose_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncclose_return_vp) *ncclose_return_vp = ncclose_return_v;
done:
    return ACATCH(status);
} /*NCClose_Return_read*/

ast_err
NCClose_Return_reclaim(ast_runtime* rt, NCClose_Return* ncclose_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncclose_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCClose_Return_reclaim*/

size_t
NCClose_Return_get_size(ast_runtime* rt, NCClose_Return* ncclose_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncclose_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCClose_Return_get_size*/

ast_err
NCSet_Fill_write(ast_runtime* rt, NCSet_Fill* ncset_fill_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_fill_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncset_fill_v->fillmode);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_Fill_write*/

ast_err
NCSet_Fill_read(ast_runtime* rt, NCSet_Fill** ncset_fill_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_Fill* ncset_fill_v;
    unsigned long pos;

    ncset_fill_v = (NCSet_Fill*)ast_alloc(rt,sizeof(NCSet_Fill));
    if(ncset_fill_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_Fill|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_fill_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncset_fill_v->fillmode);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_fill_vp) *ncset_fill_vp = ncset_fill_v;
done:
    return ACATCH(status);
} /*NCSet_Fill_read*/

ast_err
NCSet_Fill_reclaim(ast_runtime* rt, NCSet_Fill* ncset_fill_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_fill_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_Fill_reclaim*/

size_t
NCSet_Fill_get_size(ast_runtime* rt, NCSet_Fill* ncset_fill_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_fill_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_fill_v->fillmode);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_Fill_get_size*/

ast_err
NCSet_Fill_Return_write(ast_runtime* rt, NCSet_Fill_Return* ncset_fill_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_fill_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncset_fill_return_v->oldmode);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_Fill_Return_write*/

ast_err
NCSet_Fill_Return_read(ast_runtime* rt, NCSet_Fill_Return** ncset_fill_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_Fill_Return* ncset_fill_return_v;
    unsigned long pos;

    ncset_fill_return_v = (NCSet_Fill_Return*)ast_alloc(rt,sizeof(NCSet_Fill_Return));
    if(ncset_fill_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_Fill_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_fill_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncset_fill_return_v->oldmode);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_fill_return_vp) *ncset_fill_return_vp = ncset_fill_return_v;
done:
    return ACATCH(status);
} /*NCSet_Fill_Return_read*/

ast_err
NCSet_Fill_Return_reclaim(ast_runtime* rt, NCSet_Fill_Return* ncset_fill_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_fill_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_Fill_Return_reclaim*/

size_t
NCSet_Fill_Return_get_size(ast_runtime* rt, NCSet_Fill_Return* ncset_fill_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_fill_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_fill_return_v->oldmode);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_Fill_Return_get_size*/

ast_err
NCInq_Base_PE_write(ast_runtime* rt, NCInq_Base_PE* ncinq_base_pe_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_base_pe_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_Base_PE_write*/

ast_err
NCInq_Base_PE_read(ast_runtime* rt, NCInq_Base_PE** ncinq_base_pe_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_Base_PE* ncinq_base_pe_v;
    unsigned long pos;

    ncinq_base_pe_v = (NCInq_Base_PE*)ast_alloc(rt,sizeof(NCInq_Base_PE));
    if(ncinq_base_pe_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_Base_PE|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_base_pe_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_base_pe_vp) *ncinq_base_pe_vp = ncinq_base_pe_v;
done:
    return ACATCH(status);
} /*NCInq_Base_PE_read*/

ast_err
NCInq_Base_PE_reclaim(ast_runtime* rt, NCInq_Base_PE* ncinq_base_pe_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_base_pe_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_Base_PE_reclaim*/

size_t
NCInq_Base_PE_get_size(ast_runtime* rt, NCInq_Base_PE* ncinq_base_pe_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_base_pe_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_Base_PE_get_size*/

ast_err
NCInq_Base_PE_Return_write(ast_runtime* rt, NCInq_Base_PE_Return* ncinq_base_pe_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_base_pe_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_base_pe_return_v->pe);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_Base_PE_Return_write*/

ast_err
NCInq_Base_PE_Return_read(ast_runtime* rt, NCInq_Base_PE_Return** ncinq_base_pe_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_Base_PE_Return* ncinq_base_pe_return_v;
    unsigned long pos;

    ncinq_base_pe_return_v = (NCInq_Base_PE_Return*)ast_alloc(rt,sizeof(NCInq_Base_PE_Return));
    if(ncinq_base_pe_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_Base_PE_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_base_pe_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_base_pe_return_v->pe);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_base_pe_return_vp) *ncinq_base_pe_return_vp = ncinq_base_pe_return_v;
done:
    return ACATCH(status);
} /*NCInq_Base_PE_Return_read*/

ast_err
NCInq_Base_PE_Return_reclaim(ast_runtime* rt, NCInq_Base_PE_Return* ncinq_base_pe_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_base_pe_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_Base_PE_Return_reclaim*/

size_t
NCInq_Base_PE_Return_get_size(ast_runtime* rt, NCInq_Base_PE_Return* ncinq_base_pe_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_base_pe_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_base_pe_return_v->pe);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_Base_PE_Return_get_size*/

ast_err
NCSet_base_pe_write(ast_runtime* rt, NCSet_base_pe* ncset_base_pe_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_base_pe_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncset_base_pe_v->pe);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_base_pe_write*/

ast_err
NCSet_base_pe_read(ast_runtime* rt, NCSet_base_pe** ncset_base_pe_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_base_pe* ncset_base_pe_v;
    unsigned long pos;

    ncset_base_pe_v = (NCSet_base_pe*)ast_alloc(rt,sizeof(NCSet_base_pe));
    if(ncset_base_pe_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_base_pe|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_base_pe_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncset_base_pe_v->pe);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_base_pe_vp) *ncset_base_pe_vp = ncset_base_pe_v;
done:
    return ACATCH(status);
} /*NCSet_base_pe_read*/

ast_err
NCSet_base_pe_reclaim(ast_runtime* rt, NCSet_base_pe* ncset_base_pe_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_base_pe_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_base_pe_reclaim*/

size_t
NCSet_base_pe_get_size(ast_runtime* rt, NCSet_base_pe* ncset_base_pe_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_base_pe_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_base_pe_v->pe);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_base_pe_get_size*/

ast_err
NCSet_base_pe_Return_write(ast_runtime* rt, NCSet_base_pe_Return* ncset_base_pe_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_base_pe_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_base_pe_Return_write*/

ast_err
NCSet_base_pe_Return_read(ast_runtime* rt, NCSet_base_pe_Return** ncset_base_pe_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_base_pe_Return* ncset_base_pe_return_v;
    unsigned long pos;

    ncset_base_pe_return_v = (NCSet_base_pe_Return*)ast_alloc(rt,sizeof(NCSet_base_pe_Return));
    if(ncset_base_pe_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_base_pe_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_base_pe_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_base_pe_return_vp) *ncset_base_pe_return_vp = ncset_base_pe_return_v;
done:
    return ACATCH(status);
} /*NCSet_base_pe_Return_read*/

ast_err
NCSet_base_pe_Return_reclaim(ast_runtime* rt, NCSet_base_pe_Return* ncset_base_pe_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_base_pe_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_base_pe_Return_reclaim*/

size_t
NCSet_base_pe_Return_get_size(ast_runtime* rt, NCSet_base_pe_Return* ncset_base_pe_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_base_pe_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_base_pe_Return_get_size*/

ast_err
NCInq_format_write(ast_runtime* rt, NCInq_format* ncinq_format_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_format_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_format_write*/

ast_err
NCInq_format_read(ast_runtime* rt, NCInq_format** ncinq_format_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_format* ncinq_format_v;
    unsigned long pos;

    ncinq_format_v = (NCInq_format*)ast_alloc(rt,sizeof(NCInq_format));
    if(ncinq_format_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_format|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_format_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_format_vp) *ncinq_format_vp = ncinq_format_v;
done:
    return ACATCH(status);
} /*NCInq_format_read*/

ast_err
NCInq_format_reclaim(ast_runtime* rt, NCInq_format* ncinq_format_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_format_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_format_reclaim*/

size_t
NCInq_format_get_size(ast_runtime* rt, NCInq_format* ncinq_format_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_format_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_format_get_size*/

ast_err
NCInq_format_Return_write(ast_runtime* rt, NCInq_format_Return* ncinq_format_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_format_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_format_return_v->format);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_format_Return_write*/

ast_err
NCInq_format_Return_read(ast_runtime* rt, NCInq_format_Return** ncinq_format_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_format_Return* ncinq_format_return_v;
    unsigned long pos;

    ncinq_format_return_v = (NCInq_format_Return*)ast_alloc(rt,sizeof(NCInq_format_Return));
    if(ncinq_format_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_format_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_format_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_format_return_v->format);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_format_return_vp) *ncinq_format_return_vp = ncinq_format_return_v;
done:
    return ACATCH(status);
} /*NCInq_format_Return_read*/

ast_err
NCInq_format_Return_reclaim(ast_runtime* rt, NCInq_format_Return* ncinq_format_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_format_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_format_Return_reclaim*/

size_t
NCInq_format_Return_get_size(ast_runtime* rt, NCInq_format_Return* ncinq_format_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_format_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_format_return_v->format);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_format_Return_get_size*/

ast_err
NCInq_write(ast_runtime* rt, NCInq* ncinq_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_write*/

ast_err
NCInq_read(ast_runtime* rt, NCInq** ncinq_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq* ncinq_v;
    unsigned long pos;

    ncinq_v = (NCInq*)ast_alloc(rt,sizeof(NCInq));
    if(ncinq_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_vp) *ncinq_vp = ncinq_v;
done:
    return ACATCH(status);
} /*NCInq_read*/

ast_err
NCInq_reclaim(ast_runtime* rt, NCInq* ncinq_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_reclaim*/

size_t
NCInq_get_size(ast_runtime* rt, NCInq* ncinq_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_get_size*/

ast_err
NCInq_Return_write(ast_runtime* rt, NCInq_Return* ncinq_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_return_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_return_v->nvars);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncinq_return_v->natts);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncinq_return_v->unlimdimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_Return_write*/

ast_err
NCInq_Return_read(ast_runtime* rt, NCInq_Return** ncinq_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_Return* ncinq_return_v;
    unsigned long pos;

    ncinq_return_v = (NCInq_Return*)ast_alloc(rt,sizeof(NCInq_Return));
    if(ncinq_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_return_v->ndims);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_return_v->nvars);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncinq_return_v->natts);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncinq_return_v->unlimdimid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_return_vp) *ncinq_return_vp = ncinq_return_v;
done:
    return ACATCH(status);
} /*NCInq_Return_read*/

ast_err
NCInq_Return_reclaim(ast_runtime* rt, NCInq_Return* ncinq_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_Return_reclaim*/

size_t
NCInq_Return_get_size(ast_runtime* rt, NCInq_Return* ncinq_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_return_v->ndims);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_return_v->nvars);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_return_v->natts);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_return_v->unlimdimid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_Return_get_size*/

ast_err
NCInq_Type_write(ast_runtime* rt, NCInq_Type* ncinq_type_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_type_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_type_v->xtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_Type_write*/

ast_err
NCInq_Type_read(ast_runtime* rt, NCInq_Type** ncinq_type_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_Type* ncinq_type_v;
    unsigned long pos;

    ncinq_type_v = (NCInq_Type*)ast_alloc(rt,sizeof(NCInq_Type));
    if(ncinq_type_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_Type|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_type_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_type_v->xtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_type_vp) *ncinq_type_vp = ncinq_type_v;
done:
    return ACATCH(status);
} /*NCInq_Type_read*/

ast_err
NCInq_Type_reclaim(ast_runtime* rt, NCInq_Type* ncinq_type_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_type_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_Type_reclaim*/

size_t
NCInq_Type_get_size(ast_runtime* rt, NCInq_Type* ncinq_type_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_v->xtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_Type_get_size*/

ast_err
NCInq_Type_Return_write(ast_runtime* rt, NCInq_Type_Return* ncinq_type_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_type_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_type_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_type_return_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_Type_Return_write*/

ast_err
NCInq_Type_Return_read(ast_runtime* rt, NCInq_Type_Return** ncinq_type_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_Type_Return* ncinq_type_return_v;
    unsigned long pos;

    ncinq_type_return_v = (NCInq_Type_Return*)ast_alloc(rt,sizeof(NCInq_Type_Return));
    if(ncinq_type_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_Type_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_type_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_type_return_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_type_return_v->size);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_type_return_vp) *ncinq_type_return_vp = ncinq_type_return_v;
done:
    return ACATCH(status);
} /*NCInq_Type_Return_read*/

ast_err
NCInq_Type_Return_reclaim(ast_runtime* rt, NCInq_Type_Return* ncinq_type_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_type_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_type_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_Type_Return_reclaim*/

size_t
NCInq_Type_Return_get_size(ast_runtime* rt, NCInq_Type_Return* ncinq_type_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_type_return_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_type_return_v->size);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_Type_Return_get_size*/

ast_err
NCDef_Dim_write(ast_runtime* rt, NCDef_Dim* ncdef_dim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_dim_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncdef_dim_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncdef_dim_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Dim_write*/

ast_err
NCDef_Dim_read(ast_runtime* rt, NCDef_Dim** ncdef_dim_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Dim* ncdef_dim_v;
    unsigned long pos;

    ncdef_dim_v = (NCDef_Dim*)ast_alloc(rt,sizeof(NCDef_Dim));
    if(ncdef_dim_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Dim|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_dim_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncdef_dim_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncdef_dim_v->len);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_dim_vp) *ncdef_dim_vp = ncdef_dim_v;
done:
    return ACATCH(status);
} /*NCDef_Dim_read*/

ast_err
NCDef_Dim_reclaim(ast_runtime* rt, NCDef_Dim* ncdef_dim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_dim_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_dim_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Dim_reclaim*/

size_t
NCDef_Dim_get_size(ast_runtime* rt, NCDef_Dim* ncdef_dim_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_dim_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_dim_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncdef_dim_v->len);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Dim_get_size*/

ast_err
NCDef_Dim_Return_write(ast_runtime* rt, NCDef_Dim_Return* ncdef_dim_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_dim_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_dim_return_v->dimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Dim_Return_write*/

ast_err
NCDef_Dim_Return_read(ast_runtime* rt, NCDef_Dim_Return** ncdef_dim_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Dim_Return* ncdef_dim_return_v;
    unsigned long pos;

    ncdef_dim_return_v = (NCDef_Dim_Return*)ast_alloc(rt,sizeof(NCDef_Dim_Return));
    if(ncdef_dim_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Dim_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_dim_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_dim_return_v->dimid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_dim_return_vp) *ncdef_dim_return_vp = ncdef_dim_return_v;
done:
    return ACATCH(status);
} /*NCDef_Dim_Return_read*/

ast_err
NCDef_Dim_Return_reclaim(ast_runtime* rt, NCDef_Dim_Return* ncdef_dim_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_dim_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Dim_Return_reclaim*/

size_t
NCDef_Dim_Return_get_size(ast_runtime* rt, NCDef_Dim_Return* ncdef_dim_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_dim_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_dim_return_v->dimid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Dim_Return_get_size*/

ast_err
NCInq_dimid_write(ast_runtime* rt, NCInq_dimid* ncinq_dimid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dimid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_dimid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_dimid_write*/

ast_err
NCInq_dimid_read(ast_runtime* rt, NCInq_dimid** ncinq_dimid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dimid* ncinq_dimid_v;
    unsigned long pos;

    ncinq_dimid_v = (NCInq_dimid*)ast_alloc(rt,sizeof(NCInq_dimid));
    if(ncinq_dimid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dimid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dimid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_dimid_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dimid_vp) *ncinq_dimid_vp = ncinq_dimid_v;
done:
    return ACATCH(status);
} /*NCInq_dimid_read*/

ast_err
NCInq_dimid_reclaim(ast_runtime* rt, NCInq_dimid* ncinq_dimid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_dimid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_dimid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dimid_reclaim*/

size_t
NCInq_dimid_get_size(ast_runtime* rt, NCInq_dimid* ncinq_dimid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_dimid_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dimid_get_size*/

ast_err
NCInq_dimid_Return_write(ast_runtime* rt, NCInq_dimid_Return* ncinq_dimid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dimid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_dimid_return_v->dimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_dimid_Return_write*/

ast_err
NCInq_dimid_Return_read(ast_runtime* rt, NCInq_dimid_Return** ncinq_dimid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dimid_Return* ncinq_dimid_return_v;
    unsigned long pos;

    ncinq_dimid_return_v = (NCInq_dimid_Return*)ast_alloc(rt,sizeof(NCInq_dimid_Return));
    if(ncinq_dimid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dimid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dimid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_dimid_return_v->dimid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dimid_return_vp) *ncinq_dimid_return_vp = ncinq_dimid_return_v;
done:
    return ACATCH(status);
} /*NCInq_dimid_Return_read*/

ast_err
NCInq_dimid_Return_reclaim(ast_runtime* rt, NCInq_dimid_Return* ncinq_dimid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_dimid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dimid_Return_reclaim*/

size_t
NCInq_dimid_Return_get_size(ast_runtime* rt, NCInq_dimid_Return* ncinq_dimid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimid_return_v->dimid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dimid_Return_get_size*/

ast_err
NCInq_dim_write(ast_runtime* rt, NCInq_dim* ncinq_dim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dim_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_dim_v->dimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_dim_write*/

ast_err
NCInq_dim_read(ast_runtime* rt, NCInq_dim** ncinq_dim_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dim* ncinq_dim_v;
    unsigned long pos;

    ncinq_dim_v = (NCInq_dim*)ast_alloc(rt,sizeof(NCInq_dim));
    if(ncinq_dim_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dim|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dim_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_dim_v->dimid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dim_vp) *ncinq_dim_vp = ncinq_dim_v;
done:
    return ACATCH(status);
} /*NCInq_dim_read*/

ast_err
NCInq_dim_reclaim(ast_runtime* rt, NCInq_dim* ncinq_dim_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_dim_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dim_reclaim*/

size_t
NCInq_dim_get_size(ast_runtime* rt, NCInq_dim* ncinq_dim_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dim_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dim_v->dimid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dim_get_size*/

ast_err
NCInq_dim_Return_write(ast_runtime* rt, NCInq_dim_Return* ncinq_dim_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dim_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_dim_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_dim_return_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_dim_Return_write*/

ast_err
NCInq_dim_Return_read(ast_runtime* rt, NCInq_dim_Return** ncinq_dim_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dim_Return* ncinq_dim_return_v;
    unsigned long pos;

    ncinq_dim_return_v = (NCInq_dim_Return*)ast_alloc(rt,sizeof(NCInq_dim_Return));
    if(ncinq_dim_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dim_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dim_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_dim_return_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_dim_return_v->len);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dim_return_vp) *ncinq_dim_return_vp = ncinq_dim_return_v;
done:
    return ACATCH(status);
} /*NCInq_dim_Return_read*/

ast_err
NCInq_dim_Return_reclaim(ast_runtime* rt, NCInq_dim_Return* ncinq_dim_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_dim_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_dim_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dim_Return_reclaim*/

size_t
NCInq_dim_Return_get_size(ast_runtime* rt, NCInq_dim_Return* ncinq_dim_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dim_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_dim_return_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_dim_return_v->len);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dim_Return_get_size*/

ast_err
NCInq_unlimdim_write(ast_runtime* rt, NCInq_unlimdim* ncinq_unlimdim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_unlimdim_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_unlimdim_write*/

ast_err
NCInq_unlimdim_read(ast_runtime* rt, NCInq_unlimdim** ncinq_unlimdim_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_unlimdim* ncinq_unlimdim_v;
    unsigned long pos;

    ncinq_unlimdim_v = (NCInq_unlimdim*)ast_alloc(rt,sizeof(NCInq_unlimdim));
    if(ncinq_unlimdim_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_unlimdim|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_unlimdim_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_unlimdim_vp) *ncinq_unlimdim_vp = ncinq_unlimdim_v;
done:
    return ACATCH(status);
} /*NCInq_unlimdim_read*/

ast_err
NCInq_unlimdim_reclaim(ast_runtime* rt, NCInq_unlimdim* ncinq_unlimdim_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_unlimdim_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_unlimdim_reclaim*/

size_t
NCInq_unlimdim_get_size(ast_runtime* rt, NCInq_unlimdim* ncinq_unlimdim_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdim_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_unlimdim_get_size*/

ast_err
NCInq_unlimdim_Return_write(ast_runtime* rt, NCInq_unlimdim_Return* ncinq_unlimdim_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_unlimdim_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_unlimdim_return_v->unlimdimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_unlimdim_Return_write*/

ast_err
NCInq_unlimdim_Return_read(ast_runtime* rt, NCInq_unlimdim_Return** ncinq_unlimdim_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_unlimdim_Return* ncinq_unlimdim_return_v;
    unsigned long pos;

    ncinq_unlimdim_return_v = (NCInq_unlimdim_Return*)ast_alloc(rt,sizeof(NCInq_unlimdim_Return));
    if(ncinq_unlimdim_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_unlimdim_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_unlimdim_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_unlimdim_return_v->unlimdimid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_unlimdim_return_vp) *ncinq_unlimdim_return_vp = ncinq_unlimdim_return_v;
done:
    return ACATCH(status);
} /*NCInq_unlimdim_Return_read*/

ast_err
NCInq_unlimdim_Return_reclaim(ast_runtime* rt, NCInq_unlimdim_Return* ncinq_unlimdim_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_unlimdim_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_unlimdim_Return_reclaim*/

size_t
NCInq_unlimdim_Return_get_size(ast_runtime* rt, NCInq_unlimdim_Return* ncinq_unlimdim_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdim_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdim_return_v->unlimdimid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_unlimdim_Return_get_size*/

ast_err
NCRename_dim_write(ast_runtime* rt, NCRename_dim* ncrename_dim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_dim_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncrename_dim_v->dimid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncrename_dim_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_dim_write*/

ast_err
NCRename_dim_read(ast_runtime* rt, NCRename_dim** ncrename_dim_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_dim* ncrename_dim_v;
    unsigned long pos;

    ncrename_dim_v = (NCRename_dim*)ast_alloc(rt,sizeof(NCRename_dim));
    if(ncrename_dim_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_dim|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_dim_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncrename_dim_v->dimid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncrename_dim_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_dim_vp) *ncrename_dim_vp = ncrename_dim_v;
done:
    return ACATCH(status);
} /*NCRename_dim_read*/

ast_err
NCRename_dim_reclaim(ast_runtime* rt, NCRename_dim* ncrename_dim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncrename_dim_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncrename_dim_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_dim_reclaim*/

size_t
NCRename_dim_get_size(ast_runtime* rt, NCRename_dim* ncrename_dim_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_dim_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_dim_v->dimid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncrename_dim_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_dim_get_size*/

ast_err
NCRename_dim_Return_write(ast_runtime* rt, NCRename_dim_Return* ncrename_dim_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_dim_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_dim_Return_write*/

ast_err
NCRename_dim_Return_read(ast_runtime* rt, NCRename_dim_Return** ncrename_dim_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_dim_Return* ncrename_dim_return_v;
    unsigned long pos;

    ncrename_dim_return_v = (NCRename_dim_Return*)ast_alloc(rt,sizeof(NCRename_dim_Return));
    if(ncrename_dim_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_dim_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_dim_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_dim_return_vp) *ncrename_dim_return_vp = ncrename_dim_return_v;
done:
    return ACATCH(status);
} /*NCRename_dim_Return_read*/

ast_err
NCRename_dim_Return_reclaim(ast_runtime* rt, NCRename_dim_Return* ncrename_dim_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncrename_dim_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_dim_Return_reclaim*/

size_t
NCRename_dim_Return_get_size(ast_runtime* rt, NCRename_dim_Return* ncrename_dim_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_dim_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_dim_Return_get_size*/

ast_err
NCInq_att_write(ast_runtime* rt, NCInq_att* ncinq_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_att_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_att_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinq_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_att_write*/

ast_err
NCInq_att_read(ast_runtime* rt, NCInq_att** ncinq_att_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_att* ncinq_att_v;
    unsigned long pos;

    ncinq_att_v = (NCInq_att*)ast_alloc(rt,sizeof(NCInq_att));
    if(ncinq_att_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_att|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_att_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_att_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinq_att_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_att_vp) *ncinq_att_vp = ncinq_att_v;
done:
    return ACATCH(status);
} /*NCInq_att_read*/

ast_err
NCInq_att_reclaim(ast_runtime* rt, NCInq_att* ncinq_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_att_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_att_reclaim*/

size_t
NCInq_att_get_size(ast_runtime* rt, NCInq_att* ncinq_att_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_att_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_att_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_att_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_att_get_size*/

ast_err
NCInq_att_Return_write(ast_runtime* rt, NCInq_att_Return* ncinq_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_att_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_att_return_v->xtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_att_return_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_att_Return_write*/

ast_err
NCInq_att_Return_read(ast_runtime* rt, NCInq_att_Return** ncinq_att_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_att_Return* ncinq_att_return_v;
    unsigned long pos;

    ncinq_att_return_v = (NCInq_att_Return*)ast_alloc(rt,sizeof(NCInq_att_Return));
    if(ncinq_att_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_att_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_att_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_att_return_v->xtype);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_att_return_v->len);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_att_return_vp) *ncinq_att_return_vp = ncinq_att_return_v;
done:
    return ACATCH(status);
} /*NCInq_att_Return_read*/

ast_err
NCInq_att_Return_reclaim(ast_runtime* rt, NCInq_att_Return* ncinq_att_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_att_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_att_Return_reclaim*/

size_t
NCInq_att_Return_get_size(ast_runtime* rt, NCInq_att_Return* ncinq_att_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_att_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_att_return_v->xtype);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_att_return_v->len);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_att_Return_get_size*/

ast_err
NCInq_attid_write(ast_runtime* rt, NCInq_attid* ncinq_attid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_attid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_attid_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinq_attid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_attid_write*/

ast_err
NCInq_attid_read(ast_runtime* rt, NCInq_attid** ncinq_attid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_attid* ncinq_attid_v;
    unsigned long pos;

    ncinq_attid_v = (NCInq_attid*)ast_alloc(rt,sizeof(NCInq_attid));
    if(ncinq_attid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_attid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_attid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_attid_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinq_attid_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_attid_vp) *ncinq_attid_vp = ncinq_attid_v;
done:
    return ACATCH(status);
} /*NCInq_attid_read*/

ast_err
NCInq_attid_reclaim(ast_runtime* rt, NCInq_attid* ncinq_attid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_attid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_attid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_attid_reclaim*/

size_t
NCInq_attid_get_size(ast_runtime* rt, NCInq_attid* ncinq_attid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attid_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_attid_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_attid_get_size*/

ast_err
NCInq_attid_Return_write(ast_runtime* rt, NCInq_attid_Return* ncinq_attid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_attid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_attid_return_v->attid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_attid_Return_write*/

ast_err
NCInq_attid_Return_read(ast_runtime* rt, NCInq_attid_Return** ncinq_attid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_attid_Return* ncinq_attid_return_v;
    unsigned long pos;

    ncinq_attid_return_v = (NCInq_attid_Return*)ast_alloc(rt,sizeof(NCInq_attid_Return));
    if(ncinq_attid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_attid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_attid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_attid_return_v->attid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_attid_return_vp) *ncinq_attid_return_vp = ncinq_attid_return_v;
done:
    return ACATCH(status);
} /*NCInq_attid_Return_read*/

ast_err
NCInq_attid_Return_reclaim(ast_runtime* rt, NCInq_attid_Return* ncinq_attid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_attid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_attid_Return_reclaim*/

size_t
NCInq_attid_Return_get_size(ast_runtime* rt, NCInq_attid_Return* ncinq_attid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attid_return_v->attid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_attid_Return_get_size*/

ast_err
NCInq_attname_write(ast_runtime* rt, NCInq_attname* ncinq_attname_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_attname_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_attname_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_attname_v->attnum);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_attname_write*/

ast_err
NCInq_attname_read(ast_runtime* rt, NCInq_attname** ncinq_attname_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_attname* ncinq_attname_v;
    unsigned long pos;

    ncinq_attname_v = (NCInq_attname*)ast_alloc(rt,sizeof(NCInq_attname));
    if(ncinq_attname_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_attname|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_attname_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_attname_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_attname_v->attnum);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_attname_vp) *ncinq_attname_vp = ncinq_attname_v;
done:
    return ACATCH(status);
} /*NCInq_attname_read*/

ast_err
NCInq_attname_reclaim(ast_runtime* rt, NCInq_attname* ncinq_attname_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_attname_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_attname_reclaim*/

size_t
NCInq_attname_get_size(ast_runtime* rt, NCInq_attname* ncinq_attname_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attname_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attname_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attname_v->attnum);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_attname_get_size*/

ast_err
NCInq_attname_Return_write(ast_runtime* rt, NCInq_attname_Return* ncinq_attname_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_attname_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_attname_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_attname_Return_write*/

ast_err
NCInq_attname_Return_read(ast_runtime* rt, NCInq_attname_Return** ncinq_attname_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_attname_Return* ncinq_attname_return_v;
    unsigned long pos;

    ncinq_attname_return_v = (NCInq_attname_Return*)ast_alloc(rt,sizeof(NCInq_attname_Return));
    if(ncinq_attname_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_attname_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_attname_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_attname_return_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_attname_return_vp) *ncinq_attname_return_vp = ncinq_attname_return_v;
done:
    return ACATCH(status);
} /*NCInq_attname_Return_read*/

ast_err
NCInq_attname_Return_reclaim(ast_runtime* rt, NCInq_attname_Return* ncinq_attname_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_attname_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_attname_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_attname_Return_reclaim*/

size_t
NCInq_attname_Return_get_size(ast_runtime* rt, NCInq_attname_Return* ncinq_attname_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_attname_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_attname_return_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_attname_Return_get_size*/

ast_err
NCRename_att_write(ast_runtime* rt, NCRename_att* ncrename_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_att_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncrename_att_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncrename_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,4,&ncrename_att_v->newname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_att_write*/

ast_err
NCRename_att_read(ast_runtime* rt, NCRename_att** ncrename_att_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_att* ncrename_att_v;
    unsigned long pos;

    ncrename_att_v = (NCRename_att*)ast_alloc(rt,sizeof(NCRename_att));
    if(ncrename_att_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_att|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_att_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncrename_att_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncrename_att_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_string,4,&ncrename_att_v->newname);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_att_vp) *ncrename_att_vp = ncrename_att_v;
done:
    return ACATCH(status);
} /*NCRename_att_read*/

ast_err
NCRename_att_reclaim(ast_runtime* rt, NCRename_att* ncrename_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncrename_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_string(rt,ncrename_att_v->newname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncrename_att_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_att_reclaim*/

size_t
NCRename_att_get_size(ast_runtime* rt, NCRename_att* ncrename_att_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_att_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_att_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncrename_att_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_string,&ncrename_att_v->newname);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_att_get_size*/

ast_err
NCRename_att_Return_write(ast_runtime* rt, NCRename_att_Return* ncrename_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_att_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_att_Return_write*/

ast_err
NCRename_att_Return_read(ast_runtime* rt, NCRename_att_Return** ncrename_att_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_att_Return* ncrename_att_return_v;
    unsigned long pos;

    ncrename_att_return_v = (NCRename_att_Return*)ast_alloc(rt,sizeof(NCRename_att_Return));
    if(ncrename_att_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_att_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_att_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_att_return_vp) *ncrename_att_return_vp = ncrename_att_return_v;
done:
    return ACATCH(status);
} /*NCRename_att_Return_read*/

ast_err
NCRename_att_Return_reclaim(ast_runtime* rt, NCRename_att_Return* ncrename_att_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncrename_att_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_att_Return_reclaim*/

size_t
NCRename_att_Return_get_size(ast_runtime* rt, NCRename_att_Return* ncrename_att_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_att_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_att_Return_get_size*/

ast_err
NCDel_att_write(ast_runtime* rt, NCDel_att* ncdel_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdel_att_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdel_att_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncdel_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDel_att_write*/

ast_err
NCDel_att_read(ast_runtime* rt, NCDel_att** ncdel_att_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDel_att* ncdel_att_v;
    unsigned long pos;

    ncdel_att_v = (NCDel_att*)ast_alloc(rt,sizeof(NCDel_att));
    if(ncdel_att_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDel_att|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdel_att_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdel_att_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncdel_att_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdel_att_vp) *ncdel_att_vp = ncdel_att_v;
done:
    return ACATCH(status);
} /*NCDel_att_read*/

ast_err
NCDel_att_reclaim(ast_runtime* rt, NCDel_att* ncdel_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdel_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdel_att_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDel_att_reclaim*/

size_t
NCDel_att_get_size(ast_runtime* rt, NCDel_att* ncdel_att_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdel_att_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdel_att_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncdel_att_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDel_att_get_size*/

ast_err
NCDel_att_Return_write(ast_runtime* rt, NCDel_att_Return* ncdel_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdel_att_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDel_att_Return_write*/

ast_err
NCDel_att_Return_read(ast_runtime* rt, NCDel_att_Return** ncdel_att_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDel_att_Return* ncdel_att_return_v;
    unsigned long pos;

    ncdel_att_return_v = (NCDel_att_Return*)ast_alloc(rt,sizeof(NCDel_att_Return));
    if(ncdel_att_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDel_att_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdel_att_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdel_att_return_vp) *ncdel_att_return_vp = ncdel_att_return_v;
done:
    return ACATCH(status);
} /*NCDel_att_Return_read*/

ast_err
NCDel_att_Return_reclaim(ast_runtime* rt, NCDel_att_Return* ncdel_att_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdel_att_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDel_att_Return_reclaim*/

size_t
NCDel_att_Return_get_size(ast_runtime* rt, NCDel_att_Return* ncdel_att_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdel_att_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDel_att_Return_get_size*/

ast_err
NCGet_att_write(ast_runtime* rt, NCGet_att* ncget_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_att_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_att_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncget_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncget_att_v->xtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_att_write*/

ast_err
NCGet_att_read(ast_runtime* rt, NCGet_att** ncget_att_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_att* ncget_att_v;
    unsigned long pos;

    ncget_att_v = (NCGet_att*)ast_alloc(rt,sizeof(NCGet_att));
    if(ncget_att_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_att|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_att_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_att_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncget_att_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncget_att_v->xtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_att_vp) *ncget_att_vp = ncget_att_v;
done:
    return ACATCH(status);
} /*NCGet_att_read*/

ast_err
NCGet_att_reclaim(ast_runtime* rt, NCGet_att* ncget_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncget_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_att_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_att_reclaim*/

size_t
NCGet_att_get_size(ast_runtime* rt, NCGet_att* ncget_att_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_att_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_att_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncget_att_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_att_v->xtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_att_get_size*/

ast_err
NCGet_att_Return_write(ast_runtime* rt, NCGet_att_Return* ncget_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_att_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&ncget_att_return_v->values);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_att_Return_write*/

ast_err
NCGet_att_Return_read(ast_runtime* rt, NCGet_att_Return** ncget_att_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_att_Return* ncget_att_return_v;
    unsigned long pos;

    ncget_att_return_v = (NCGet_att_Return*)ast_alloc(rt,sizeof(NCGet_att_Return));
    if(ncget_att_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_att_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_att_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&ncget_att_return_v->values);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_att_return_vp) *ncget_att_return_vp = ncget_att_return_v;
done:
    return ACATCH(status);
} /*NCGet_att_Return_read*/

ast_err
NCGet_att_Return_reclaim(ast_runtime* rt, NCGet_att_Return* ncget_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncget_att_return_v->values);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_att_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_att_Return_reclaim*/

size_t
NCGet_att_Return_get_size(ast_runtime* rt, NCGet_att_Return* ncget_att_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_att_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_att_return_v->values);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_att_Return_get_size*/

ast_err
NCPut_att_write(ast_runtime* rt, NCPut_att* ncput_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_att_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncput_att_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncput_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncput_att_v->vtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,5,&ncput_att_v->nelems);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,6,&ncput_att_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,7,&ncput_att_v->atype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_att_write*/

ast_err
NCPut_att_read(ast_runtime* rt, NCPut_att** ncput_att_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_att* ncput_att_v;
    unsigned long pos;

    ncput_att_v = (NCPut_att*)ast_alloc(rt,sizeof(NCPut_att));
    if(ncput_att_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_att|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_att_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncput_att_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncput_att_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncput_att_v->vtype);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_uint64,5,&ncput_att_v->nelems);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_bytes,6,&ncput_att_v->value);
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_int32,7,&ncput_att_v->atype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_att_vp) *ncput_att_vp = ncput_att_v;
done:
    return ACATCH(status);
} /*NCPut_att_read*/

ast_err
NCPut_att_reclaim(ast_runtime* rt, NCPut_att* ncput_att_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncput_att_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncput_att_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncput_att_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_att_reclaim*/

size_t
NCPut_att_get_size(ast_runtime* rt, NCPut_att* ncput_att_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_att_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_att_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncput_att_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_att_v->vtype);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_uint64,&ncput_att_v->nelems);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_att_v->value);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_att_v->atype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_att_get_size*/

ast_err
NCPut_att_Return_write(ast_runtime* rt, NCPut_att_Return* ncput_att_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_att_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_att_Return_write*/

ast_err
NCPut_att_Return_read(ast_runtime* rt, NCPut_att_Return** ncput_att_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_att_Return* ncput_att_return_v;
    unsigned long pos;

    ncput_att_return_v = (NCPut_att_Return*)ast_alloc(rt,sizeof(NCPut_att_Return));
    if(ncput_att_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_att_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_att_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_att_return_vp) *ncput_att_return_vp = ncput_att_return_v;
done:
    return ACATCH(status);
} /*NCPut_att_Return_read*/

ast_err
NCPut_att_Return_reclaim(ast_runtime* rt, NCPut_att_Return* ncput_att_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncput_att_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_att_Return_reclaim*/

size_t
NCPut_att_Return_get_size(ast_runtime* rt, NCPut_att_Return* ncput_att_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_att_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_att_Return_get_size*/

ast_err
NCDef_Var_write(ast_runtime* rt, NCDef_Var* ncdef_var_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncdef_var_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncdef_var_v->xtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncdef_var_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncdef_var_v->dimids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,5,&ncdef_var_v->dimids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCDef_Var_write*/

ast_err
NCDef_Var_read(ast_runtime* rt, NCDef_Var** ncdef_var_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var* ncdef_var_v;
    unsigned long pos;

    ncdef_var_v = (NCDef_Var*)ast_alloc(rt,sizeof(NCDef_Var));
    if(ncdef_var_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncdef_var_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncdef_var_v->xtype);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncdef_var_v->ndims);
            } break;
        case 5: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncdef_var_v->dimids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_vp) *ncdef_var_vp = ncdef_var_v;
done:
    return ACATCH(status);
} /*NCDef_Var_read*/

ast_err
NCDef_Var_reclaim(ast_runtime* rt, NCDef_Var* ncdef_var_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_var_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_var_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_reclaim*/

size_t
NCDef_Var_get_size(ast_runtime* rt, NCDef_Var* ncdef_var_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_var_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_v->xtype);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_v->ndims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncdef_var_v->dimids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_v->dimids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_get_size*/

ast_err
NCDef_Var_Return_write(ast_runtime* rt, NCDef_Var_Return* ncdef_var_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_return_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Return_write*/

ast_err
NCDef_Var_Return_read(ast_runtime* rt, NCDef_Var_Return** ncdef_var_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Return* ncdef_var_return_v;
    unsigned long pos;

    ncdef_var_return_v = (NCDef_Var_Return*)ast_alloc(rt,sizeof(NCDef_Var_Return));
    if(ncdef_var_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_return_v->varid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_return_vp) *ncdef_var_return_vp = ncdef_var_return_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Return_read*/

ast_err
NCDef_Var_Return_reclaim(ast_runtime* rt, NCDef_Var_Return* ncdef_var_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Return_reclaim*/

size_t
NCDef_Var_Return_get_size(ast_runtime* rt, NCDef_Var_Return* ncdef_var_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_return_v->varid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Return_get_size*/

ast_err
NCInq_varid_write(ast_runtime* rt, NCInq_varid* ncinq_varid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_varid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_varid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_varid_write*/

ast_err
NCInq_varid_read(ast_runtime* rt, NCInq_varid** ncinq_varid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_varid* ncinq_varid_v;
    unsigned long pos;

    ncinq_varid_v = (NCInq_varid*)ast_alloc(rt,sizeof(NCInq_varid));
    if(ncinq_varid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_varid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_varid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_varid_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_varid_vp) *ncinq_varid_vp = ncinq_varid_v;
done:
    return ACATCH(status);
} /*NCInq_varid_read*/

ast_err
NCInq_varid_reclaim(ast_runtime* rt, NCInq_varid* ncinq_varid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_varid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_varid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_varid_reclaim*/

size_t
NCInq_varid_get_size(ast_runtime* rt, NCInq_varid* ncinq_varid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_varid_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_varid_get_size*/

ast_err
NCInq_varid_Return_write(ast_runtime* rt, NCInq_varid_Return* ncinq_varid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_varid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_varid_return_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_varid_Return_write*/

ast_err
NCInq_varid_Return_read(ast_runtime* rt, NCInq_varid_Return** ncinq_varid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_varid_Return* ncinq_varid_return_v;
    unsigned long pos;

    ncinq_varid_return_v = (NCInq_varid_Return*)ast_alloc(rt,sizeof(NCInq_varid_Return));
    if(ncinq_varid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_varid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_varid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_varid_return_v->varid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_varid_return_vp) *ncinq_varid_return_vp = ncinq_varid_return_v;
done:
    return ACATCH(status);
} /*NCInq_varid_Return_read*/

ast_err
NCInq_varid_Return_reclaim(ast_runtime* rt, NCInq_varid_Return* ncinq_varid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_varid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_varid_Return_reclaim*/

size_t
NCInq_varid_Return_get_size(ast_runtime* rt, NCInq_varid_Return* ncinq_varid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varid_return_v->varid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_varid_Return_get_size*/

ast_err
NCRename_var_write(ast_runtime* rt, NCRename_var* ncrename_var_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_var_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncrename_var_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncrename_var_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_var_write*/

ast_err
NCRename_var_read(ast_runtime* rt, NCRename_var** ncrename_var_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_var* ncrename_var_v;
    unsigned long pos;

    ncrename_var_v = (NCRename_var*)ast_alloc(rt,sizeof(NCRename_var));
    if(ncrename_var_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_var|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_var_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncrename_var_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncrename_var_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_var_vp) *ncrename_var_vp = ncrename_var_v;
done:
    return ACATCH(status);
} /*NCRename_var_read*/

ast_err
NCRename_var_reclaim(ast_runtime* rt, NCRename_var* ncrename_var_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncrename_var_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncrename_var_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_var_reclaim*/

size_t
NCRename_var_get_size(ast_runtime* rt, NCRename_var* ncrename_var_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_var_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_var_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncrename_var_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_var_get_size*/

ast_err
NCRename_var_Return_write(ast_runtime* rt, NCRename_var_Return* ncrename_var_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncrename_var_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCRename_var_Return_write*/

ast_err
NCRename_var_Return_read(ast_runtime* rt, NCRename_var_Return** ncrename_var_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCRename_var_Return* ncrename_var_return_v;
    unsigned long pos;

    ncrename_var_return_v = (NCRename_var_Return*)ast_alloc(rt,sizeof(NCRename_var_Return));
    if(ncrename_var_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCRename_var_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncrename_var_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncrename_var_return_vp) *ncrename_var_return_vp = ncrename_var_return_v;
done:
    return ACATCH(status);
} /*NCRename_var_Return_read*/

ast_err
NCRename_var_Return_reclaim(ast_runtime* rt, NCRename_var_Return* ncrename_var_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncrename_var_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCRename_var_Return_reclaim*/

size_t
NCRename_var_Return_get_size(ast_runtime* rt, NCRename_var_Return* ncrename_var_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncrename_var_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCRename_var_Return_get_size*/

ast_err
NCGet_vara_write(ast_runtime* rt, NCGet_vara* ncget_vara_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vara_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_vara_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncget_vara_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncget_vara_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_vara_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncget_vara_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncget_vara_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vara_write*/

ast_err
NCGet_vara_read(ast_runtime* rt, NCGet_vara** ncget_vara_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vara* ncget_vara_v;
    unsigned long pos;

    ncget_vara_v = (NCGet_vara*)ast_alloc(rt,sizeof(NCGet_vara));
    if(ncget_vara_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vara|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vara_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_vara_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_vara_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_vara_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncget_vara_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vara_vp) *ncget_vara_vp = ncget_vara_v;
done:
    return ACATCH(status);
} /*NCGet_vara_read*/

ast_err
NCGet_vara_reclaim(ast_runtime* rt, NCGet_vara* ncget_vara_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_vara_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vara_reclaim*/

size_t
NCGet_vara_get_size(ast_runtime* rt, NCGet_vara* ncget_vara_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vara_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vara_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_vara_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_vara_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_vara_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_vara_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vara_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vara_get_size*/

ast_err
NCGet_vara_Return_write(ast_runtime* rt, NCGet_vara_Return* ncget_vara_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vara_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&ncget_vara_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vara_Return_write*/

ast_err
NCGet_vara_Return_read(ast_runtime* rt, NCGet_vara_Return** ncget_vara_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vara_Return* ncget_vara_return_v;
    unsigned long pos;

    ncget_vara_return_v = (NCGet_vara_Return*)ast_alloc(rt,sizeof(NCGet_vara_Return));
    if(ncget_vara_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vara_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vara_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&ncget_vara_return_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vara_return_vp) *ncget_vara_return_vp = ncget_vara_return_v;
done:
    return ACATCH(status);
} /*NCGet_vara_Return_read*/

ast_err
NCGet_vara_Return_reclaim(ast_runtime* rt, NCGet_vara_Return* ncget_vara_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncget_vara_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_vara_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vara_Return_reclaim*/

size_t
NCGet_vara_Return_get_size(ast_runtime* rt, NCGet_vara_Return* ncget_vara_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vara_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_vara_return_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vara_Return_get_size*/

ast_err
NCPut_vara_write(ast_runtime* rt, NCPut_vara* ncput_vara_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vara_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncput_vara_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncput_vara_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncput_vara_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_vara_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncput_vara_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_bytes,5,&ncput_vara_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,6,&ncput_vara_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vara_write*/

ast_err
NCPut_vara_read(ast_runtime* rt, NCPut_vara** ncput_vara_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vara* ncput_vara_v;
    unsigned long pos;

    ncput_vara_v = (NCPut_vara*)ast_alloc(rt,sizeof(NCPut_vara));
    if(ncput_vara_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vara|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vara_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncput_vara_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_vara_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_vara_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_bytes,5,&ncput_vara_v->value);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_int32,6,&ncput_vara_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vara_vp) *ncput_vara_vp = ncput_vara_v;
done:
    return ACATCH(status);
} /*NCPut_vara_read*/

ast_err
NCPut_vara_reclaim(ast_runtime* rt, NCPut_vara* ncput_vara_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncput_vara_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncput_vara_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vara_reclaim*/

size_t
NCPut_vara_get_size(ast_runtime* rt, NCPut_vara* ncput_vara_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vara_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vara_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_vara_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_vara_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_vara_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_vara_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_vara_v->value);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vara_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vara_get_size*/

ast_err
NCPut_vara_Return_write(ast_runtime* rt, NCPut_vara_Return* ncput_vara_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vara_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vara_Return_write*/

ast_err
NCPut_vara_Return_read(ast_runtime* rt, NCPut_vara_Return** ncput_vara_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vara_Return* ncput_vara_return_v;
    unsigned long pos;

    ncput_vara_return_v = (NCPut_vara_Return*)ast_alloc(rt,sizeof(NCPut_vara_Return));
    if(ncput_vara_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vara_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vara_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vara_return_vp) *ncput_vara_return_vp = ncput_vara_return_v;
done:
    return ACATCH(status);
} /*NCPut_vara_Return_read*/

ast_err
NCPut_vara_Return_reclaim(ast_runtime* rt, NCPut_vara_Return* ncput_vara_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncput_vara_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vara_Return_reclaim*/

size_t
NCPut_vara_Return_get_size(ast_runtime* rt, NCPut_vara_Return* ncput_vara_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vara_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vara_Return_get_size*/

ast_err
NCGet_vars_write(ast_runtime* rt, NCGet_vars* ncget_vars_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vars_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_vars_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncget_vars_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncget_vars_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_vars_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncget_vars_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_vars_v->stride.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,5,&ncget_vars_v->stride.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_int32,6,&ncget_vars_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vars_write*/

ast_err
NCGet_vars_read(ast_runtime* rt, NCGet_vars** ncget_vars_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vars* ncget_vars_v;
    unsigned long pos;

    ncget_vars_v = (NCGet_vars*)ast_alloc(rt,sizeof(NCGet_vars));
    if(ncget_vars_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vars|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vars_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_vars_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_vars_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_vars_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_vars_v->stride,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_int32,6,&ncget_vars_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vars_vp) *ncget_vars_vp = ncget_vars_v;
done:
    return ACATCH(status);
} /*NCGet_vars_read*/

ast_err
NCGet_vars_reclaim(ast_runtime* rt, NCGet_vars* ncget_vars_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_vars_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vars_reclaim*/

size_t
NCGet_vars_get_size(ast_runtime* rt, NCGet_vars* ncget_vars_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vars_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vars_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_vars_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_vars_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_vars_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_vars_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_vars_v->stride.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_vars_v->stride.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vars_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vars_get_size*/

ast_err
NCGet_vars_Return_write(ast_runtime* rt, NCGet_vars_Return* ncget_vars_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vars_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&ncget_vars_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vars_Return_write*/

ast_err
NCGet_vars_Return_read(ast_runtime* rt, NCGet_vars_Return** ncget_vars_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vars_Return* ncget_vars_return_v;
    unsigned long pos;

    ncget_vars_return_v = (NCGet_vars_Return*)ast_alloc(rt,sizeof(NCGet_vars_Return));
    if(ncget_vars_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vars_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vars_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&ncget_vars_return_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vars_return_vp) *ncget_vars_return_vp = ncget_vars_return_v;
done:
    return ACATCH(status);
} /*NCGet_vars_Return_read*/

ast_err
NCGet_vars_Return_reclaim(ast_runtime* rt, NCGet_vars_Return* ncget_vars_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncget_vars_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_vars_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vars_Return_reclaim*/

size_t
NCGet_vars_Return_get_size(ast_runtime* rt, NCGet_vars_Return* ncget_vars_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vars_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_vars_return_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vars_Return_get_size*/

ast_err
NCPut_vars_write(ast_runtime* rt, NCPut_vars* ncput_vars_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vars_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncput_vars_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncput_vars_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncput_vars_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_vars_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncput_vars_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_vars_v->stride.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,5,&ncput_vars_v->stride.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_bytes,6,&ncput_vars_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,7,&ncput_vars_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vars_write*/

ast_err
NCPut_vars_read(ast_runtime* rt, NCPut_vars** ncput_vars_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vars* ncput_vars_v;
    unsigned long pos;

    ncput_vars_v = (NCPut_vars*)ast_alloc(rt,sizeof(NCPut_vars));
    if(ncput_vars_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vars|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vars_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncput_vars_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_vars_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_vars_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_vars_v->stride,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_bytes,6,&ncput_vars_v->value);
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_int32,7,&ncput_vars_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vars_vp) *ncput_vars_vp = ncput_vars_v;
done:
    return ACATCH(status);
} /*NCPut_vars_read*/

ast_err
NCPut_vars_reclaim(ast_runtime* rt, NCPut_vars* ncput_vars_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncput_vars_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncput_vars_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vars_reclaim*/

size_t
NCPut_vars_get_size(ast_runtime* rt, NCPut_vars* ncput_vars_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vars_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vars_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_vars_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_vars_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_vars_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_vars_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_vars_v->stride.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_vars_v->stride.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_vars_v->value);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vars_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vars_get_size*/

ast_err
NCPut_vars_Return_write(ast_runtime* rt, NCPut_vars_Return* ncput_vars_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vars_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vars_Return_write*/

ast_err
NCPut_vars_Return_read(ast_runtime* rt, NCPut_vars_Return** ncput_vars_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vars_Return* ncput_vars_return_v;
    unsigned long pos;

    ncput_vars_return_v = (NCPut_vars_Return*)ast_alloc(rt,sizeof(NCPut_vars_Return));
    if(ncput_vars_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vars_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vars_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vars_return_vp) *ncput_vars_return_vp = ncput_vars_return_v;
done:
    return ACATCH(status);
} /*NCPut_vars_Return_read*/

ast_err
NCPut_vars_Return_reclaim(ast_runtime* rt, NCPut_vars_Return* ncput_vars_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncput_vars_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vars_Return_reclaim*/

size_t
NCPut_vars_Return_get_size(ast_runtime* rt, NCPut_vars_Return* ncput_vars_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vars_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vars_Return_get_size*/

ast_err
NCGet_varm_write(ast_runtime* rt, NCGet_varm* ncget_varm_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_varm_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_varm_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncget_varm_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncget_varm_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_varm_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncget_varm_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_varm_v->stride.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,5,&ncget_varm_v->stride.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncget_varm_v->imap.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,6,&ncget_varm_v->imap.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_int32,7,&ncget_varm_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_varm_write*/

ast_err
NCGet_varm_read(ast_runtime* rt, NCGet_varm** ncget_varm_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_varm* ncget_varm_v;
    unsigned long pos;

    ncget_varm_v = (NCGet_varm*)ast_alloc(rt,sizeof(NCGet_varm));
    if(ncget_varm_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_varm|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_varm_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_varm_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_varm_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_varm_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_varm_v->stride,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 6: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,6,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncget_varm_v->imap,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_int32,7,&ncget_varm_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_varm_vp) *ncget_varm_vp = ncget_varm_v;
done:
    return ACATCH(status);
} /*NCGet_varm_read*/

ast_err
NCGet_varm_reclaim(ast_runtime* rt, NCGet_varm* ncget_varm_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_varm_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_varm_reclaim*/

size_t
NCGet_varm_get_size(ast_runtime* rt, NCGet_varm* ncget_varm_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_varm_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_varm_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_varm_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_varm_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_varm_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_varm_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_varm_v->stride.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_varm_v->stride.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncget_varm_v->imap.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_uint64,&ncget_varm_v->imap.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_varm_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_varm_get_size*/

ast_err
NCGet_varm_Return_write(ast_runtime* rt, NCGet_varm_Return* ncget_varm_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_varm_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&ncget_varm_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_varm_Return_write*/

ast_err
NCGet_varm_Return_read(ast_runtime* rt, NCGet_varm_Return** ncget_varm_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_varm_Return* ncget_varm_return_v;
    unsigned long pos;

    ncget_varm_return_v = (NCGet_varm_Return*)ast_alloc(rt,sizeof(NCGet_varm_Return));
    if(ncget_varm_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_varm_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_varm_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&ncget_varm_return_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_varm_return_vp) *ncget_varm_return_vp = ncget_varm_return_v;
done:
    return ACATCH(status);
} /*NCGet_varm_Return_read*/

ast_err
NCGet_varm_Return_reclaim(ast_runtime* rt, NCGet_varm_Return* ncget_varm_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncget_varm_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_varm_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_varm_Return_reclaim*/

size_t
NCGet_varm_Return_get_size(ast_runtime* rt, NCGet_varm_Return* ncget_varm_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_varm_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_varm_return_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_varm_Return_get_size*/

ast_err
NCPut_varm_write(ast_runtime* rt, NCPut_varm* ncput_varm_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_varm_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncput_varm_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncput_varm_v->start.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,3,&ncput_varm_v->start.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_varm_v->edges.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncput_varm_v->edges.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_varm_v->stride.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,5,&ncput_varm_v->stride.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<ncput_varm_v->imap.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,6,&ncput_varm_v->imap.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_bytes,7,&ncput_varm_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,8,&ncput_varm_v->memtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_varm_write*/

ast_err
NCPut_varm_read(ast_runtime* rt, NCPut_varm** ncput_varm_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_varm* ncput_varm_v;
    unsigned long pos;

    ncput_varm_v = (NCPut_varm*)ast_alloc(rt,sizeof(NCPut_varm));
    if(ncput_varm_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_varm|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_varm_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncput_varm_v->varid);
            } break;
        case 3: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_varm_v->start,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_varm_v->edges,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_varm_v->stride,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 6: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,6,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncput_varm_v->imap,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_bytes,7,&ncput_varm_v->value);
            } break;
        case 8: {
            status = ast_read_primitive(rt,ast_int32,8,&ncput_varm_v->memtype);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_varm_vp) *ncput_varm_vp = ncput_varm_v;
done:
    return ACATCH(status);
} /*NCPut_varm_read*/

ast_err
NCPut_varm_reclaim(ast_runtime* rt, NCPut_varm* ncput_varm_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncput_varm_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncput_varm_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_varm_reclaim*/

size_t
NCPut_varm_get_size(ast_runtime* rt, NCPut_varm* ncput_varm_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_varm_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_varm_v->varid);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_varm_v->start.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_varm_v->start.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_varm_v->edges.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_varm_v->edges.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_varm_v->stride.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_varm_v->stride.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncput_varm_v->imap.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_uint64,&ncput_varm_v->imap.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_varm_v->value);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,8);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_varm_v->memtype);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_varm_get_size*/

ast_err
NCPut_varm_Return_write(ast_runtime* rt, NCPut_varm_Return* ncput_varm_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_varm_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_varm_Return_write*/

ast_err
NCPut_varm_Return_read(ast_runtime* rt, NCPut_varm_Return** ncput_varm_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_varm_Return* ncput_varm_return_v;
    unsigned long pos;

    ncput_varm_return_v = (NCPut_varm_Return*)ast_alloc(rt,sizeof(NCPut_varm_Return));
    if(ncput_varm_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_varm_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_varm_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_varm_return_vp) *ncput_varm_return_vp = ncput_varm_return_v;
done:
    return ACATCH(status);
} /*NCPut_varm_Return_read*/

ast_err
NCPut_varm_Return_reclaim(ast_runtime* rt, NCPut_varm_Return* ncput_varm_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncput_varm_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_varm_Return_reclaim*/

size_t
NCPut_varm_Return_get_size(ast_runtime* rt, NCPut_varm_Return* ncput_varm_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_varm_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_varm_Return_get_size*/

ast_err
NCInq_var_all_write(ast_runtime* rt, NCInq_var_all* ncinq_var_all_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_var_all_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_var_all_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinq_var_all_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_var_all_write*/

ast_err
NCInq_var_all_read(ast_runtime* rt, NCInq_var_all** ncinq_var_all_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_var_all* ncinq_var_all_v;
    unsigned long pos;

    ncinq_var_all_v = (NCInq_var_all*)ast_alloc(rt,sizeof(NCInq_var_all));
    if(ncinq_var_all_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_var_all|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_var_all_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_var_all_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinq_var_all_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_var_all_vp) *ncinq_var_all_vp = ncinq_var_all_v;
done:
    return ACATCH(status);
} /*NCInq_var_all_read*/

ast_err
NCInq_var_all_reclaim(ast_runtime* rt, NCInq_var_all* ncinq_var_all_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_var_all_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_var_all_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_var_all_reclaim*/

size_t
NCInq_var_all_get_size(ast_runtime* rt, NCInq_var_all* ncinq_var_all_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_var_all_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_var_all_get_size*/

ast_err
NCInq_var_all_Return_write(ast_runtime* rt, NCInq_var_all_Return* ncinq_var_all_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_var_all_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_var_all_return_v->xtype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_var_all_return_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_var_all_return_v->dimids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,4,&ncinq_var_all_return_v->dimids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncinq_var_all_return_v->natts);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,6,&ncinq_var_all_return_v->shuffle);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,7,&ncinq_var_all_return_v->deflate);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,8,&ncinq_var_all_return_v->deflate_level);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,9,&ncinq_var_all_return_v->fletcher32);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,10,&ncinq_var_all_return_v->contiguous);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_var_all_return_v->chunksizes.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,11,&ncinq_var_all_return_v->chunksizes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_bool,12,&ncinq_var_all_return_v->no_fill);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,13,&ncinq_var_all_return_v->fill_value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,14,&ncinq_var_all_return_v->endianness);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,15,&ncinq_var_all_return_v->options_mask);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,16,&ncinq_var_all_return_v->pixels_per_block);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_var_all_Return_write*/

ast_err
NCInq_var_all_Return_read(ast_runtime* rt, NCInq_var_all_Return** ncinq_var_all_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_var_all_Return* ncinq_var_all_return_v;
    unsigned long pos;

    ncinq_var_all_return_v = (NCInq_var_all_Return*)ast_alloc(rt,sizeof(NCInq_var_all_Return));
    if(ncinq_var_all_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_var_all_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_var_all_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_var_all_return_v->xtype);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_var_all_return_v->ndims);
            } break;
        case 4: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_var_all_return_v->dimids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncinq_var_all_return_v->natts);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_bool,6,&ncinq_var_all_return_v->shuffle);
            } break;
        case 7: {
            status = ast_read_primitive(rt,ast_bool,7,&ncinq_var_all_return_v->deflate);
            } break;
        case 8: {
            status = ast_read_primitive(rt,ast_int32,8,&ncinq_var_all_return_v->deflate_level);
            } break;
        case 9: {
            status = ast_read_primitive(rt,ast_bool,9,&ncinq_var_all_return_v->fletcher32);
            } break;
        case 10: {
            status = ast_read_primitive(rt,ast_bool,10,&ncinq_var_all_return_v->contiguous);
            } break;
        case 11: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,11,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncinq_var_all_return_v->chunksizes,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 12: {
            status = ast_read_primitive(rt,ast_bool,12,&ncinq_var_all_return_v->no_fill);
            } break;
        case 13: {
            status = ast_read_primitive(rt,ast_bytes,13,&ncinq_var_all_return_v->fill_value);
            } break;
        case 14: {
            status = ast_read_primitive(rt,ast_bool,14,&ncinq_var_all_return_v->endianness);
            } break;
        case 15: {
            status = ast_read_primitive(rt,ast_int32,15,&ncinq_var_all_return_v->options_mask);
            } break;
        case 16: {
            status = ast_read_primitive(rt,ast_int32,16,&ncinq_var_all_return_v->pixels_per_block);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_var_all_return_vp) *ncinq_var_all_return_vp = ncinq_var_all_return_v;
done:
    return ACATCH(status);
} /*NCInq_var_all_Return_read*/

ast_err
NCInq_var_all_Return_reclaim(ast_runtime* rt, NCInq_var_all_Return* ncinq_var_all_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncinq_var_all_return_v->fill_value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_var_all_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_var_all_Return_reclaim*/

size_t
NCInq_var_all_Return_get_size(ast_runtime* rt, NCInq_var_all_Return* ncinq_var_all_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->xtype);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->ndims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_var_all_return_v->dimids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->dimids.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->natts);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->shuffle);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,7);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->deflate);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,8);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->deflate_level);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,9);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->fletcher32);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,10);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->contiguous);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_var_all_return_v->chunksizes.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,11);
            fieldsize += ast_get_size(rt,ast_uint64,&ncinq_var_all_return_v->chunksizes.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,12);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->no_fill);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,13);
        fieldsize += ast_get_size(rt,ast_bytes,&ncinq_var_all_return_v->fill_value);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,14);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_var_all_return_v->endianness);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,15);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->options_mask);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,16);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_var_all_return_v->pixels_per_block);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_var_all_Return_get_size*/

ast_err
NCShow_metadata_write(ast_runtime* rt, NCShow_metadata* ncshow_metadata_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncshow_metadata_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCShow_metadata_write*/

ast_err
NCShow_metadata_read(ast_runtime* rt, NCShow_metadata** ncshow_metadata_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCShow_metadata* ncshow_metadata_v;
    unsigned long pos;

    ncshow_metadata_v = (NCShow_metadata*)ast_alloc(rt,sizeof(NCShow_metadata));
    if(ncshow_metadata_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCShow_metadata|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncshow_metadata_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncshow_metadata_vp) *ncshow_metadata_vp = ncshow_metadata_v;
done:
    return ACATCH(status);
} /*NCShow_metadata_read*/

ast_err
NCShow_metadata_reclaim(ast_runtime* rt, NCShow_metadata* ncshow_metadata_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncshow_metadata_v);
    goto done;

done:
    return ACATCH(status);

} /*NCShow_metadata_reclaim*/

size_t
NCShow_metadata_get_size(ast_runtime* rt, NCShow_metadata* ncshow_metadata_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncshow_metadata_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCShow_metadata_get_size*/

ast_err
NCShow_metadata_Return_write(ast_runtime* rt, NCShow_metadata_Return* ncshow_metadata_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncshow_metadata_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCShow_metadata_Return_write*/

ast_err
NCShow_metadata_Return_read(ast_runtime* rt, NCShow_metadata_Return** ncshow_metadata_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCShow_metadata_Return* ncshow_metadata_return_v;
    unsigned long pos;

    ncshow_metadata_return_v = (NCShow_metadata_Return*)ast_alloc(rt,sizeof(NCShow_metadata_Return));
    if(ncshow_metadata_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCShow_metadata_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncshow_metadata_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncshow_metadata_return_vp) *ncshow_metadata_return_vp = ncshow_metadata_return_v;
done:
    return ACATCH(status);
} /*NCShow_metadata_Return_read*/

ast_err
NCShow_metadata_Return_reclaim(ast_runtime* rt, NCShow_metadata_Return* ncshow_metadata_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncshow_metadata_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCShow_metadata_Return_reclaim*/

size_t
NCShow_metadata_Return_get_size(ast_runtime* rt, NCShow_metadata_Return* ncshow_metadata_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncshow_metadata_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCShow_metadata_Return_get_size*/

ast_err
NCInq_unlimdims_write(ast_runtime* rt, NCInq_unlimdims* ncinq_unlimdims_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_unlimdims_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_unlimdims_write*/

ast_err
NCInq_unlimdims_read(ast_runtime* rt, NCInq_unlimdims** ncinq_unlimdims_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_unlimdims* ncinq_unlimdims_v;
    unsigned long pos;

    ncinq_unlimdims_v = (NCInq_unlimdims*)ast_alloc(rt,sizeof(NCInq_unlimdims));
    if(ncinq_unlimdims_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_unlimdims|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_unlimdims_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_unlimdims_vp) *ncinq_unlimdims_vp = ncinq_unlimdims_v;
done:
    return ACATCH(status);
} /*NCInq_unlimdims_read*/

ast_err
NCInq_unlimdims_reclaim(ast_runtime* rt, NCInq_unlimdims* ncinq_unlimdims_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_unlimdims_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_unlimdims_reclaim*/

size_t
NCInq_unlimdims_get_size(ast_runtime* rt, NCInq_unlimdims* ncinq_unlimdims_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdims_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_unlimdims_get_size*/

ast_err
NCInq_unlimdims_Return_write(ast_runtime* rt, NCInq_unlimdims_Return* ncinq_unlimdims_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_unlimdims_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_unlimdims_return_v->nunlimdims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_unlimdims_return_v->unlimdimids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,3,&ncinq_unlimdims_return_v->unlimdimids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_unlimdims_Return_write*/

ast_err
NCInq_unlimdims_Return_read(ast_runtime* rt, NCInq_unlimdims_Return** ncinq_unlimdims_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_unlimdims_Return* ncinq_unlimdims_return_v;
    unsigned long pos;

    ncinq_unlimdims_return_v = (NCInq_unlimdims_Return*)ast_alloc(rt,sizeof(NCInq_unlimdims_Return));
    if(ncinq_unlimdims_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_unlimdims_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_unlimdims_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_unlimdims_return_v->nunlimdims);
            } break;
        case 3: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_unlimdims_return_v->unlimdimids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_unlimdims_return_vp) *ncinq_unlimdims_return_vp = ncinq_unlimdims_return_v;
done:
    return ACATCH(status);
} /*NCInq_unlimdims_Return_read*/

ast_err
NCInq_unlimdims_Return_reclaim(ast_runtime* rt, NCInq_unlimdims_Return* ncinq_unlimdims_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_unlimdims_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_unlimdims_Return_reclaim*/

size_t
NCInq_unlimdims_Return_get_size(ast_runtime* rt, NCInq_unlimdims_Return* ncinq_unlimdims_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdims_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdims_return_v->nunlimdims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_unlimdims_return_v->unlimdimids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_unlimdims_return_v->unlimdimids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_unlimdims_Return_get_size*/

ast_err
NCVar_par_access_write(ast_runtime* rt, NCVar_par_access* ncvar_par_access_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncvar_par_access_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncvar_par_access_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncvar_par_access_v->par_access);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCVar_par_access_write*/

ast_err
NCVar_par_access_read(ast_runtime* rt, NCVar_par_access** ncvar_par_access_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCVar_par_access* ncvar_par_access_v;
    unsigned long pos;

    ncvar_par_access_v = (NCVar_par_access*)ast_alloc(rt,sizeof(NCVar_par_access));
    if(ncvar_par_access_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCVar_par_access|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncvar_par_access_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncvar_par_access_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncvar_par_access_v->par_access);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncvar_par_access_vp) *ncvar_par_access_vp = ncvar_par_access_v;
done:
    return ACATCH(status);
} /*NCVar_par_access_read*/

ast_err
NCVar_par_access_reclaim(ast_runtime* rt, NCVar_par_access* ncvar_par_access_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncvar_par_access_v);
    goto done;

done:
    return ACATCH(status);

} /*NCVar_par_access_reclaim*/

size_t
NCVar_par_access_get_size(ast_runtime* rt, NCVar_par_access* ncvar_par_access_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncvar_par_access_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncvar_par_access_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncvar_par_access_v->par_access);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCVar_par_access_get_size*/

ast_err
NCVar_par_access_Return_write(ast_runtime* rt, NCVar_par_access_Return* ncvar_par_access_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncvar_par_access_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCVar_par_access_Return_write*/

ast_err
NCVar_par_access_Return_read(ast_runtime* rt, NCVar_par_access_Return** ncvar_par_access_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCVar_par_access_Return* ncvar_par_access_return_v;
    unsigned long pos;

    ncvar_par_access_return_v = (NCVar_par_access_Return*)ast_alloc(rt,sizeof(NCVar_par_access_Return));
    if(ncvar_par_access_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCVar_par_access_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncvar_par_access_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncvar_par_access_return_vp) *ncvar_par_access_return_vp = ncvar_par_access_return_v;
done:
    return ACATCH(status);
} /*NCVar_par_access_Return_read*/

ast_err
NCVar_par_access_Return_reclaim(ast_runtime* rt, NCVar_par_access_Return* ncvar_par_access_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncvar_par_access_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCVar_par_access_Return_reclaim*/

size_t
NCVar_par_access_Return_get_size(ast_runtime* rt, NCVar_par_access_Return* ncvar_par_access_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncvar_par_access_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCVar_par_access_Return_get_size*/

ast_err
NCInq_ncid_write(ast_runtime* rt, NCInq_ncid* ncinq_ncid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_ncid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_ncid_v->group);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_ncid_write*/

ast_err
NCInq_ncid_read(ast_runtime* rt, NCInq_ncid** ncinq_ncid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_ncid* ncinq_ncid_v;
    unsigned long pos;

    ncinq_ncid_v = (NCInq_ncid*)ast_alloc(rt,sizeof(NCInq_ncid));
    if(ncinq_ncid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_ncid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_ncid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_ncid_v->group);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_ncid_vp) *ncinq_ncid_vp = ncinq_ncid_v;
done:
    return ACATCH(status);
} /*NCInq_ncid_read*/

ast_err
NCInq_ncid_reclaim(ast_runtime* rt, NCInq_ncid* ncinq_ncid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_ncid_v->group);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_ncid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_ncid_reclaim*/

size_t
NCInq_ncid_get_size(ast_runtime* rt, NCInq_ncid* ncinq_ncid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_ncid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_ncid_v->group);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_ncid_get_size*/

ast_err
NCInq_ncid_Return_write(ast_runtime* rt, NCInq_ncid_Return* ncinq_ncid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_ncid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_ncid_return_v->grp_ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_ncid_Return_write*/

ast_err
NCInq_ncid_Return_read(ast_runtime* rt, NCInq_ncid_Return** ncinq_ncid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_ncid_Return* ncinq_ncid_return_v;
    unsigned long pos;

    ncinq_ncid_return_v = (NCInq_ncid_Return*)ast_alloc(rt,sizeof(NCInq_ncid_Return));
    if(ncinq_ncid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_ncid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_ncid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_ncid_return_v->grp_ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_ncid_return_vp) *ncinq_ncid_return_vp = ncinq_ncid_return_v;
done:
    return ACATCH(status);
} /*NCInq_ncid_Return_read*/

ast_err
NCInq_ncid_Return_reclaim(ast_runtime* rt, NCInq_ncid_Return* ncinq_ncid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_ncid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_ncid_Return_reclaim*/

size_t
NCInq_ncid_Return_get_size(ast_runtime* rt, NCInq_ncid_Return* ncinq_ncid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_ncid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_ncid_return_v->grp_ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_ncid_Return_get_size*/

ast_err
NCInq_grps_write(ast_runtime* rt, NCInq_grps* ncinq_grps_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grps_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grps_write*/

ast_err
NCInq_grps_read(ast_runtime* rt, NCInq_grps** ncinq_grps_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grps* ncinq_grps_v;
    unsigned long pos;

    ncinq_grps_v = (NCInq_grps*)ast_alloc(rt,sizeof(NCInq_grps));
    if(ncinq_grps_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grps|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grps_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grps_vp) *ncinq_grps_vp = ncinq_grps_v;
done:
    return ACATCH(status);
} /*NCInq_grps_read*/

ast_err
NCInq_grps_reclaim(ast_runtime* rt, NCInq_grps* ncinq_grps_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grps_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grps_reclaim*/

size_t
NCInq_grps_get_size(ast_runtime* rt, NCInq_grps* ncinq_grps_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grps_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grps_get_size*/

ast_err
NCInq_grps_Return_write(ast_runtime* rt, NCInq_grps_Return* ncinq_grps_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grps_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_grps_return_v->ngroups);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_grps_return_v->ncids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,3,&ncinq_grps_return_v->ncids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_grps_Return_write*/

ast_err
NCInq_grps_Return_read(ast_runtime* rt, NCInq_grps_Return** ncinq_grps_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grps_Return* ncinq_grps_return_v;
    unsigned long pos;

    ncinq_grps_return_v = (NCInq_grps_Return*)ast_alloc(rt,sizeof(NCInq_grps_Return));
    if(ncinq_grps_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grps_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grps_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_grps_return_v->ngroups);
            } break;
        case 3: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_grps_return_v->ncids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grps_return_vp) *ncinq_grps_return_vp = ncinq_grps_return_v;
done:
    return ACATCH(status);
} /*NCInq_grps_Return_read*/

ast_err
NCInq_grps_Return_reclaim(ast_runtime* rt, NCInq_grps_Return* ncinq_grps_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grps_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grps_Return_reclaim*/

size_t
NCInq_grps_Return_get_size(ast_runtime* rt, NCInq_grps_Return* ncinq_grps_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grps_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grps_return_v->ngroups);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_grps_return_v->ncids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_grps_return_v->ncids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grps_Return_get_size*/

ast_err
NCInq_grpname_write(ast_runtime* rt, NCInq_grpname* ncinq_grpname_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grpname_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grpname_write*/

ast_err
NCInq_grpname_read(ast_runtime* rt, NCInq_grpname** ncinq_grpname_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grpname* ncinq_grpname_v;
    unsigned long pos;

    ncinq_grpname_v = (NCInq_grpname*)ast_alloc(rt,sizeof(NCInq_grpname));
    if(ncinq_grpname_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grpname|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grpname_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grpname_vp) *ncinq_grpname_vp = ncinq_grpname_v;
done:
    return ACATCH(status);
} /*NCInq_grpname_read*/

ast_err
NCInq_grpname_reclaim(ast_runtime* rt, NCInq_grpname* ncinq_grpname_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grpname_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grpname_reclaim*/

size_t
NCInq_grpname_get_size(ast_runtime* rt, NCInq_grpname* ncinq_grpname_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grpname_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grpname_get_size*/

ast_err
NCInq_grpname_Return_write(ast_runtime* rt, NCInq_grpname_Return* ncinq_grpname_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grpname_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_grpname_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grpname_Return_write*/

ast_err
NCInq_grpname_Return_read(ast_runtime* rt, NCInq_grpname_Return** ncinq_grpname_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grpname_Return* ncinq_grpname_return_v;
    unsigned long pos;

    ncinq_grpname_return_v = (NCInq_grpname_Return*)ast_alloc(rt,sizeof(NCInq_grpname_Return));
    if(ncinq_grpname_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grpname_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grpname_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_grpname_return_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grpname_return_vp) *ncinq_grpname_return_vp = ncinq_grpname_return_v;
done:
    return ACATCH(status);
} /*NCInq_grpname_Return_read*/

ast_err
NCInq_grpname_Return_reclaim(ast_runtime* rt, NCInq_grpname_Return* ncinq_grpname_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_grpname_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_grpname_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grpname_Return_reclaim*/

size_t
NCInq_grpname_Return_get_size(ast_runtime* rt, NCInq_grpname_Return* ncinq_grpname_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grpname_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_grpname_return_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grpname_Return_get_size*/

ast_err
NCInq_grpname_full_write(ast_runtime* rt, NCInq_grpname_full* ncinq_grpname_full_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grpname_full_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grpname_full_write*/

ast_err
NCInq_grpname_full_read(ast_runtime* rt, NCInq_grpname_full** ncinq_grpname_full_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grpname_full* ncinq_grpname_full_v;
    unsigned long pos;

    ncinq_grpname_full_v = (NCInq_grpname_full*)ast_alloc(rt,sizeof(NCInq_grpname_full));
    if(ncinq_grpname_full_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grpname_full|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grpname_full_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grpname_full_vp) *ncinq_grpname_full_vp = ncinq_grpname_full_v;
done:
    return ACATCH(status);
} /*NCInq_grpname_full_read*/

ast_err
NCInq_grpname_full_reclaim(ast_runtime* rt, NCInq_grpname_full* ncinq_grpname_full_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grpname_full_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grpname_full_reclaim*/

size_t
NCInq_grpname_full_get_size(ast_runtime* rt, NCInq_grpname_full* ncinq_grpname_full_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grpname_full_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grpname_full_get_size*/

ast_err
NCInq_grpname_full_Return_write(ast_runtime* rt, NCInq_grpname_full_Return* ncinq_grpname_full_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grpname_full_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_grpname_full_return_v->len.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,2,&ncinq_grpname_full_return_v->len.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinq_grpname_full_return_v->fullname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grpname_full_Return_write*/

ast_err
NCInq_grpname_full_Return_read(ast_runtime* rt, NCInq_grpname_full_Return** ncinq_grpname_full_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grpname_full_Return* ncinq_grpname_full_return_v;
    unsigned long pos;

    ncinq_grpname_full_return_v = (NCInq_grpname_full_Return*)ast_alloc(rt,sizeof(NCInq_grpname_full_Return));
    if(ncinq_grpname_full_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grpname_full_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grpname_full_return_v->ncstatus);
            } break;
        case 2: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,2,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncinq_grpname_full_return_v->len,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinq_grpname_full_return_v->fullname);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grpname_full_return_vp) *ncinq_grpname_full_return_vp = ncinq_grpname_full_return_v;
done:
    return ACATCH(status);
} /*NCInq_grpname_full_Return_read*/

ast_err
NCInq_grpname_full_Return_reclaim(ast_runtime* rt, NCInq_grpname_full_Return* ncinq_grpname_full_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_grpname_full_return_v->fullname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_grpname_full_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grpname_full_Return_reclaim*/

size_t
NCInq_grpname_full_Return_get_size(ast_runtime* rt, NCInq_grpname_full_Return* ncinq_grpname_full_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grpname_full_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_grpname_full_return_v->len.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_uint64,&ncinq_grpname_full_return_v->len.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_grpname_full_return_v->fullname);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grpname_full_Return_get_size*/

ast_err
NCInq_grp_parent_write(ast_runtime* rt, NCInq_grp_parent* ncinq_grp_parent_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grp_parent_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grp_parent_write*/

ast_err
NCInq_grp_parent_read(ast_runtime* rt, NCInq_grp_parent** ncinq_grp_parent_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grp_parent* ncinq_grp_parent_v;
    unsigned long pos;

    ncinq_grp_parent_v = (NCInq_grp_parent*)ast_alloc(rt,sizeof(NCInq_grp_parent));
    if(ncinq_grp_parent_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grp_parent|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grp_parent_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grp_parent_vp) *ncinq_grp_parent_vp = ncinq_grp_parent_v;
done:
    return ACATCH(status);
} /*NCInq_grp_parent_read*/

ast_err
NCInq_grp_parent_reclaim(ast_runtime* rt, NCInq_grp_parent* ncinq_grp_parent_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grp_parent_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grp_parent_reclaim*/

size_t
NCInq_grp_parent_get_size(ast_runtime* rt, NCInq_grp_parent* ncinq_grp_parent_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_parent_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grp_parent_get_size*/

ast_err
NCInq_grp_parent_Return_write(ast_runtime* rt, NCInq_grp_parent_Return* ncinq_grp_parent_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grp_parent_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_grp_parent_return_v->parentncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grp_parent_Return_write*/

ast_err
NCInq_grp_parent_Return_read(ast_runtime* rt, NCInq_grp_parent_Return** ncinq_grp_parent_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grp_parent_Return* ncinq_grp_parent_return_v;
    unsigned long pos;

    ncinq_grp_parent_return_v = (NCInq_grp_parent_Return*)ast_alloc(rt,sizeof(NCInq_grp_parent_Return));
    if(ncinq_grp_parent_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grp_parent_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grp_parent_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_grp_parent_return_v->parentncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grp_parent_return_vp) *ncinq_grp_parent_return_vp = ncinq_grp_parent_return_v;
done:
    return ACATCH(status);
} /*NCInq_grp_parent_Return_read*/

ast_err
NCInq_grp_parent_Return_reclaim(ast_runtime* rt, NCInq_grp_parent_Return* ncinq_grp_parent_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grp_parent_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grp_parent_Return_reclaim*/

size_t
NCInq_grp_parent_Return_get_size(ast_runtime* rt, NCInq_grp_parent_Return* ncinq_grp_parent_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_parent_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_parent_return_v->parentncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grp_parent_Return_get_size*/

ast_err
NCInq_grp_full_ncid_write(ast_runtime* rt, NCInq_grp_full_ncid* ncinq_grp_full_ncid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grp_full_ncid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_grp_full_ncid_v->fullname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grp_full_ncid_write*/

ast_err
NCInq_grp_full_ncid_read(ast_runtime* rt, NCInq_grp_full_ncid** ncinq_grp_full_ncid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grp_full_ncid* ncinq_grp_full_ncid_v;
    unsigned long pos;

    ncinq_grp_full_ncid_v = (NCInq_grp_full_ncid*)ast_alloc(rt,sizeof(NCInq_grp_full_ncid));
    if(ncinq_grp_full_ncid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grp_full_ncid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grp_full_ncid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_grp_full_ncid_v->fullname);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grp_full_ncid_vp) *ncinq_grp_full_ncid_vp = ncinq_grp_full_ncid_v;
done:
    return ACATCH(status);
} /*NCInq_grp_full_ncid_read*/

ast_err
NCInq_grp_full_ncid_reclaim(ast_runtime* rt, NCInq_grp_full_ncid* ncinq_grp_full_ncid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_grp_full_ncid_v->fullname);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_grp_full_ncid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grp_full_ncid_reclaim*/

size_t
NCInq_grp_full_ncid_get_size(ast_runtime* rt, NCInq_grp_full_ncid* ncinq_grp_full_ncid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_full_ncid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_grp_full_ncid_v->fullname);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grp_full_ncid_get_size*/

ast_err
NCInq_grp_full_ncid_Return_write(ast_runtime* rt, NCInq_grp_full_ncid_Return* ncinq_grp_full_ncid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_grp_full_ncid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_grp_full_ncid_return_v->groupncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_grp_full_ncid_Return_write*/

ast_err
NCInq_grp_full_ncid_Return_read(ast_runtime* rt, NCInq_grp_full_ncid_Return** ncinq_grp_full_ncid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_grp_full_ncid_Return* ncinq_grp_full_ncid_return_v;
    unsigned long pos;

    ncinq_grp_full_ncid_return_v = (NCInq_grp_full_ncid_Return*)ast_alloc(rt,sizeof(NCInq_grp_full_ncid_Return));
    if(ncinq_grp_full_ncid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_grp_full_ncid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_grp_full_ncid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_grp_full_ncid_return_v->groupncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_grp_full_ncid_return_vp) *ncinq_grp_full_ncid_return_vp = ncinq_grp_full_ncid_return_v;
done:
    return ACATCH(status);
} /*NCInq_grp_full_ncid_Return_read*/

ast_err
NCInq_grp_full_ncid_Return_reclaim(ast_runtime* rt, NCInq_grp_full_ncid_Return* ncinq_grp_full_ncid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_grp_full_ncid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_grp_full_ncid_Return_reclaim*/

size_t
NCInq_grp_full_ncid_Return_get_size(ast_runtime* rt, NCInq_grp_full_ncid_Return* ncinq_grp_full_ncid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_full_ncid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_grp_full_ncid_return_v->groupncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_grp_full_ncid_Return_get_size*/

ast_err
NCInq_varids_write(ast_runtime* rt, NCInq_varids* ncinq_varids_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_varids_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_varids_write*/

ast_err
NCInq_varids_read(ast_runtime* rt, NCInq_varids** ncinq_varids_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_varids* ncinq_varids_v;
    unsigned long pos;

    ncinq_varids_v = (NCInq_varids*)ast_alloc(rt,sizeof(NCInq_varids));
    if(ncinq_varids_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_varids|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_varids_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_varids_vp) *ncinq_varids_vp = ncinq_varids_v;
done:
    return ACATCH(status);
} /*NCInq_varids_read*/

ast_err
NCInq_varids_reclaim(ast_runtime* rt, NCInq_varids* ncinq_varids_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_varids_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_varids_reclaim*/

size_t
NCInq_varids_get_size(ast_runtime* rt, NCInq_varids* ncinq_varids_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varids_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_varids_get_size*/

ast_err
NCInq_varids_Return_write(ast_runtime* rt, NCInq_varids_Return* ncinq_varids_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_varids_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_varids_return_v->nvars);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_varids_return_v->varids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,3,&ncinq_varids_return_v->varids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_varids_Return_write*/

ast_err
NCInq_varids_Return_read(ast_runtime* rt, NCInq_varids_Return** ncinq_varids_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_varids_Return* ncinq_varids_return_v;
    unsigned long pos;

    ncinq_varids_return_v = (NCInq_varids_Return*)ast_alloc(rt,sizeof(NCInq_varids_Return));
    if(ncinq_varids_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_varids_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_varids_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_varids_return_v->nvars);
            } break;
        case 3: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_varids_return_v->varids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_varids_return_vp) *ncinq_varids_return_vp = ncinq_varids_return_v;
done:
    return ACATCH(status);
} /*NCInq_varids_Return_read*/

ast_err
NCInq_varids_Return_reclaim(ast_runtime* rt, NCInq_varids_Return* ncinq_varids_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_varids_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_varids_Return_reclaim*/

size_t
NCInq_varids_Return_get_size(ast_runtime* rt, NCInq_varids_Return* ncinq_varids_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varids_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_varids_return_v->nvars);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_varids_return_v->varids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_varids_return_v->varids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_varids_Return_get_size*/

ast_err
NCInq_dimids_write(ast_runtime* rt, NCInq_dimids* ncinq_dimids_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dimids_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,2,&ncinq_dimids_v->includeparents);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_dimids_write*/

ast_err
NCInq_dimids_read(ast_runtime* rt, NCInq_dimids** ncinq_dimids_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dimids* ncinq_dimids_v;
    unsigned long pos;

    ncinq_dimids_v = (NCInq_dimids*)ast_alloc(rt,sizeof(NCInq_dimids));
    if(ncinq_dimids_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dimids|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dimids_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bool,2,&ncinq_dimids_v->includeparents);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dimids_vp) *ncinq_dimids_vp = ncinq_dimids_v;
done:
    return ACATCH(status);
} /*NCInq_dimids_read*/

ast_err
NCInq_dimids_reclaim(ast_runtime* rt, NCInq_dimids* ncinq_dimids_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_dimids_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dimids_reclaim*/

size_t
NCInq_dimids_get_size(ast_runtime* rt, NCInq_dimids* ncinq_dimids_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimids_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_dimids_v->includeparents);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dimids_get_size*/

ast_err
NCInq_dimids_Return_write(ast_runtime* rt, NCInq_dimids_Return* ncinq_dimids_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_dimids_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_dimids_return_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_dimids_return_v->dimids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,3,&ncinq_dimids_return_v->dimids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_dimids_Return_write*/

ast_err
NCInq_dimids_Return_read(ast_runtime* rt, NCInq_dimids_Return** ncinq_dimids_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_dimids_Return* ncinq_dimids_return_v;
    unsigned long pos;

    ncinq_dimids_return_v = (NCInq_dimids_Return*)ast_alloc(rt,sizeof(NCInq_dimids_Return));
    if(ncinq_dimids_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_dimids_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_dimids_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_dimids_return_v->ndims);
            } break;
        case 3: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_dimids_return_v->dimids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_dimids_return_vp) *ncinq_dimids_return_vp = ncinq_dimids_return_v;
done:
    return ACATCH(status);
} /*NCInq_dimids_Return_read*/

ast_err
NCInq_dimids_Return_reclaim(ast_runtime* rt, NCInq_dimids_Return* ncinq_dimids_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_dimids_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_dimids_Return_reclaim*/

size_t
NCInq_dimids_Return_get_size(ast_runtime* rt, NCInq_dimids_Return* ncinq_dimids_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimids_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimids_return_v->ndims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_dimids_return_v->dimids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_dimids_return_v->dimids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_dimids_Return_get_size*/

ast_err
NCInq_typeids_write(ast_runtime* rt, NCInq_typeids* ncinq_typeids_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_typeids_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_typeids_write*/

ast_err
NCInq_typeids_read(ast_runtime* rt, NCInq_typeids** ncinq_typeids_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_typeids* ncinq_typeids_v;
    unsigned long pos;

    ncinq_typeids_v = (NCInq_typeids*)ast_alloc(rt,sizeof(NCInq_typeids));
    if(ncinq_typeids_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_typeids|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_typeids_v->ncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_typeids_vp) *ncinq_typeids_vp = ncinq_typeids_v;
done:
    return ACATCH(status);
} /*NCInq_typeids_read*/

ast_err
NCInq_typeids_reclaim(ast_runtime* rt, NCInq_typeids* ncinq_typeids_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_typeids_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_typeids_reclaim*/

size_t
NCInq_typeids_get_size(ast_runtime* rt, NCInq_typeids* ncinq_typeids_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeids_v->ncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_typeids_get_size*/

ast_err
NCInq_typeids_Return_write(ast_runtime* rt, NCInq_typeids_Return* ncinq_typeids_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_typeids_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_typeids_return_v->ntypes);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_typeids_return_v->typeids.count;i++) {
            status = ast_write_primitive(rt,ast_int32,3,&ncinq_typeids_return_v->typeids.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_typeids_Return_write*/

ast_err
NCInq_typeids_Return_read(ast_runtime* rt, NCInq_typeids_Return** ncinq_typeids_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_typeids_Return* ncinq_typeids_return_v;
    unsigned long pos;

    ncinq_typeids_return_v = (NCInq_typeids_Return*)ast_alloc(rt,sizeof(NCInq_typeids_Return));
    if(ncinq_typeids_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_typeids_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_typeids_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_typeids_return_v->ntypes);
            } break;
        case 3: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_typeids_return_v->typeids,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_typeids_return_vp) *ncinq_typeids_return_vp = ncinq_typeids_return_v;
done:
    return ACATCH(status);
} /*NCInq_typeids_Return_read*/

ast_err
NCInq_typeids_Return_reclaim(ast_runtime* rt, NCInq_typeids_Return* ncinq_typeids_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_typeids_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_typeids_Return_reclaim*/

size_t
NCInq_typeids_Return_get_size(ast_runtime* rt, NCInq_typeids_Return* ncinq_typeids_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeids_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeids_return_v->ntypes);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_typeids_return_v->typeids.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeids_return_v->typeids.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_typeids_Return_get_size*/

ast_err
NCInq_type_equal_write(ast_runtime* rt, NCInq_type_equal* ncinq_type_equal_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_type_equal_v->ncid1);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_type_equal_v->typeid1);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_type_equal_v->ncid2);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncinq_type_equal_v->typeid2);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_type_equal_write*/

ast_err
NCInq_type_equal_read(ast_runtime* rt, NCInq_type_equal** ncinq_type_equal_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_type_equal* ncinq_type_equal_v;
    unsigned long pos;

    ncinq_type_equal_v = (NCInq_type_equal*)ast_alloc(rt,sizeof(NCInq_type_equal));
    if(ncinq_type_equal_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_type_equal|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_type_equal_v->ncid1);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_type_equal_v->typeid1);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_type_equal_v->ncid2);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncinq_type_equal_v->typeid2);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_type_equal_vp) *ncinq_type_equal_vp = ncinq_type_equal_v;
done:
    return ACATCH(status);
} /*NCInq_type_equal_read*/

ast_err
NCInq_type_equal_reclaim(ast_runtime* rt, NCInq_type_equal* ncinq_type_equal_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_type_equal_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_type_equal_reclaim*/

size_t
NCInq_type_equal_get_size(ast_runtime* rt, NCInq_type_equal* ncinq_type_equal_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_equal_v->ncid1);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_equal_v->typeid1);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_equal_v->ncid2);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_equal_v->typeid2);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_type_equal_get_size*/

ast_err
NCInq_type_equal_Return_write(ast_runtime* rt, NCInq_type_equal_Return* ncinq_type_equal_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_type_equal_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,2,&ncinq_type_equal_return_v->equal);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_type_equal_Return_write*/

ast_err
NCInq_type_equal_Return_read(ast_runtime* rt, NCInq_type_equal_Return** ncinq_type_equal_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_type_equal_Return* ncinq_type_equal_return_v;
    unsigned long pos;

    ncinq_type_equal_return_v = (NCInq_type_equal_Return*)ast_alloc(rt,sizeof(NCInq_type_equal_Return));
    if(ncinq_type_equal_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_type_equal_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_type_equal_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bool,2,&ncinq_type_equal_return_v->equal);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_type_equal_return_vp) *ncinq_type_equal_return_vp = ncinq_type_equal_return_v;
done:
    return ACATCH(status);
} /*NCInq_type_equal_Return_read*/

ast_err
NCInq_type_equal_Return_reclaim(ast_runtime* rt, NCInq_type_equal_Return* ncinq_type_equal_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_type_equal_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_type_equal_Return_reclaim*/

size_t
NCInq_type_equal_Return_get_size(ast_runtime* rt, NCInq_type_equal_Return* ncinq_type_equal_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_type_equal_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bool,&ncinq_type_equal_return_v->equal);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_type_equal_Return_get_size*/

ast_err
NCDef_Grp_write(ast_runtime* rt, NCDef_Grp* ncdef_grp_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_grp_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncdef_grp_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Grp_write*/

ast_err
NCDef_Grp_read(ast_runtime* rt, NCDef_Grp** ncdef_grp_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Grp* ncdef_grp_v;
    unsigned long pos;

    ncdef_grp_v = (NCDef_Grp*)ast_alloc(rt,sizeof(NCDef_Grp));
    if(ncdef_grp_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Grp|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_grp_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncdef_grp_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_grp_vp) *ncdef_grp_vp = ncdef_grp_v;
done:
    return ACATCH(status);
} /*NCDef_Grp_read*/

ast_err
NCDef_Grp_reclaim(ast_runtime* rt, NCDef_Grp* ncdef_grp_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_grp_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_grp_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Grp_reclaim*/

size_t
NCDef_Grp_get_size(ast_runtime* rt, NCDef_Grp* ncdef_grp_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_grp_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_grp_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Grp_get_size*/

ast_err
NCDef_Grp_Return_write(ast_runtime* rt, NCDef_Grp_Return* ncdef_grp_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_grp_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_grp_return_v->grpncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Grp_Return_write*/

ast_err
NCDef_Grp_Return_read(ast_runtime* rt, NCDef_Grp_Return** ncdef_grp_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Grp_Return* ncdef_grp_return_v;
    unsigned long pos;

    ncdef_grp_return_v = (NCDef_Grp_Return*)ast_alloc(rt,sizeof(NCDef_Grp_Return));
    if(ncdef_grp_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Grp_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_grp_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_grp_return_v->grpncid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_grp_return_vp) *ncdef_grp_return_vp = ncdef_grp_return_v;
done:
    return ACATCH(status);
} /*NCDef_Grp_Return_read*/

ast_err
NCDef_Grp_Return_reclaim(ast_runtime* rt, NCDef_Grp_Return* ncdef_grp_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_grp_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Grp_Return_reclaim*/

size_t
NCDef_Grp_Return_get_size(ast_runtime* rt, NCDef_Grp_Return* ncdef_grp_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_grp_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_grp_return_v->grpncid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Grp_Return_get_size*/

ast_err
NCInq_user_type_write(ast_runtime* rt, NCInq_user_type* ncinq_user_type_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_user_type_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_user_type_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_user_type_write*/

ast_err
NCInq_user_type_read(ast_runtime* rt, NCInq_user_type** ncinq_user_type_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_user_type* ncinq_user_type_v;
    unsigned long pos;

    ncinq_user_type_v = (NCInq_user_type*)ast_alloc(rt,sizeof(NCInq_user_type));
    if(ncinq_user_type_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_user_type|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_user_type_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_user_type_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_user_type_vp) *ncinq_user_type_vp = ncinq_user_type_v;
done:
    return ACATCH(status);
} /*NCInq_user_type_read*/

ast_err
NCInq_user_type_reclaim(ast_runtime* rt, NCInq_user_type* ncinq_user_type_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_user_type_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_user_type_reclaim*/

size_t
NCInq_user_type_get_size(ast_runtime* rt, NCInq_user_type* ncinq_user_type_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_user_type_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_user_type_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_user_type_get_size*/

ast_err
NCInq_user_type_Return_write(ast_runtime* rt, NCInq_user_type_Return* ncinq_user_type_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_user_type_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_user_type_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_user_type_return_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncinq_user_type_return_v->basetype);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,5,&ncinq_user_type_return_v->nfields);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,6,&ncinq_user_type_return_v->typeclass);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_user_type_Return_write*/

ast_err
NCInq_user_type_Return_read(ast_runtime* rt, NCInq_user_type_Return** ncinq_user_type_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_user_type_Return* ncinq_user_type_return_v;
    unsigned long pos;

    ncinq_user_type_return_v = (NCInq_user_type_Return*)ast_alloc(rt,sizeof(NCInq_user_type_Return));
    if(ncinq_user_type_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_user_type_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_user_type_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_user_type_return_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_user_type_return_v->size);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncinq_user_type_return_v->basetype);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_uint64,5,&ncinq_user_type_return_v->nfields);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_int32,6,&ncinq_user_type_return_v->typeclass);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_user_type_return_vp) *ncinq_user_type_return_vp = ncinq_user_type_return_v;
done:
    return ACATCH(status);
} /*NCInq_user_type_Return_read*/

ast_err
NCInq_user_type_Return_reclaim(ast_runtime* rt, NCInq_user_type_Return* ncinq_user_type_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_user_type_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_user_type_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_user_type_Return_reclaim*/

size_t
NCInq_user_type_Return_get_size(ast_runtime* rt, NCInq_user_type_Return* ncinq_user_type_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_user_type_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_user_type_return_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_user_type_return_v->size);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_user_type_return_v->basetype);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_user_type_return_v->nfields);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_user_type_return_v->typeclass);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_user_type_Return_get_size*/

ast_err
NCInq_typeid_write(ast_runtime* rt, NCInq_typeid* ncinq_typeid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_typeid_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_typeid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_typeid_write*/

ast_err
NCInq_typeid_read(ast_runtime* rt, NCInq_typeid** ncinq_typeid_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_typeid* ncinq_typeid_v;
    unsigned long pos;

    ncinq_typeid_v = (NCInq_typeid*)ast_alloc(rt,sizeof(NCInq_typeid));
    if(ncinq_typeid_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_typeid|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_typeid_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_typeid_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_typeid_vp) *ncinq_typeid_vp = ncinq_typeid_v;
done:
    return ACATCH(status);
} /*NCInq_typeid_read*/

ast_err
NCInq_typeid_reclaim(ast_runtime* rt, NCInq_typeid* ncinq_typeid_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_typeid_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_typeid_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_typeid_reclaim*/

size_t
NCInq_typeid_get_size(ast_runtime* rt, NCInq_typeid* ncinq_typeid_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeid_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_typeid_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_typeid_get_size*/

ast_err
NCInq_typeid_Return_write(ast_runtime* rt, NCInq_typeid_Return* ncinq_typeid_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_typeid_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_typeid_return_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_typeid_Return_write*/

ast_err
NCInq_typeid_Return_read(ast_runtime* rt, NCInq_typeid_Return** ncinq_typeid_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_typeid_Return* ncinq_typeid_return_v;
    unsigned long pos;

    ncinq_typeid_return_v = (NCInq_typeid_Return*)ast_alloc(rt,sizeof(NCInq_typeid_Return));
    if(ncinq_typeid_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_typeid_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_typeid_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_typeid_return_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_typeid_return_vp) *ncinq_typeid_return_vp = ncinq_typeid_return_v;
done:
    return ACATCH(status);
} /*NCInq_typeid_Return_read*/

ast_err
NCInq_typeid_Return_reclaim(ast_runtime* rt, NCInq_typeid_Return* ncinq_typeid_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_typeid_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_typeid_Return_reclaim*/

size_t
NCInq_typeid_Return_get_size(ast_runtime* rt, NCInq_typeid_Return* ncinq_typeid_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeid_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_typeid_return_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_typeid_Return_get_size*/

ast_err
NCDef_Compound_write(ast_runtime* rt, NCDef_Compound* ncdef_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_compound_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&ncdef_compound_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncdef_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Compound_write*/

ast_err
NCDef_Compound_read(ast_runtime* rt, NCDef_Compound** ncdef_compound_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Compound* ncdef_compound_v;
    unsigned long pos;

    ncdef_compound_v = (NCDef_Compound*)ast_alloc(rt,sizeof(NCDef_Compound));
    if(ncdef_compound_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Compound|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_compound_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&ncdef_compound_v->size);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncdef_compound_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_compound_vp) *ncdef_compound_vp = ncdef_compound_v;
done:
    return ACATCH(status);
} /*NCDef_Compound_read*/

ast_err
NCDef_Compound_reclaim(ast_runtime* rt, NCDef_Compound* ncdef_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_compound_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Compound_reclaim*/

size_t
NCDef_Compound_get_size(ast_runtime* rt, NCDef_Compound* ncdef_compound_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_compound_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&ncdef_compound_v->size);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_compound_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Compound_get_size*/

ast_err
NCDef_Compound_Return_write(ast_runtime* rt, NCDef_Compound_Return* ncdef_compound_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_compound_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_compound_return_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Compound_Return_write*/

ast_err
NCDef_Compound_Return_read(ast_runtime* rt, NCDef_Compound_Return** ncdef_compound_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Compound_Return* ncdef_compound_return_v;
    unsigned long pos;

    ncdef_compound_return_v = (NCDef_Compound_Return*)ast_alloc(rt,sizeof(NCDef_Compound_Return));
    if(ncdef_compound_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Compound_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_compound_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_compound_return_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_compound_return_vp) *ncdef_compound_return_vp = ncdef_compound_return_v;
done:
    return ACATCH(status);
} /*NCDef_Compound_Return_read*/

ast_err
NCDef_Compound_Return_reclaim(ast_runtime* rt, NCDef_Compound_Return* ncdef_compound_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_compound_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Compound_Return_reclaim*/

size_t
NCDef_Compound_Return_get_size(ast_runtime* rt, NCDef_Compound_Return* ncdef_compound_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_compound_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_compound_return_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Compound_Return_get_size*/

ast_err
NCInsert_compound_write(ast_runtime* rt, NCInsert_compound* ncinsert_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_compound_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinsert_compound_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinsert_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&ncinsert_compound_v->offset);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncinsert_compound_v->fieldtypeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInsert_compound_write*/

ast_err
NCInsert_compound_read(ast_runtime* rt, NCInsert_compound** ncinsert_compound_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_compound* ncinsert_compound_v;
    unsigned long pos;

    ncinsert_compound_v = (NCInsert_compound*)ast_alloc(rt,sizeof(NCInsert_compound));
    if(ncinsert_compound_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_compound|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_compound_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinsert_compound_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinsert_compound_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&ncinsert_compound_v->offset);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncinsert_compound_v->fieldtypeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_compound_vp) *ncinsert_compound_vp = ncinsert_compound_v;
done:
    return ACATCH(status);
} /*NCInsert_compound_read*/

ast_err
NCInsert_compound_reclaim(ast_runtime* rt, NCInsert_compound* ncinsert_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinsert_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinsert_compound_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_compound_reclaim*/

size_t
NCInsert_compound_get_size(ast_runtime* rt, NCInsert_compound* ncinsert_compound_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_compound_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_compound_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinsert_compound_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinsert_compound_v->offset);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_compound_v->fieldtypeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_compound_get_size*/

ast_err
NCInsert_compound_Return_write(ast_runtime* rt, NCInsert_compound_Return* ncinsert_compound_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_compound_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInsert_compound_Return_write*/

ast_err
NCInsert_compound_Return_read(ast_runtime* rt, NCInsert_compound_Return** ncinsert_compound_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_compound_Return* ncinsert_compound_return_v;
    unsigned long pos;

    ncinsert_compound_return_v = (NCInsert_compound_Return*)ast_alloc(rt,sizeof(NCInsert_compound_Return));
    if(ncinsert_compound_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_compound_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_compound_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_compound_return_vp) *ncinsert_compound_return_vp = ncinsert_compound_return_v;
done:
    return ACATCH(status);
} /*NCInsert_compound_Return_read*/

ast_err
NCInsert_compound_Return_reclaim(ast_runtime* rt, NCInsert_compound_Return* ncinsert_compound_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinsert_compound_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_compound_Return_reclaim*/

size_t
NCInsert_compound_Return_get_size(ast_runtime* rt, NCInsert_compound_Return* ncinsert_compound_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_compound_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_compound_Return_get_size*/

ast_err
NCInsert_array_compound_write(ast_runtime* rt, NCInsert_array_compound* ncinsert_array_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_array_compound_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinsert_array_compound_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinsert_array_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&ncinsert_array_compound_v->offset);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncinsert_array_compound_v->fieldtypeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,6,&ncinsert_array_compound_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinsert_array_compound_v->dimsizes.count;i++) {
            status = ast_write_primitive(rt,ast_int32,7,&ncinsert_array_compound_v->dimsizes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInsert_array_compound_write*/

ast_err
NCInsert_array_compound_read(ast_runtime* rt, NCInsert_array_compound** ncinsert_array_compound_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_array_compound* ncinsert_array_compound_v;
    unsigned long pos;

    ncinsert_array_compound_v = (NCInsert_array_compound*)ast_alloc(rt,sizeof(NCInsert_array_compound));
    if(ncinsert_array_compound_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_array_compound|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_array_compound_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinsert_array_compound_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinsert_array_compound_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&ncinsert_array_compound_v->offset);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncinsert_array_compound_v->fieldtypeid);
            } break;
        case 6: {
            status = ast_read_primitive(rt,ast_int32,6,&ncinsert_array_compound_v->ndims);
            } break;
        case 7: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,7,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinsert_array_compound_v->dimsizes,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_array_compound_vp) *ncinsert_array_compound_vp = ncinsert_array_compound_v;
done:
    return ACATCH(status);
} /*NCInsert_array_compound_read*/

ast_err
NCInsert_array_compound_reclaim(ast_runtime* rt, NCInsert_array_compound* ncinsert_array_compound_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinsert_array_compound_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinsert_array_compound_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_array_compound_reclaim*/

size_t
NCInsert_array_compound_get_size(ast_runtime* rt, NCInsert_array_compound* ncinsert_array_compound_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinsert_array_compound_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinsert_array_compound_v->offset);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_v->fieldtypeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,6);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_v->ndims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinsert_array_compound_v->dimsizes.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_v->dimsizes.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_array_compound_get_size*/

ast_err
NCInsert_array_compound_Return_write(ast_runtime* rt, NCInsert_array_compound_Return* ncinsert_array_compound_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_array_compound_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInsert_array_compound_Return_write*/

ast_err
NCInsert_array_compound_Return_read(ast_runtime* rt, NCInsert_array_compound_Return** ncinsert_array_compound_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_array_compound_Return* ncinsert_array_compound_return_v;
    unsigned long pos;

    ncinsert_array_compound_return_v = (NCInsert_array_compound_Return*)ast_alloc(rt,sizeof(NCInsert_array_compound_Return));
    if(ncinsert_array_compound_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_array_compound_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_array_compound_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_array_compound_return_vp) *ncinsert_array_compound_return_vp = ncinsert_array_compound_return_v;
done:
    return ACATCH(status);
} /*NCInsert_array_compound_Return_read*/

ast_err
NCInsert_array_compound_Return_reclaim(ast_runtime* rt, NCInsert_array_compound_Return* ncinsert_array_compound_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinsert_array_compound_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_array_compound_Return_reclaim*/

size_t
NCInsert_array_compound_Return_get_size(ast_runtime* rt, NCInsert_array_compound_Return* ncinsert_array_compound_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_array_compound_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_array_compound_Return_get_size*/

ast_err
NCInq_compound_field_write(ast_runtime* rt, NCInq_compound_field* ncinq_compound_field_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_compound_field_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_compound_field_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_compound_field_v->fieldid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_compound_field_write*/

ast_err
NCInq_compound_field_read(ast_runtime* rt, NCInq_compound_field** ncinq_compound_field_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_compound_field* ncinq_compound_field_v;
    unsigned long pos;

    ncinq_compound_field_v = (NCInq_compound_field*)ast_alloc(rt,sizeof(NCInq_compound_field));
    if(ncinq_compound_field_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_compound_field|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_compound_field_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_compound_field_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_compound_field_v->fieldid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_compound_field_vp) *ncinq_compound_field_vp = ncinq_compound_field_v;
done:
    return ACATCH(status);
} /*NCInq_compound_field_read*/

ast_err
NCInq_compound_field_reclaim(ast_runtime* rt, NCInq_compound_field* ncinq_compound_field_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_compound_field_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_compound_field_reclaim*/

size_t
NCInq_compound_field_get_size(ast_runtime* rt, NCInq_compound_field* ncinq_compound_field_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_v->fieldid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_compound_field_get_size*/

ast_err
NCInq_compound_field_Return_write(ast_runtime* rt, NCInq_compound_field_Return* ncinq_compound_field_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_compound_field_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_compound_field_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_compound_field_return_v->offset);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,4,&ncinq_compound_field_return_v->fieldtypeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncinq_compound_field_return_v->ndims);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncinq_compound_field_return_v->dimsizes.count;i++) {
            status = ast_write_primitive(rt,ast_int32,6,&ncinq_compound_field_return_v->dimsizes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCInq_compound_field_Return_write*/

ast_err
NCInq_compound_field_Return_read(ast_runtime* rt, NCInq_compound_field_Return** ncinq_compound_field_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_compound_field_Return* ncinq_compound_field_return_v;
    unsigned long pos;

    ncinq_compound_field_return_v = (NCInq_compound_field_Return*)ast_alloc(rt,sizeof(NCInq_compound_field_Return));
    if(ncinq_compound_field_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_compound_field_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_compound_field_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_compound_field_return_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_compound_field_return_v->offset);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_int32,4,&ncinq_compound_field_return_v->fieldtypeid);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncinq_compound_field_return_v->ndims);
            } break;
        case 6: {
            int32_t tmp;
            status = ast_read_primitive(rt,ast_int32,6,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_int32,&ncinq_compound_field_return_v->dimsizes,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_compound_field_return_vp) *ncinq_compound_field_return_vp = ncinq_compound_field_return_v;
done:
    return ACATCH(status);
} /*NCInq_compound_field_Return_read*/

ast_err
NCInq_compound_field_Return_reclaim(ast_runtime* rt, NCInq_compound_field_Return* ncinq_compound_field_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_compound_field_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_compound_field_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_compound_field_Return_reclaim*/

size_t
NCInq_compound_field_Return_get_size(ast_runtime* rt, NCInq_compound_field_Return* ncinq_compound_field_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_compound_field_return_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_compound_field_return_v->offset);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_return_v->fieldtypeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_return_v->ndims);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncinq_compound_field_return_v->dimsizes.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_field_return_v->dimsizes.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_compound_field_Return_get_size*/

ast_err
NCInq_compound_fieldindex_write(ast_runtime* rt, NCInq_compound_fieldindex* ncinq_compound_fieldindex_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_compound_fieldindex_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_compound_fieldindex_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinq_compound_fieldindex_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_compound_fieldindex_write*/

ast_err
NCInq_compound_fieldindex_read(ast_runtime* rt, NCInq_compound_fieldindex** ncinq_compound_fieldindex_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_compound_fieldindex* ncinq_compound_fieldindex_v;
    unsigned long pos;

    ncinq_compound_fieldindex_v = (NCInq_compound_fieldindex*)ast_alloc(rt,sizeof(NCInq_compound_fieldindex));
    if(ncinq_compound_fieldindex_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_compound_fieldindex|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_compound_fieldindex_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_compound_fieldindex_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinq_compound_fieldindex_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_compound_fieldindex_vp) *ncinq_compound_fieldindex_vp = ncinq_compound_fieldindex_v;
done:
    return ACATCH(status);
} /*NCInq_compound_fieldindex_read*/

ast_err
NCInq_compound_fieldindex_reclaim(ast_runtime* rt, NCInq_compound_fieldindex* ncinq_compound_fieldindex_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_compound_fieldindex_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_compound_fieldindex_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_compound_fieldindex_reclaim*/

size_t
NCInq_compound_fieldindex_get_size(ast_runtime* rt, NCInq_compound_fieldindex* ncinq_compound_fieldindex_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_fieldindex_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_fieldindex_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_compound_fieldindex_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_compound_fieldindex_get_size*/

ast_err
NCInq_compound_fieldindex_Return_write(ast_runtime* rt, NCInq_compound_fieldindex_Return* ncinq_compound_fieldindex_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_compound_fieldindex_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_compound_fieldindex_return_v->fieldid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_compound_fieldindex_Return_write*/

ast_err
NCInq_compound_fieldindex_Return_read(ast_runtime* rt, NCInq_compound_fieldindex_Return** ncinq_compound_fieldindex_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_compound_fieldindex_Return* ncinq_compound_fieldindex_return_v;
    unsigned long pos;

    ncinq_compound_fieldindex_return_v = (NCInq_compound_fieldindex_Return*)ast_alloc(rt,sizeof(NCInq_compound_fieldindex_Return));
    if(ncinq_compound_fieldindex_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_compound_fieldindex_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_compound_fieldindex_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_compound_fieldindex_return_v->fieldid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_compound_fieldindex_return_vp) *ncinq_compound_fieldindex_return_vp = ncinq_compound_fieldindex_return_v;
done:
    return ACATCH(status);
} /*NCInq_compound_fieldindex_Return_read*/

ast_err
NCInq_compound_fieldindex_Return_reclaim(ast_runtime* rt, NCInq_compound_fieldindex_Return* ncinq_compound_fieldindex_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_compound_fieldindex_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_compound_fieldindex_Return_reclaim*/

size_t
NCInq_compound_fieldindex_Return_get_size(ast_runtime* rt, NCInq_compound_fieldindex_Return* ncinq_compound_fieldindex_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_fieldindex_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_compound_fieldindex_return_v->fieldid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_compound_fieldindex_Return_get_size*/

ast_err
NCDef_Vlen_write(ast_runtime* rt, NCDef_Vlen* ncdef_vlen_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_vlen_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncdef_vlen_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncdef_vlen_v->base_typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Vlen_write*/

ast_err
NCDef_Vlen_read(ast_runtime* rt, NCDef_Vlen** ncdef_vlen_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Vlen* ncdef_vlen_v;
    unsigned long pos;

    ncdef_vlen_v = (NCDef_Vlen*)ast_alloc(rt,sizeof(NCDef_Vlen));
    if(ncdef_vlen_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Vlen|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_vlen_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncdef_vlen_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncdef_vlen_v->base_typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_vlen_vp) *ncdef_vlen_vp = ncdef_vlen_v;
done:
    return ACATCH(status);
} /*NCDef_Vlen_read*/

ast_err
NCDef_Vlen_reclaim(ast_runtime* rt, NCDef_Vlen* ncdef_vlen_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_vlen_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_vlen_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Vlen_reclaim*/

size_t
NCDef_Vlen_get_size(ast_runtime* rt, NCDef_Vlen* ncdef_vlen_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_vlen_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_vlen_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_vlen_v->base_typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Vlen_get_size*/

ast_err
NCDef_Vlen_Return_write(ast_runtime* rt, NCDef_Vlen_Return* ncdef_vlen_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_vlen_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_vlen_return_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Vlen_Return_write*/

ast_err
NCDef_Vlen_Return_read(ast_runtime* rt, NCDef_Vlen_Return** ncdef_vlen_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Vlen_Return* ncdef_vlen_return_v;
    unsigned long pos;

    ncdef_vlen_return_v = (NCDef_Vlen_Return*)ast_alloc(rt,sizeof(NCDef_Vlen_Return));
    if(ncdef_vlen_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Vlen_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_vlen_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_vlen_return_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_vlen_return_vp) *ncdef_vlen_return_vp = ncdef_vlen_return_v;
done:
    return ACATCH(status);
} /*NCDef_Vlen_Return_read*/

ast_err
NCDef_Vlen_Return_reclaim(ast_runtime* rt, NCDef_Vlen_Return* ncdef_vlen_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_vlen_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Vlen_Return_reclaim*/

size_t
NCDef_Vlen_Return_get_size(ast_runtime* rt, NCDef_Vlen_Return* ncdef_vlen_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_vlen_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_vlen_return_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Vlen_Return_get_size*/

ast_err
NCPut_vlen_element_write(ast_runtime* rt, NCPut_vlen_element* ncput_vlen_element_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vlen_element_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncput_vlen_element_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,3,&ncput_vlen_element_v->element);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&ncput_vlen_element_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,5,&ncput_vlen_element_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vlen_element_write*/

ast_err
NCPut_vlen_element_read(ast_runtime* rt, NCPut_vlen_element** ncput_vlen_element_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vlen_element* ncput_vlen_element_v;
    unsigned long pos;

    ncput_vlen_element_v = (NCPut_vlen_element*)ast_alloc(rt,sizeof(NCPut_vlen_element));
    if(ncput_vlen_element_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vlen_element|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vlen_element_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncput_vlen_element_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bytes,3,&ncput_vlen_element_v->element);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&ncput_vlen_element_v->len);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_bytes,5,&ncput_vlen_element_v->data);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vlen_element_vp) *ncput_vlen_element_vp = ncput_vlen_element_v;
done:
    return ACATCH(status);
} /*NCPut_vlen_element_read*/

ast_err
NCPut_vlen_element_reclaim(ast_runtime* rt, NCPut_vlen_element* ncput_vlen_element_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncput_vlen_element_v->element);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncput_vlen_element_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncput_vlen_element_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vlen_element_reclaim*/

size_t
NCPut_vlen_element_get_size(ast_runtime* rt, NCPut_vlen_element* ncput_vlen_element_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vlen_element_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vlen_element_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_vlen_element_v->element);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&ncput_vlen_element_v->len);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_bytes,&ncput_vlen_element_v->data);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vlen_element_get_size*/

ast_err
NCPut_vlen_element_Return_write(ast_runtime* rt, NCPut_vlen_element_Return* ncput_vlen_element_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncput_vlen_element_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCPut_vlen_element_Return_write*/

ast_err
NCPut_vlen_element_Return_read(ast_runtime* rt, NCPut_vlen_element_Return** ncput_vlen_element_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCPut_vlen_element_Return* ncput_vlen_element_return_v;
    unsigned long pos;

    ncput_vlen_element_return_v = (NCPut_vlen_element_Return*)ast_alloc(rt,sizeof(NCPut_vlen_element_Return));
    if(ncput_vlen_element_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCPut_vlen_element_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncput_vlen_element_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncput_vlen_element_return_vp) *ncput_vlen_element_return_vp = ncput_vlen_element_return_v;
done:
    return ACATCH(status);
} /*NCPut_vlen_element_Return_read*/

ast_err
NCPut_vlen_element_Return_reclaim(ast_runtime* rt, NCPut_vlen_element_Return* ncput_vlen_element_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncput_vlen_element_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCPut_vlen_element_Return_reclaim*/

size_t
NCPut_vlen_element_Return_get_size(ast_runtime* rt, NCPut_vlen_element_Return* ncput_vlen_element_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncput_vlen_element_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCPut_vlen_element_Return_get_size*/

ast_err
NCGet_vlen_element_write(ast_runtime* rt, NCGet_vlen_element* ncget_vlen_element_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vlen_element_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_vlen_element_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vlen_element_write*/

ast_err
NCGet_vlen_element_read(ast_runtime* rt, NCGet_vlen_element** ncget_vlen_element_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vlen_element* ncget_vlen_element_v;
    unsigned long pos;

    ncget_vlen_element_v = (NCGet_vlen_element*)ast_alloc(rt,sizeof(NCGet_vlen_element));
    if(ncget_vlen_element_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vlen_element|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vlen_element_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_vlen_element_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vlen_element_vp) *ncget_vlen_element_vp = ncget_vlen_element_v;
done:
    return ACATCH(status);
} /*NCGet_vlen_element_read*/

ast_err
NCGet_vlen_element_reclaim(ast_runtime* rt, NCGet_vlen_element* ncget_vlen_element_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_vlen_element_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vlen_element_reclaim*/

size_t
NCGet_vlen_element_get_size(ast_runtime* rt, NCGet_vlen_element* ncget_vlen_element_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vlen_element_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vlen_element_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vlen_element_get_size*/

ast_err
NCGet_vlen_element_Return_write(ast_runtime* rt, NCGet_vlen_element_Return* ncget_vlen_element_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_vlen_element_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&ncget_vlen_element_return_v->element);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncget_vlen_element_return_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,4,&ncget_vlen_element_return_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_vlen_element_Return_write*/

ast_err
NCGet_vlen_element_Return_read(ast_runtime* rt, NCGet_vlen_element_Return** ncget_vlen_element_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_vlen_element_Return* ncget_vlen_element_return_v;
    unsigned long pos;

    ncget_vlen_element_return_v = (NCGet_vlen_element_Return*)ast_alloc(rt,sizeof(NCGet_vlen_element_Return));
    if(ncget_vlen_element_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_vlen_element_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_vlen_element_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&ncget_vlen_element_return_v->element);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncget_vlen_element_return_v->len);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_bytes,4,&ncget_vlen_element_return_v->data);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_vlen_element_return_vp) *ncget_vlen_element_return_vp = ncget_vlen_element_return_v;
done:
    return ACATCH(status);
} /*NCGet_vlen_element_Return_read*/

ast_err
NCGet_vlen_element_Return_reclaim(ast_runtime* rt, NCGet_vlen_element_Return* ncget_vlen_element_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncget_vlen_element_return_v->element);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncget_vlen_element_return_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncget_vlen_element_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_vlen_element_Return_reclaim*/

size_t
NCGet_vlen_element_Return_get_size(ast_runtime* rt, NCGet_vlen_element_Return* ncget_vlen_element_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_vlen_element_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_vlen_element_return_v->element);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncget_vlen_element_return_v->len);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_bytes,&ncget_vlen_element_return_v->data);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_vlen_element_Return_get_size*/

ast_err
NCDef_Enum_write(ast_runtime* rt, NCDef_Enum* ncdef_enum_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_enum_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_enum_v->basetypeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncdef_enum_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Enum_write*/

ast_err
NCDef_Enum_read(ast_runtime* rt, NCDef_Enum** ncdef_enum_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Enum* ncdef_enum_v;
    unsigned long pos;

    ncdef_enum_v = (NCDef_Enum*)ast_alloc(rt,sizeof(NCDef_Enum));
    if(ncdef_enum_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Enum|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_enum_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_enum_v->basetypeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncdef_enum_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_enum_vp) *ncdef_enum_vp = ncdef_enum_v;
done:
    return ACATCH(status);
} /*NCDef_Enum_read*/

ast_err
NCDef_Enum_reclaim(ast_runtime* rt, NCDef_Enum* ncdef_enum_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_enum_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_enum_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Enum_reclaim*/

size_t
NCDef_Enum_get_size(ast_runtime* rt, NCDef_Enum* ncdef_enum_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_enum_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_enum_v->basetypeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_enum_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Enum_get_size*/

ast_err
NCDef_Enum_Return_write(ast_runtime* rt, NCDef_Enum_Return* ncdef_enum_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_enum_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_enum_return_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Enum_Return_write*/

ast_err
NCDef_Enum_Return_read(ast_runtime* rt, NCDef_Enum_Return** ncdef_enum_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Enum_Return* ncdef_enum_return_v;
    unsigned long pos;

    ncdef_enum_return_v = (NCDef_Enum_Return*)ast_alloc(rt,sizeof(NCDef_Enum_Return));
    if(ncdef_enum_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Enum_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_enum_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_enum_return_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_enum_return_vp) *ncdef_enum_return_vp = ncdef_enum_return_v;
done:
    return ACATCH(status);
} /*NCDef_Enum_Return_read*/

ast_err
NCDef_Enum_Return_reclaim(ast_runtime* rt, NCDef_Enum_Return* ncdef_enum_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_enum_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Enum_Return_reclaim*/

size_t
NCDef_Enum_Return_get_size(ast_runtime* rt, NCDef_Enum_Return* ncdef_enum_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_enum_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_enum_return_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Enum_Return_get_size*/

ast_err
NCInsert_enum_write(ast_runtime* rt, NCInsert_enum* ncinsert_enum_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_enum_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinsert_enum_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncinsert_enum_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,4,&ncinsert_enum_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInsert_enum_write*/

ast_err
NCInsert_enum_read(ast_runtime* rt, NCInsert_enum** ncinsert_enum_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_enum* ncinsert_enum_v;
    unsigned long pos;

    ncinsert_enum_v = (NCInsert_enum*)ast_alloc(rt,sizeof(NCInsert_enum));
    if(ncinsert_enum_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_enum|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_enum_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinsert_enum_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncinsert_enum_v->name);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_bytes,4,&ncinsert_enum_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_enum_vp) *ncinsert_enum_vp = ncinsert_enum_v;
done:
    return ACATCH(status);
} /*NCInsert_enum_read*/

ast_err
NCInsert_enum_reclaim(ast_runtime* rt, NCInsert_enum* ncinsert_enum_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinsert_enum_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncinsert_enum_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinsert_enum_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_enum_reclaim*/

size_t
NCInsert_enum_get_size(ast_runtime* rt, NCInsert_enum* ncinsert_enum_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_enum_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_enum_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncinsert_enum_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_bytes,&ncinsert_enum_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_enum_get_size*/

ast_err
NCInsert_enum_Return_write(ast_runtime* rt, NCInsert_enum_Return* ncinsert_enum_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinsert_enum_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInsert_enum_Return_write*/

ast_err
NCInsert_enum_Return_read(ast_runtime* rt, NCInsert_enum_Return** ncinsert_enum_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInsert_enum_Return* ncinsert_enum_return_v;
    unsigned long pos;

    ncinsert_enum_return_v = (NCInsert_enum_Return*)ast_alloc(rt,sizeof(NCInsert_enum_Return));
    if(ncinsert_enum_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInsert_enum_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinsert_enum_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinsert_enum_return_vp) *ncinsert_enum_return_vp = ncinsert_enum_return_v;
done:
    return ACATCH(status);
} /*NCInsert_enum_Return_read*/

ast_err
NCInsert_enum_Return_reclaim(ast_runtime* rt, NCInsert_enum_Return* ncinsert_enum_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinsert_enum_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInsert_enum_Return_reclaim*/

size_t
NCInsert_enum_Return_get_size(ast_runtime* rt, NCInsert_enum_Return* ncinsert_enum_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinsert_enum_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInsert_enum_Return_get_size*/

ast_err
NCInq_enum_member_write(ast_runtime* rt, NCInq_enum_member* ncinq_enum_member_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_enum_member_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_enum_member_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,3,&ncinq_enum_member_v->index);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_enum_member_write*/

ast_err
NCInq_enum_member_read(ast_runtime* rt, NCInq_enum_member** ncinq_enum_member_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_enum_member* ncinq_enum_member_v;
    unsigned long pos;

    ncinq_enum_member_v = (NCInq_enum_member*)ast_alloc(rt,sizeof(NCInq_enum_member));
    if(ncinq_enum_member_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_enum_member|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_enum_member_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_enum_member_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_int32,3,&ncinq_enum_member_v->index);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_enum_member_vp) *ncinq_enum_member_vp = ncinq_enum_member_v;
done:
    return ACATCH(status);
} /*NCInq_enum_member_read*/

ast_err
NCInq_enum_member_reclaim(ast_runtime* rt, NCInq_enum_member* ncinq_enum_member_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_enum_member_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_enum_member_reclaim*/

size_t
NCInq_enum_member_get_size(ast_runtime* rt, NCInq_enum_member* ncinq_enum_member_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_member_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_member_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_member_v->index);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_enum_member_get_size*/

ast_err
NCInq_enum_member_Return_write(ast_runtime* rt, NCInq_enum_member_Return* ncinq_enum_member_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_enum_member_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_enum_member_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,3,&ncinq_enum_member_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_enum_member_Return_write*/

ast_err
NCInq_enum_member_Return_read(ast_runtime* rt, NCInq_enum_member_Return** ncinq_enum_member_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_enum_member_Return* ncinq_enum_member_return_v;
    unsigned long pos;

    ncinq_enum_member_return_v = (NCInq_enum_member_Return*)ast_alloc(rt,sizeof(NCInq_enum_member_Return));
    if(ncinq_enum_member_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_enum_member_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_enum_member_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_enum_member_return_v->name);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bytes,3,&ncinq_enum_member_return_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_enum_member_return_vp) *ncinq_enum_member_return_vp = ncinq_enum_member_return_v;
done:
    return ACATCH(status);
} /*NCInq_enum_member_Return_read*/

ast_err
NCInq_enum_member_Return_reclaim(ast_runtime* rt, NCInq_enum_member_Return* ncinq_enum_member_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_enum_member_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&ncinq_enum_member_return_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_enum_member_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_enum_member_Return_reclaim*/

size_t
NCInq_enum_member_Return_get_size(ast_runtime* rt, NCInq_enum_member_Return* ncinq_enum_member_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_member_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_enum_member_return_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bytes,&ncinq_enum_member_return_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_enum_member_Return_get_size*/

ast_err
NCInq_enum_ident_write(ast_runtime* rt, NCInq_enum_ident* ncinq_enum_ident_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_enum_ident_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncinq_enum_ident_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncinq_enum_ident_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_enum_ident_write*/

ast_err
NCInq_enum_ident_read(ast_runtime* rt, NCInq_enum_ident** ncinq_enum_ident_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_enum_ident* ncinq_enum_ident_v;
    unsigned long pos;

    ncinq_enum_ident_v = (NCInq_enum_ident*)ast_alloc(rt,sizeof(NCInq_enum_ident));
    if(ncinq_enum_ident_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_enum_ident|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_enum_ident_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncinq_enum_ident_v->typeid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncinq_enum_ident_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_enum_ident_vp) *ncinq_enum_ident_vp = ncinq_enum_ident_v;
done:
    return ACATCH(status);
} /*NCInq_enum_ident_read*/

ast_err
NCInq_enum_ident_reclaim(ast_runtime* rt, NCInq_enum_ident* ncinq_enum_ident_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncinq_enum_ident_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_enum_ident_reclaim*/

size_t
NCInq_enum_ident_get_size(ast_runtime* rt, NCInq_enum_ident* ncinq_enum_ident_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_ident_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_ident_v->typeid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncinq_enum_ident_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_enum_ident_get_size*/

ast_err
NCInq_enum_ident_Return_write(ast_runtime* rt, NCInq_enum_ident_Return* ncinq_enum_ident_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncinq_enum_ident_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&ncinq_enum_ident_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCInq_enum_ident_Return_write*/

ast_err
NCInq_enum_ident_Return_read(ast_runtime* rt, NCInq_enum_ident_Return** ncinq_enum_ident_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCInq_enum_ident_Return* ncinq_enum_ident_return_v;
    unsigned long pos;

    ncinq_enum_ident_return_v = (NCInq_enum_ident_Return*)ast_alloc(rt,sizeof(NCInq_enum_ident_Return));
    if(ncinq_enum_ident_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCInq_enum_ident_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncinq_enum_ident_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&ncinq_enum_ident_return_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncinq_enum_ident_return_vp) *ncinq_enum_ident_return_vp = ncinq_enum_ident_return_v;
done:
    return ACATCH(status);
} /*NCInq_enum_ident_Return_read*/

ast_err
NCInq_enum_ident_Return_reclaim(ast_runtime* rt, NCInq_enum_ident_Return* ncinq_enum_ident_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncinq_enum_ident_return_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncinq_enum_ident_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCInq_enum_ident_Return_reclaim*/

size_t
NCInq_enum_ident_Return_get_size(ast_runtime* rt, NCInq_enum_ident_Return* ncinq_enum_ident_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncinq_enum_ident_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&ncinq_enum_ident_return_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCInq_enum_ident_Return_get_size*/

ast_err
NCDef_Opaque_write(ast_runtime* rt, NCDef_Opaque* ncdef_opaque_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_opaque_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&ncdef_opaque_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,3,&ncdef_opaque_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Opaque_write*/

ast_err
NCDef_Opaque_read(ast_runtime* rt, NCDef_Opaque** ncdef_opaque_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Opaque* ncdef_opaque_v;
    unsigned long pos;

    ncdef_opaque_v = (NCDef_Opaque*)ast_alloc(rt,sizeof(NCDef_Opaque));
    if(ncdef_opaque_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Opaque|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_opaque_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&ncdef_opaque_v->size);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_string,3,&ncdef_opaque_v->name);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_opaque_vp) *ncdef_opaque_vp = ncdef_opaque_v;
done:
    return ACATCH(status);
} /*NCDef_Opaque_read*/

ast_err
NCDef_Opaque_reclaim(ast_runtime* rt, NCDef_Opaque* ncdef_opaque_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncdef_opaque_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_opaque_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Opaque_reclaim*/

size_t
NCDef_Opaque_get_size(ast_runtime* rt, NCDef_Opaque* ncdef_opaque_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_opaque_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&ncdef_opaque_v->size);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_string,&ncdef_opaque_v->name);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Opaque_get_size*/

ast_err
NCDef_Opaque_Return_write(ast_runtime* rt, NCDef_Opaque_Return* ncdef_opaque_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_opaque_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_opaque_return_v->typeid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Opaque_Return_write*/

ast_err
NCDef_Opaque_Return_read(ast_runtime* rt, NCDef_Opaque_Return** ncdef_opaque_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Opaque_Return* ncdef_opaque_return_v;
    unsigned long pos;

    ncdef_opaque_return_v = (NCDef_Opaque_Return*)ast_alloc(rt,sizeof(NCDef_Opaque_Return));
    if(ncdef_opaque_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Opaque_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_opaque_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_opaque_return_v->typeid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_opaque_return_vp) *ncdef_opaque_return_vp = ncdef_opaque_return_v;
done:
    return ACATCH(status);
} /*NCDef_Opaque_Return_read*/

ast_err
NCDef_Opaque_Return_reclaim(ast_runtime* rt, NCDef_Opaque_Return* ncdef_opaque_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_opaque_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Opaque_Return_reclaim*/

size_t
NCDef_Opaque_Return_get_size(ast_runtime* rt, NCDef_Opaque_Return* ncdef_opaque_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_opaque_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_opaque_return_v->typeid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Opaque_Return_get_size*/

ast_err
NCDef_var_deflate_write(ast_runtime* rt, NCDef_var_deflate* ncdef_var_deflate_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_deflate_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_deflate_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncdef_var_deflate_v->shuffle);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,4,&ncdef_var_deflate_v->deflate);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,5,&ncdef_var_deflate_v->deflatelevel);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_var_deflate_write*/

ast_err
NCDef_var_deflate_read(ast_runtime* rt, NCDef_var_deflate** ncdef_var_deflate_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_var_deflate* ncdef_var_deflate_v;
    unsigned long pos;

    ncdef_var_deflate_v = (NCDef_var_deflate*)ast_alloc(rt,sizeof(NCDef_var_deflate));
    if(ncdef_var_deflate_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_var_deflate|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_deflate_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_deflate_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncdef_var_deflate_v->shuffle);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_bool,4,&ncdef_var_deflate_v->deflate);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_int32,5,&ncdef_var_deflate_v->deflatelevel);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_deflate_vp) *ncdef_var_deflate_vp = ncdef_var_deflate_v;
done:
    return ACATCH(status);
} /*NCDef_var_deflate_read*/

ast_err
NCDef_var_deflate_reclaim(ast_runtime* rt, NCDef_var_deflate* ncdef_var_deflate_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_deflate_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_var_deflate_reclaim*/

size_t
NCDef_var_deflate_get_size(ast_runtime* rt, NCDef_var_deflate* ncdef_var_deflate_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_deflate_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_deflate_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_deflate_v->shuffle);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_deflate_v->deflate);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_deflate_v->deflatelevel);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_var_deflate_get_size*/

ast_err
NCDef_var_deflate_Return_write(ast_runtime* rt, NCDef_var_deflate_Return* ncdef_var_deflate_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_deflate_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_var_deflate_Return_write*/

ast_err
NCDef_var_deflate_Return_read(ast_runtime* rt, NCDef_var_deflate_Return** ncdef_var_deflate_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_var_deflate_Return* ncdef_var_deflate_return_v;
    unsigned long pos;

    ncdef_var_deflate_return_v = (NCDef_var_deflate_Return*)ast_alloc(rt,sizeof(NCDef_var_deflate_Return));
    if(ncdef_var_deflate_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_var_deflate_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_deflate_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_deflate_return_vp) *ncdef_var_deflate_return_vp = ncdef_var_deflate_return_v;
done:
    return ACATCH(status);
} /*NCDef_var_deflate_Return_read*/

ast_err
NCDef_var_deflate_Return_reclaim(ast_runtime* rt, NCDef_var_deflate_Return* ncdef_var_deflate_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_deflate_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_var_deflate_Return_reclaim*/

size_t
NCDef_var_deflate_Return_get_size(ast_runtime* rt, NCDef_var_deflate_Return* ncdef_var_deflate_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_deflate_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_var_deflate_Return_get_size*/

ast_err
NCDef_Var_Fletcher32_write(ast_runtime* rt, NCDef_Var_Fletcher32* ncdef_var_fletcher32_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_fletcher32_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_fletcher32_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncdef_var_fletcher32_v->fletcher32);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Fletcher32_write*/

ast_err
NCDef_Var_Fletcher32_read(ast_runtime* rt, NCDef_Var_Fletcher32** ncdef_var_fletcher32_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Fletcher32* ncdef_var_fletcher32_v;
    unsigned long pos;

    ncdef_var_fletcher32_v = (NCDef_Var_Fletcher32*)ast_alloc(rt,sizeof(NCDef_Var_Fletcher32));
    if(ncdef_var_fletcher32_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Fletcher32|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_fletcher32_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_fletcher32_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncdef_var_fletcher32_v->fletcher32);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_fletcher32_vp) *ncdef_var_fletcher32_vp = ncdef_var_fletcher32_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Fletcher32_read*/

ast_err
NCDef_Var_Fletcher32_reclaim(ast_runtime* rt, NCDef_Var_Fletcher32* ncdef_var_fletcher32_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_fletcher32_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Fletcher32_reclaim*/

size_t
NCDef_Var_Fletcher32_get_size(ast_runtime* rt, NCDef_Var_Fletcher32* ncdef_var_fletcher32_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fletcher32_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fletcher32_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_fletcher32_v->fletcher32);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Fletcher32_get_size*/

ast_err
NCDef_Var_Fletcher32_Return_write(ast_runtime* rt, NCDef_Var_Fletcher32_Return* ncdef_var_fletcher32_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_fletcher32_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Fletcher32_Return_write*/

ast_err
NCDef_Var_Fletcher32_Return_read(ast_runtime* rt, NCDef_Var_Fletcher32_Return** ncdef_var_fletcher32_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Fletcher32_Return* ncdef_var_fletcher32_return_v;
    unsigned long pos;

    ncdef_var_fletcher32_return_v = (NCDef_Var_Fletcher32_Return*)ast_alloc(rt,sizeof(NCDef_Var_Fletcher32_Return));
    if(ncdef_var_fletcher32_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Fletcher32_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_fletcher32_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_fletcher32_return_vp) *ncdef_var_fletcher32_return_vp = ncdef_var_fletcher32_return_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Fletcher32_Return_read*/

ast_err
NCDef_Var_Fletcher32_Return_reclaim(ast_runtime* rt, NCDef_Var_Fletcher32_Return* ncdef_var_fletcher32_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_fletcher32_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Fletcher32_Return_reclaim*/

size_t
NCDef_Var_Fletcher32_Return_get_size(ast_runtime* rt, NCDef_Var_Fletcher32_Return* ncdef_var_fletcher32_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fletcher32_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Fletcher32_Return_get_size*/

ast_err
NCDef_Var_Chunking_write(ast_runtime* rt, NCDef_Var_Chunking* ncdef_var_chunking_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_chunking_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_chunking_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncdef_var_chunking_v->contiguous);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<ncdef_var_chunking_v->chunksizes.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,4,&ncdef_var_chunking_v->chunksizes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Chunking_write*/

ast_err
NCDef_Var_Chunking_read(ast_runtime* rt, NCDef_Var_Chunking** ncdef_var_chunking_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Chunking* ncdef_var_chunking_v;
    unsigned long pos;

    ncdef_var_chunking_v = (NCDef_Var_Chunking*)ast_alloc(rt,sizeof(NCDef_Var_Chunking));
    if(ncdef_var_chunking_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Chunking|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_chunking_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_chunking_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncdef_var_chunking_v->contiguous);
            } break;
        case 4: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&ncdef_var_chunking_v->chunksizes,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_chunking_vp) *ncdef_var_chunking_vp = ncdef_var_chunking_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Chunking_read*/

ast_err
NCDef_Var_Chunking_reclaim(ast_runtime* rt, NCDef_Var_Chunking* ncdef_var_chunking_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_chunking_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Chunking_reclaim*/

size_t
NCDef_Var_Chunking_get_size(ast_runtime* rt, NCDef_Var_Chunking* ncdef_var_chunking_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_chunking_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_chunking_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_chunking_v->contiguous);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<ncdef_var_chunking_v->chunksizes.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_uint64,&ncdef_var_chunking_v->chunksizes.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Chunking_get_size*/

ast_err
NCDef_Var_Chunking_Return_write(ast_runtime* rt, NCDef_Var_Chunking_Return* ncdef_var_chunking_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_chunking_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Chunking_Return_write*/

ast_err
NCDef_Var_Chunking_Return_read(ast_runtime* rt, NCDef_Var_Chunking_Return** ncdef_var_chunking_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Chunking_Return* ncdef_var_chunking_return_v;
    unsigned long pos;

    ncdef_var_chunking_return_v = (NCDef_Var_Chunking_Return*)ast_alloc(rt,sizeof(NCDef_Var_Chunking_Return));
    if(ncdef_var_chunking_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Chunking_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_chunking_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_chunking_return_vp) *ncdef_var_chunking_return_vp = ncdef_var_chunking_return_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Chunking_Return_read*/

ast_err
NCDef_Var_Chunking_Return_reclaim(ast_runtime* rt, NCDef_Var_Chunking_Return* ncdef_var_chunking_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_chunking_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Chunking_Return_reclaim*/

size_t
NCDef_Var_Chunking_Return_get_size(ast_runtime* rt, NCDef_Var_Chunking_Return* ncdef_var_chunking_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_chunking_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Chunking_Return_get_size*/

ast_err
NCDef_Var_Fill_write(ast_runtime* rt, NCDef_Var_Fill* ncdef_var_fill_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_fill_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_fill_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncdef_var_fill_v->nofill);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,4,&ncdef_var_fill_v->fill_value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Fill_write*/

ast_err
NCDef_Var_Fill_read(ast_runtime* rt, NCDef_Var_Fill** ncdef_var_fill_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Fill* ncdef_var_fill_v;
    unsigned long pos;

    ncdef_var_fill_v = (NCDef_Var_Fill*)ast_alloc(rt,sizeof(NCDef_Var_Fill));
    if(ncdef_var_fill_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Fill|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_fill_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_fill_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncdef_var_fill_v->nofill);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_bytes,4,&ncdef_var_fill_v->fill_value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_fill_vp) *ncdef_var_fill_vp = ncdef_var_fill_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Fill_read*/

ast_err
NCDef_Var_Fill_reclaim(ast_runtime* rt, NCDef_Var_Fill* ncdef_var_fill_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&ncdef_var_fill_v->fill_value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncdef_var_fill_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Fill_reclaim*/

size_t
NCDef_Var_Fill_get_size(ast_runtime* rt, NCDef_Var_Fill* ncdef_var_fill_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fill_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fill_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_fill_v->nofill);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_bytes,&ncdef_var_fill_v->fill_value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Fill_get_size*/

ast_err
NCDef_Var_Fill_Return_write(ast_runtime* rt, NCDef_Var_Fill_Return* ncdef_var_fill_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_fill_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_Fill_Return_write*/

ast_err
NCDef_Var_Fill_Return_read(ast_runtime* rt, NCDef_Var_Fill_Return** ncdef_var_fill_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_Fill_Return* ncdef_var_fill_return_v;
    unsigned long pos;

    ncdef_var_fill_return_v = (NCDef_Var_Fill_Return*)ast_alloc(rt,sizeof(NCDef_Var_Fill_Return));
    if(ncdef_var_fill_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_Fill_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_fill_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_fill_return_vp) *ncdef_var_fill_return_vp = ncdef_var_fill_return_v;
done:
    return ACATCH(status);
} /*NCDef_Var_Fill_Return_read*/

ast_err
NCDef_Var_Fill_Return_reclaim(ast_runtime* rt, NCDef_Var_Fill_Return* ncdef_var_fill_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_fill_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_Fill_Return_reclaim*/

size_t
NCDef_Var_Fill_Return_get_size(ast_runtime* rt, NCDef_Var_Fill_Return* ncdef_var_fill_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_fill_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_Fill_Return_get_size*/

ast_err
NCDef_Var_endian_write(ast_runtime* rt, NCDef_Var_endian* ncdef_var_endian_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_endian_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncdef_var_endian_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bool,3,&ncdef_var_endian_v->bigendian);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_endian_write*/

ast_err
NCDef_Var_endian_read(ast_runtime* rt, NCDef_Var_endian** ncdef_var_endian_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_endian* ncdef_var_endian_v;
    unsigned long pos;

    ncdef_var_endian_v = (NCDef_Var_endian*)ast_alloc(rt,sizeof(NCDef_Var_endian));
    if(ncdef_var_endian_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_endian|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_endian_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncdef_var_endian_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_bool,3,&ncdef_var_endian_v->bigendian);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_endian_vp) *ncdef_var_endian_vp = ncdef_var_endian_v;
done:
    return ACATCH(status);
} /*NCDef_Var_endian_read*/

ast_err
NCDef_Var_endian_reclaim(ast_runtime* rt, NCDef_Var_endian* ncdef_var_endian_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_endian_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_endian_reclaim*/

size_t
NCDef_Var_endian_get_size(ast_runtime* rt, NCDef_Var_endian* ncdef_var_endian_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_endian_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_endian_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_bool,&ncdef_var_endian_v->bigendian);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_endian_get_size*/

ast_err
NCDef_Var_endian_Return_write(ast_runtime* rt, NCDef_Var_endian_Return* ncdef_var_endian_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncdef_var_endian_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCDef_Var_endian_Return_write*/

ast_err
NCDef_Var_endian_Return_read(ast_runtime* rt, NCDef_Var_endian_Return** ncdef_var_endian_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCDef_Var_endian_Return* ncdef_var_endian_return_v;
    unsigned long pos;

    ncdef_var_endian_return_v = (NCDef_Var_endian_Return*)ast_alloc(rt,sizeof(NCDef_Var_endian_Return));
    if(ncdef_var_endian_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCDef_Var_endian_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncdef_var_endian_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncdef_var_endian_return_vp) *ncdef_var_endian_return_vp = ncdef_var_endian_return_v;
done:
    return ACATCH(status);
} /*NCDef_Var_endian_Return_read*/

ast_err
NCDef_Var_endian_Return_reclaim(ast_runtime* rt, NCDef_Var_endian_Return* ncdef_var_endian_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncdef_var_endian_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCDef_Var_endian_Return_reclaim*/

size_t
NCDef_Var_endian_Return_get_size(ast_runtime* rt, NCDef_Var_endian_Return* ncdef_var_endian_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncdef_var_endian_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCDef_Var_endian_Return_get_size*/

ast_err
NCSet_var_chunk_cache_write(ast_runtime* rt, NCSet_var_chunk_cache* ncset_var_chunk_cache_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_var_chunk_cache_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncset_var_chunk_cache_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncset_var_chunk_cache_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,4,&ncset_var_chunk_cache_v->nelems);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_float,5,&ncset_var_chunk_cache_v->preemption);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_var_chunk_cache_write*/

ast_err
NCSet_var_chunk_cache_read(ast_runtime* rt, NCSet_var_chunk_cache** ncset_var_chunk_cache_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_var_chunk_cache* ncset_var_chunk_cache_v;
    unsigned long pos;

    ncset_var_chunk_cache_v = (NCSet_var_chunk_cache*)ast_alloc(rt,sizeof(NCSet_var_chunk_cache));
    if(ncset_var_chunk_cache_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_var_chunk_cache|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_var_chunk_cache_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncset_var_chunk_cache_v->varid);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncset_var_chunk_cache_v->size);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_uint64,4,&ncset_var_chunk_cache_v->nelems);
            } break;
        case 5: {
            status = ast_read_primitive(rt,ast_float,5,&ncset_var_chunk_cache_v->preemption);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_var_chunk_cache_vp) *ncset_var_chunk_cache_vp = ncset_var_chunk_cache_v;
done:
    return ACATCH(status);
} /*NCSet_var_chunk_cache_read*/

ast_err
NCSet_var_chunk_cache_reclaim(ast_runtime* rt, NCSet_var_chunk_cache* ncset_var_chunk_cache_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_var_chunk_cache_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_var_chunk_cache_reclaim*/

size_t
NCSet_var_chunk_cache_get_size(ast_runtime* rt, NCSet_var_chunk_cache* ncset_var_chunk_cache_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_var_chunk_cache_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_var_chunk_cache_v->varid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncset_var_chunk_cache_v->size);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint64,&ncset_var_chunk_cache_v->nelems);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,5);
        fieldsize += ast_get_size(rt,ast_float,&ncset_var_chunk_cache_v->preemption);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_var_chunk_cache_get_size*/

ast_err
NCSet_var_chunk_cache_Return_write(ast_runtime* rt, NCSet_var_chunk_cache_Return* ncset_var_chunk_cache_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncset_var_chunk_cache_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCSet_var_chunk_cache_Return_write*/

ast_err
NCSet_var_chunk_cache_Return_read(ast_runtime* rt, NCSet_var_chunk_cache_Return** ncset_var_chunk_cache_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCSet_var_chunk_cache_Return* ncset_var_chunk_cache_return_v;
    unsigned long pos;

    ncset_var_chunk_cache_return_v = (NCSet_var_chunk_cache_Return*)ast_alloc(rt,sizeof(NCSet_var_chunk_cache_Return));
    if(ncset_var_chunk_cache_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCSet_var_chunk_cache_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncset_var_chunk_cache_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncset_var_chunk_cache_return_vp) *ncset_var_chunk_cache_return_vp = ncset_var_chunk_cache_return_v;
done:
    return ACATCH(status);
} /*NCSet_var_chunk_cache_Return_read*/

ast_err
NCSet_var_chunk_cache_Return_reclaim(ast_runtime* rt, NCSet_var_chunk_cache_Return* ncset_var_chunk_cache_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncset_var_chunk_cache_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCSet_var_chunk_cache_Return_reclaim*/

size_t
NCSet_var_chunk_cache_Return_get_size(ast_runtime* rt, NCSet_var_chunk_cache_Return* ncset_var_chunk_cache_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncset_var_chunk_cache_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCSet_var_chunk_cache_Return_get_size*/

ast_err
NCGet_var_chunk_cache_write(ast_runtime* rt, NCGet_var_chunk_cache* ncget_var_chunk_cache_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_var_chunk_cache_v->ncid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncget_var_chunk_cache_v->varid);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_var_chunk_cache_write*/

ast_err
NCGet_var_chunk_cache_read(ast_runtime* rt, NCGet_var_chunk_cache** ncget_var_chunk_cache_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_var_chunk_cache* ncget_var_chunk_cache_v;
    unsigned long pos;

    ncget_var_chunk_cache_v = (NCGet_var_chunk_cache*)ast_alloc(rt,sizeof(NCGet_var_chunk_cache));
    if(ncget_var_chunk_cache_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_var_chunk_cache|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_var_chunk_cache_v->ncid);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncget_var_chunk_cache_v->varid);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_var_chunk_cache_vp) *ncget_var_chunk_cache_vp = ncget_var_chunk_cache_v;
done:
    return ACATCH(status);
} /*NCGet_var_chunk_cache_read*/

ast_err
NCGet_var_chunk_cache_reclaim(ast_runtime* rt, NCGet_var_chunk_cache* ncget_var_chunk_cache_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_var_chunk_cache_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_var_chunk_cache_reclaim*/

size_t
NCGet_var_chunk_cache_get_size(ast_runtime* rt, NCGet_var_chunk_cache* ncget_var_chunk_cache_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_var_chunk_cache_v->ncid);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_var_chunk_cache_v->varid);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_var_chunk_cache_get_size*/

ast_err
NCGet_var_chunk_cache_Return_write(ast_runtime* rt, NCGet_var_chunk_cache_Return* ncget_var_chunk_cache_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncget_var_chunk_cache_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&ncget_var_chunk_cache_return_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&ncget_var_chunk_cache_return_v->nelems);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_float,4,&ncget_var_chunk_cache_return_v->preemption);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCGet_var_chunk_cache_Return_write*/

ast_err
NCGet_var_chunk_cache_Return_read(ast_runtime* rt, NCGet_var_chunk_cache_Return** ncget_var_chunk_cache_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCGet_var_chunk_cache_Return* ncget_var_chunk_cache_return_v;
    unsigned long pos;

    ncget_var_chunk_cache_return_v = (NCGet_var_chunk_cache_Return*)ast_alloc(rt,sizeof(NCGet_var_chunk_cache_Return));
    if(ncget_var_chunk_cache_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCGet_var_chunk_cache_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncget_var_chunk_cache_return_v->ncstatus);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&ncget_var_chunk_cache_return_v->size);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&ncget_var_chunk_cache_return_v->nelems);
            } break;
        case 4: {
            status = ast_read_primitive(rt,ast_float,4,&ncget_var_chunk_cache_return_v->preemption);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncget_var_chunk_cache_return_vp) *ncget_var_chunk_cache_return_vp = ncget_var_chunk_cache_return_v;
done:
    return ACATCH(status);
} /*NCGet_var_chunk_cache_Return_read*/

ast_err
NCGet_var_chunk_cache_Return_reclaim(ast_runtime* rt, NCGet_var_chunk_cache_Return* ncget_var_chunk_cache_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncget_var_chunk_cache_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCGet_var_chunk_cache_Return_reclaim*/

size_t
NCGet_var_chunk_cache_Return_get_size(ast_runtime* rt, NCGet_var_chunk_cache_Return* ncget_var_chunk_cache_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncget_var_chunk_cache_return_v->ncstatus);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&ncget_var_chunk_cache_return_v->size);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&ncget_var_chunk_cache_return_v->nelems);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_float,&ncget_var_chunk_cache_return_v->preemption);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCGet_var_chunk_cache_Return_get_size*/

ast_err
NCNC_set_log_level_write(ast_runtime* rt, NCNC_set_log_level* ncnc_set_log_level_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncnc_set_log_level_v->newlevel);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCNC_set_log_level_write*/

ast_err
NCNC_set_log_level_read(ast_runtime* rt, NCNC_set_log_level** ncnc_set_log_level_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_set_log_level* ncnc_set_log_level_v;
    unsigned long pos;

    ncnc_set_log_level_v = (NCNC_set_log_level*)ast_alloc(rt,sizeof(NCNC_set_log_level));
    if(ncnc_set_log_level_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_set_log_level|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncnc_set_log_level_v->newlevel);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_set_log_level_vp) *ncnc_set_log_level_vp = ncnc_set_log_level_v;
done:
    return ACATCH(status);
} /*NCNC_set_log_level_read*/

ast_err
NCNC_set_log_level_reclaim(ast_runtime* rt, NCNC_set_log_level* ncnc_set_log_level_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncnc_set_log_level_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_set_log_level_reclaim*/

size_t
NCNC_set_log_level_get_size(ast_runtime* rt, NCNC_set_log_level* ncnc_set_log_level_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncnc_set_log_level_v->newlevel);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCNC_set_log_level_get_size*/

ast_err
NCNC_set_log_level_Return_write(ast_runtime* rt, NCNC_set_log_level_Return* ncnc_set_log_level_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncnc_set_log_level_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCNC_set_log_level_Return_write*/

ast_err
NCNC_set_log_level_Return_read(ast_runtime* rt, NCNC_set_log_level_Return** ncnc_set_log_level_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_set_log_level_Return* ncnc_set_log_level_return_v;
    unsigned long pos;

    ncnc_set_log_level_return_v = (NCNC_set_log_level_Return*)ast_alloc(rt,sizeof(NCNC_set_log_level_Return));
    if(ncnc_set_log_level_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_set_log_level_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncnc_set_log_level_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_set_log_level_return_vp) *ncnc_set_log_level_return_vp = ncnc_set_log_level_return_v;
done:
    return ACATCH(status);
} /*NCNC_set_log_level_Return_read*/

ast_err
NCNC_set_log_level_Return_reclaim(ast_runtime* rt, NCNC_set_log_level_Return* ncnc_set_log_level_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncnc_set_log_level_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_set_log_level_Return_reclaim*/

size_t
NCNC_set_log_level_Return_get_size(ast_runtime* rt, NCNC_set_log_level_Return* ncnc_set_log_level_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncnc_set_log_level_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCNC_set_log_level_Return_get_size*/

ast_err
NCNC_inq_libvers_write(ast_runtime* rt, NCNC_inq_libvers* ncnc_inq_libvers_v)
{
    ast_err status = AST_NOERR;


    return ACATCH(status);

} /*NCNC_inq_libvers_write*/

ast_err
NCNC_inq_libvers_read(ast_runtime* rt, NCNC_inq_libvers** ncnc_inq_libvers_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_inq_libvers* ncnc_inq_libvers_v;
    unsigned long pos;

    ncnc_inq_libvers_v = (NCNC_inq_libvers*)ast_alloc(rt,sizeof(NCNC_inq_libvers));
    if(ncnc_inq_libvers_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_inq_libvers|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_inq_libvers_vp) *ncnc_inq_libvers_vp = ncnc_inq_libvers_v;
done:
    return ACATCH(status);
} /*NCNC_inq_libvers_read*/

ast_err
NCNC_inq_libvers_reclaim(ast_runtime* rt, NCNC_inq_libvers* ncnc_inq_libvers_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncnc_inq_libvers_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_inq_libvers_reclaim*/

size_t
NCNC_inq_libvers_get_size(ast_runtime* rt, NCNC_inq_libvers* ncnc_inq_libvers_v)
{
    size_t totalsize = 0;

    return totalsize;

} /*NCNC_inq_libvers_get_size*/

ast_err
NCNC_inq_libvers_Return_write(ast_runtime* rt, NCNC_inq_libvers_Return* ncnc_inq_libvers_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&ncnc_inq_libvers_return_v->version);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCNC_inq_libvers_Return_write*/

ast_err
NCNC_inq_libvers_Return_read(ast_runtime* rt, NCNC_inq_libvers_Return** ncnc_inq_libvers_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_inq_libvers_Return* ncnc_inq_libvers_return_v;
    unsigned long pos;

    ncnc_inq_libvers_return_v = (NCNC_inq_libvers_Return*)ast_alloc(rt,sizeof(NCNC_inq_libvers_Return));
    if(ncnc_inq_libvers_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_inq_libvers_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&ncnc_inq_libvers_return_v->version);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_inq_libvers_return_vp) *ncnc_inq_libvers_return_vp = ncnc_inq_libvers_return_v;
done:
    return ACATCH(status);
} /*NCNC_inq_libvers_Return_read*/

ast_err
NCNC_inq_libvers_Return_reclaim(ast_runtime* rt, NCNC_inq_libvers_Return* ncnc_inq_libvers_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncnc_inq_libvers_return_v->version);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncnc_inq_libvers_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_inq_libvers_Return_reclaim*/

size_t
NCNC_inq_libvers_Return_get_size(ast_runtime* rt, NCNC_inq_libvers_Return* ncnc_inq_libvers_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&ncnc_inq_libvers_return_v->version);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCNC_inq_libvers_Return_get_size*/

ast_err
NCNC_delete_mp_write(ast_runtime* rt, NCNC_delete_mp* ncnc_delete_mp_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&ncnc_delete_mp_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_int32,2,&ncnc_delete_mp_v->basepe);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCNC_delete_mp_write*/

ast_err
NCNC_delete_mp_read(ast_runtime* rt, NCNC_delete_mp** ncnc_delete_mp_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_delete_mp* ncnc_delete_mp_v;
    unsigned long pos;

    ncnc_delete_mp_v = (NCNC_delete_mp*)ast_alloc(rt,sizeof(NCNC_delete_mp));
    if(ncnc_delete_mp_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_delete_mp|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&ncnc_delete_mp_v->path);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_int32,2,&ncnc_delete_mp_v->basepe);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_delete_mp_vp) *ncnc_delete_mp_vp = ncnc_delete_mp_v;
done:
    return ACATCH(status);
} /*NCNC_delete_mp_read*/

ast_err
NCNC_delete_mp_reclaim(ast_runtime* rt, NCNC_delete_mp* ncnc_delete_mp_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,ncnc_delete_mp_v->path);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)ncnc_delete_mp_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_delete_mp_reclaim*/

size_t
NCNC_delete_mp_get_size(ast_runtime* rt, NCNC_delete_mp* ncnc_delete_mp_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&ncnc_delete_mp_v->path);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_int32,&ncnc_delete_mp_v->basepe);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCNC_delete_mp_get_size*/

ast_err
NCNC_delete_mp_Return_write(ast_runtime* rt, NCNC_delete_mp_Return* ncnc_delete_mp_return_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_int32,1,&ncnc_delete_mp_return_v->ncstatus);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*NCNC_delete_mp_Return_write*/

ast_err
NCNC_delete_mp_Return_read(ast_runtime* rt, NCNC_delete_mp_Return** ncnc_delete_mp_return_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    NCNC_delete_mp_Return* ncnc_delete_mp_return_v;
    unsigned long pos;

    ncnc_delete_mp_return_v = (NCNC_delete_mp_Return*)ast_alloc(rt,sizeof(NCNC_delete_mp_Return));
    if(ncnc_delete_mp_return_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|NCNC_delete_mp_Return|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_int32,1,&ncnc_delete_mp_return_v->ncstatus);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(ncnc_delete_mp_return_vp) *ncnc_delete_mp_return_vp = ncnc_delete_mp_return_v;
done:
    return ACATCH(status);
} /*NCNC_delete_mp_Return_read*/

ast_err
NCNC_delete_mp_Return_reclaim(ast_runtime* rt, NCNC_delete_mp_Return* ncnc_delete_mp_return_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)ncnc_delete_mp_return_v);
    goto done;

done:
    return ACATCH(status);

} /*NCNC_delete_mp_Return_reclaim*/

size_t
NCNC_delete_mp_Return_get_size(ast_runtime* rt, NCNC_delete_mp_Return* ncnc_delete_mp_return_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_int32,&ncnc_delete_mp_return_v->ncstatus);
        totalsize += fieldsize;
    }
    return totalsize;

} /*NCNC_delete_mp_Return_get_size*/

ast_err
MetaNode_write(ast_runtime* rt, MetaNode* metanode_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        status = ast_write_tag(rt,ast_counted,1);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        size = MetaNode_get_size(rt,metanode_v->root);
        status = ast_write_size(rt,size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        status = MetaNode_write(rt,metanode_v->root);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,2,&metanode_v->nodeclass);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,3,&metanode_v->subclass);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(metanode_v->ncid.defined) {
            status = ast_write_primitive(rt,ast_int32,4,&metanode_v->ncid.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->typeid.defined) {
            status = ast_write_primitive(rt,ast_int32,5,&metanode_v->typeid.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->name.defined) {
            status = ast_write_primitive(rt,ast_string,6,&metanode_v->name.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->size.defined) {
            status = ast_write_primitive(rt,ast_uint64,7,&metanode_v->size.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->basetype.defined) {
            status = ast_write_tag(rt,ast_counted,8);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metanode_v->basetype.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metanode_v->basetype.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->graph.defined) {
            status = ast_write_tag(rt,ast_counted,9);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaGraph_get_size(rt,metanode_v->graph.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaGraph_write(rt,metanode_v->graph.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->group.defined) {
            status = ast_write_tag(rt,ast_counted,10);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaGroup_get_size(rt,metanode_v->group.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaGroup_write(rt,metanode_v->group.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->var.defined) {
            status = ast_write_tag(rt,ast_counted,11);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaVar_get_size(rt,metanode_v->var.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaVar_write(rt,metanode_v->var.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->dim.defined) {
            status = ast_write_tag(rt,ast_counted,12);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaDim_get_size(rt,metanode_v->dim.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaDim_write(rt,metanode_v->dim.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->compound_t.defined) {
            status = ast_write_tag(rt,ast_counted,13);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaCompound_get_size(rt,metanode_v->compound_t.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaCompound_write(rt,metanode_v->compound_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        if(metanode_v->enum_t.defined) {
            status = ast_write_tag(rt,ast_counted,14);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaEnum_get_size(rt,metanode_v->enum_t.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaEnum_write(rt,metanode_v->enum_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*MetaNode_write*/

ast_err
MetaNode_read(ast_runtime* rt, MetaNode** metanode_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaNode* metanode_v;
    unsigned long pos;

    metanode_v = (MetaNode*)ast_alloc(rt,sizeof(MetaNode));
    if(metanode_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaNode|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&metanode_v->root);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_enum,2,&metanode_v->nodeclass);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_enum,3,&metanode_v->subclass);
            } break;
        case 4: {
            metanode_v->ncid.defined = 1;
            metanode_v->ncid.value = 0;
            status = ast_read_primitive(rt,ast_int32,4,&metanode_v->ncid.value);
            } break;
        case 5: {
            metanode_v->typeid.defined = 1;
            metanode_v->typeid.value = 0;
            status = ast_read_primitive(rt,ast_int32,5,&metanode_v->typeid.value);
            } break;
        case 6: {
            metanode_v->name.defined = 1;
            metanode_v->name.value = NULL;
            status = ast_read_primitive(rt,ast_string,6,&metanode_v->name.value);
            } break;
        case 7: {
            metanode_v->size.defined = 1;
            metanode_v->size.value = 0;
            status = ast_read_primitive(rt,ast_uint64,7,&metanode_v->size.value);
            } break;
        case 8: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->basetype.defined = 1;
            metanode_v->basetype.value = NULL;
            status = MetaNode_read(rt,&metanode_v->basetype.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 9: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->graph.defined = 1;
            metanode_v->graph.value = NULL;
            status = MetaGraph_read(rt,&metanode_v->graph.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 10: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->group.defined = 1;
            metanode_v->group.value = NULL;
            status = MetaGroup_read(rt,&metanode_v->group.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 11: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->var.defined = 1;
            metanode_v->var.value = NULL;
            status = MetaVar_read(rt,&metanode_v->var.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 12: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->dim.defined = 1;
            metanode_v->dim.value = NULL;
            status = MetaDim_read(rt,&metanode_v->dim.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 13: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->compound_t.defined = 1;
            metanode_v->compound_t.value = NULL;
            status = MetaCompound_read(rt,&metanode_v->compound_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 14: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            metanode_v->enum_t.defined = 1;
            metanode_v->enum_t.value = NULL;
            status = MetaEnum_read(rt,&metanode_v->enum_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!metanode_v->ncid.defined) {
        metanode_v->ncid.defined = 1;
        metanode_v->ncid.value = 0;
    }
    if(!metanode_v->typeid.defined) {
        metanode_v->typeid.defined = 1;
        metanode_v->typeid.value = 0;
    }
    if(!metanode_v->name.defined) {
        metanode_v->name.value = NULL;
    }
    if(!metanode_v->size.defined) {
        metanode_v->size.defined = 1;
        metanode_v->size.value = 0;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metanode_vp) *metanode_vp = metanode_v;
done:
    return ACATCH(status);
} /*MetaNode_read*/

ast_err
MetaNode_reclaim(ast_runtime* rt, MetaNode* metanode_v)
{
    ast_err status = AST_NOERR;

    {
        status = MetaNode_reclaim(rt,metanode_v->root);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(metanode_v->name.defined) {
            status = ast_reclaim_string(rt,metanode_v->name.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->basetype.defined) {
            status = MetaNode_reclaim(rt,metanode_v->basetype.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->graph.defined) {
            status = MetaGraph_reclaim(rt,metanode_v->graph.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->group.defined) {
            status = MetaGroup_reclaim(rt,metanode_v->group.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->var.defined) {
            status = MetaVar_reclaim(rt,metanode_v->var.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->dim.defined) {
            status = MetaDim_reclaim(rt,metanode_v->dim.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->compound_t.defined) {
            status = MetaCompound_reclaim(rt,metanode_v->compound_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(metanode_v->enum_t.defined) {
            status = MetaEnum_reclaim(rt,metanode_v->enum_t.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    ast_free(rt,(void*)metanode_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaNode_reclaim*/

size_t
MetaNode_get_size(ast_runtime* rt, MetaNode* metanode_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += MetaNode_get_size(rt,metanode_v->root);
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_enum,&metanode_v->nodeclass);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_enum,&metanode_v->subclass);
        totalsize += fieldsize;
    }
    {
        if(metanode_v->ncid.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_int32,&metanode_v->ncid.value);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->typeid.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_int32,&metanode_v->typeid.value);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->name.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_string,&metanode_v->name.value);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->size.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_uint64,&metanode_v->size.value);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->basetype.defined) {
            fieldsize += MetaNode_get_size(rt,metanode_v->basetype.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,8);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->graph.defined) {
            fieldsize += MetaGraph_get_size(rt,metanode_v->graph.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,9);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->group.defined) {
            fieldsize += MetaGroup_get_size(rt,metanode_v->group.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,10);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->var.defined) {
            fieldsize += MetaVar_get_size(rt,metanode_v->var.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,11);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->dim.defined) {
            fieldsize += MetaDim_get_size(rt,metanode_v->dim.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,12);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->compound_t.defined) {
            fieldsize += MetaCompound_get_size(rt,metanode_v->compound_t.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,13);
        }
        totalsize += fieldsize;
    }
    {
        if(metanode_v->enum_t.defined) {
            fieldsize += MetaEnum_get_size(rt,metanode_v->enum_t.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,14);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaNode_get_size*/

ast_err
MetaGraph_write(ast_runtime* rt, MetaGraph* metagraph_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<metagraph_v->nodeset.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metagraph_v->nodeset.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metagraph_v->nodeset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        status = ast_write_tag(rt,ast_counted,2);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        size = MetaNode_get_size(rt,metagraph_v->rootgroup);
        status = ast_write_size(rt,size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        status = MetaNode_write(rt,metagraph_v->rootgroup);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*MetaGraph_write*/

ast_err
MetaGraph_read(ast_runtime* rt, MetaGraph** metagraph_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaGraph* metagraph_v;
    unsigned long pos;

    metagraph_v = (MetaGraph*)ast_alloc(rt,sizeof(MetaGraph));
    if(metagraph_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaGraph|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metagraph_v->nodeset,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 2: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&metagraph_v->rootgroup);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metagraph_vp) *metagraph_vp = metagraph_v;
done:
    return ACATCH(status);
} /*MetaGraph_read*/

ast_err
MetaGraph_reclaim(ast_runtime* rt, MetaGraph* metagraph_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<metagraph_v->nodeset.count;i++) {
            status = MetaNode_reclaim(rt,metagraph_v->nodeset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metagraph_v->nodeset.values);
    }
    {
        status = MetaNode_reclaim(rt,metagraph_v->rootgroup);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)metagraph_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaGraph_reclaim*/

size_t
MetaGraph_get_size(ast_runtime* rt, MetaGraph* metagraph_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metagraph_v->nodeset.count;i++) {
            fieldsize += MetaNode_get_size(rt,metagraph_v->nodeset.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += MetaNode_get_size(rt,metagraph_v->rootgroup);
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaGraph_get_size*/

ast_err
MetaGroup_write(ast_runtime* rt, MetaGroup* metagroup_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<metagroup_v->typeset.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metagroup_v->typeset.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metagroup_v->typeset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<metagroup_v->varset.count;i++) {
            status = ast_write_tag(rt,ast_counted,2);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metagroup_v->varset.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metagroup_v->varset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<metagroup_v->dimrset.count;i++) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metagroup_v->dimrset.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metagroup_v->dimrset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<metagroup_v->groups.count;i++) {
            status = ast_write_tag(rt,ast_counted,4);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metagroup_v->groups.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metagroup_v->groups.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*MetaGroup_write*/

ast_err
MetaGroup_read(ast_runtime* rt, MetaGroup** metagroup_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaGroup* metagroup_v;
    unsigned long pos;

    metagroup_v = (MetaGroup*)ast_alloc(rt,sizeof(MetaGroup));
    if(metagroup_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaGroup|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metagroup_v->typeset,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 2: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metagroup_v->varset,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 3: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metagroup_v->dimrset,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 4: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metagroup_v->groups,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metagroup_vp) *metagroup_vp = metagroup_v;
done:
    return ACATCH(status);
} /*MetaGroup_read*/

ast_err
MetaGroup_reclaim(ast_runtime* rt, MetaGroup* metagroup_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<metagroup_v->typeset.count;i++) {
            status = MetaNode_reclaim(rt,metagroup_v->typeset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metagroup_v->typeset.values);
    }
    {
        int i;
        for(i=0;i<metagroup_v->varset.count;i++) {
            status = MetaNode_reclaim(rt,metagroup_v->varset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metagroup_v->varset.values);
    }
    {
        int i;
        for(i=0;i<metagroup_v->dimrset.count;i++) {
            status = MetaNode_reclaim(rt,metagroup_v->dimrset.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metagroup_v->dimrset.values);
    }
    {
        int i;
        for(i=0;i<metagroup_v->groups.count;i++) {
            status = MetaNode_reclaim(rt,metagroup_v->groups.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metagroup_v->groups.values);
    }
    ast_free(rt,(void*)metagroup_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaGroup_reclaim*/

size_t
MetaGroup_get_size(ast_runtime* rt, MetaGroup* metagroup_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metagroup_v->typeset.count;i++) {
            fieldsize += MetaNode_get_size(rt,metagroup_v->typeset.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<metagroup_v->varset.count;i++) {
            fieldsize += MetaNode_get_size(rt,metagroup_v->varset.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<metagroup_v->dimrset.count;i++) {
            fieldsize += MetaNode_get_size(rt,metagroup_v->dimrset.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<metagroup_v->groups.count;i++) {
            fieldsize += MetaNode_get_size(rt,metagroup_v->groups.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaGroup_get_size*/

ast_err
MetaVar_write(ast_runtime* rt, MetaVar* metavar_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<metavar_v->dims.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metavar_v->dims.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metavar_v->dims.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*MetaVar_write*/

ast_err
MetaVar_read(ast_runtime* rt, MetaVar** metavar_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaVar* metavar_v;
    unsigned long pos;

    metavar_v = (MetaVar*)ast_alloc(rt,sizeof(MetaVar));
    if(metavar_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaVar|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metavar_v->dims,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metavar_vp) *metavar_vp = metavar_v;
done:
    return ACATCH(status);
} /*MetaVar_read*/

ast_err
MetaVar_reclaim(ast_runtime* rt, MetaVar* metavar_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<metavar_v->dims.count;i++) {
            status = MetaNode_reclaim(rt,metavar_v->dims.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metavar_v->dims.values);
    }
    ast_free(rt,(void*)metavar_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaVar_reclaim*/

size_t
MetaVar_get_size(ast_runtime* rt, MetaVar* metavar_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metavar_v->dims.count;i++) {
            fieldsize += MetaNode_get_size(rt,metavar_v->dims.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaVar_get_size*/

ast_err
MetaDim_write(ast_runtime* rt, MetaDim* metadim_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_uint64,1,&metadim_v->actualsize);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*MetaDim_write*/

ast_err
MetaDim_read(ast_runtime* rt, MetaDim** metadim_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaDim* metadim_v;
    unsigned long pos;

    metadim_v = (MetaDim*)ast_alloc(rt,sizeof(MetaDim));
    if(metadim_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaDim|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_uint64,1,&metadim_v->actualsize);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metadim_vp) *metadim_vp = metadim_v;
done:
    return ACATCH(status);
} /*MetaDim_read*/

ast_err
MetaDim_reclaim(ast_runtime* rt, MetaDim* metadim_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)metadim_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaDim_reclaim*/

size_t
MetaDim_get_size(ast_runtime* rt, MetaDim* metadim_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_uint64,&metadim_v->actualsize);
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaDim_get_size*/

ast_err
MetaCompound_write(ast_runtime* rt, MetaCompound* metacompound_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<metacompound_v->fields.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaNode_get_size(rt,metacompound_v->fields.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaNode_write(rt,metacompound_v->fields.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*MetaCompound_write*/

ast_err
MetaCompound_read(ast_runtime* rt, MetaCompound** metacompound_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaCompound* metacompound_v;
    unsigned long pos;

    metacompound_v = (MetaCompound*)ast_alloc(rt,sizeof(MetaCompound));
    if(metacompound_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaCompound|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            MetaNode* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaNode_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metacompound_v->fields,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metacompound_vp) *metacompound_vp = metacompound_v;
done:
    return ACATCH(status);
} /*MetaCompound_read*/

ast_err
MetaCompound_reclaim(ast_runtime* rt, MetaCompound* metacompound_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<metacompound_v->fields.count;i++) {
            status = MetaNode_reclaim(rt,metacompound_v->fields.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metacompound_v->fields.values);
    }
    ast_free(rt,(void*)metacompound_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaCompound_reclaim*/

size_t
MetaCompound_get_size(ast_runtime* rt, MetaCompound* metacompound_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metacompound_v->fields.count;i++) {
            fieldsize += MetaNode_get_size(rt,metacompound_v->fields.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaCompound_get_size*/

ast_err
MetaField_write(ast_runtime* rt, MetaField* metafield_v)
{
    ast_err status = AST_NOERR;

    {
        int i = 0;
        for(i=0;i<metafield_v->dims.count;i++) {
            status = ast_write_primitive(rt,ast_uint64,1,&metafield_v->dims.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&metafield_v->offset);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint64,3,&metafield_v->alignment);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*MetaField_write*/

ast_err
MetaField_read(ast_runtime* rt, MetaField** metafield_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaField* metafield_v;
    unsigned long pos;

    metafield_v = (MetaField*)ast_alloc(rt,sizeof(MetaField));
    if(metafield_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaField|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            uint64_t tmp;
            status = ast_read_primitive(rt,ast_uint64,1,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint64,&metafield_v->dims,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&metafield_v->offset);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint64,3,&metafield_v->alignment);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metafield_vp) *metafield_vp = metafield_v;
done:
    return ACATCH(status);
} /*MetaField_read*/

ast_err
MetaField_reclaim(ast_runtime* rt, MetaField* metafield_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)metafield_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaField_reclaim*/

size_t
MetaField_get_size(ast_runtime* rt, MetaField* metafield_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metafield_v->dims.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_uint64,&metafield_v->dims.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&metafield_v->offset);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint64,&metafield_v->alignment);
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaField_get_size*/

ast_err
MetaEnum_write(ast_runtime* rt, MetaEnum* metaenum_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<metaenum_v->econsts.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = MetaEconst_get_size(rt,metaenum_v->econsts.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = MetaEconst_write(rt,metaenum_v->econsts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*MetaEnum_write*/

ast_err
MetaEnum_read(ast_runtime* rt, MetaEnum** metaenum_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaEnum* metaenum_v;
    unsigned long pos;

    metaenum_v = (MetaEnum*)ast_alloc(rt,sizeof(MetaEnum));
    if(metaenum_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaEnum|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            MetaEconst* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = MetaEconst_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&metaenum_v->econsts,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metaenum_vp) *metaenum_vp = metaenum_v;
done:
    return ACATCH(status);
} /*MetaEnum_read*/

ast_err
MetaEnum_reclaim(ast_runtime* rt, MetaEnum* metaenum_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<metaenum_v->econsts.count;i++) {
            status = MetaEconst_reclaim(rt,metaenum_v->econsts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,metaenum_v->econsts.values);
    }
    ast_free(rt,(void*)metaenum_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaEnum_reclaim*/

size_t
MetaEnum_get_size(ast_runtime* rt, MetaEnum* metaenum_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<metaenum_v->econsts.count;i++) {
            fieldsize += MetaEconst_get_size(rt,metaenum_v->econsts.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaEnum_get_size*/

ast_err
MetaEconst_write(ast_runtime* rt, MetaEconst* metaeconst_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&metaeconst_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&metaeconst_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*MetaEconst_write*/

ast_err
MetaEconst_read(ast_runtime* rt, MetaEconst** metaeconst_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    MetaEconst* metaeconst_v;
    unsigned long pos;

    metaeconst_v = (MetaEconst*)ast_alloc(rt,sizeof(MetaEconst));
    if(metaeconst_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|MetaEconst|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&metaeconst_v->name);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&metaeconst_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(metaeconst_vp) *metaeconst_vp = metaeconst_v;
done:
    return ACATCH(status);
} /*MetaEconst_read*/

ast_err
MetaEconst_reclaim(ast_runtime* rt, MetaEconst* metaeconst_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,metaeconst_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_reclaim_bytes(rt,&metaeconst_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)metaeconst_v);
    goto done;

done:
    return ACATCH(status);

} /*MetaEconst_reclaim*/

size_t
MetaEconst_get_size(ast_runtime* rt, MetaEconst* metaeconst_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&metaeconst_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&metaeconst_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*MetaEconst_get_size*/

