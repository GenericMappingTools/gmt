#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include <ast_runtime.h>
#include <ast_debug.h>

#include "nccrnode.h"
#include "ncStreamx.h"

ast_err
Attribute_write(ast_runtime* rt, Attribute* attribute_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&attribute_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,2,&attribute_v->type);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_uint32,3,&attribute_v->len);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(attribute_v->data.defined) {
            status = ast_write_primitive(rt,ast_bytes,4,&attribute_v->data.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<attribute_v->sdata.count;i++) {
            status = ast_write_primitive(rt,ast_string,5,&attribute_v->sdata.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Attribute_write*/

ast_err
Attribute_read(ast_runtime* rt, Attribute** attribute_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Attribute* attribute_v;
    unsigned long pos;

    attribute_v = (Attribute*)ast_alloc(rt,sizeof(Attribute));
    if(attribute_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Attribute|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&attribute_v->name);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_enum,2,&attribute_v->type);
            } break;
        case 3: {
            status = ast_read_primitive(rt,ast_uint32,3,&attribute_v->len);
            } break;
        case 4: {
            attribute_v->data.defined = 1;
            attribute_v->data.value.nbytes = 0;
            attribute_v->data.value.bytes = NULL;
            status = ast_read_primitive(rt,ast_bytes,4,&attribute_v->data.value);
            } break;
        case 5: {
            char* tmp;
            status = ast_read_primitive(rt,ast_string,5,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_string,&attribute_v->sdata,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!attribute_v->data.defined) {
        attribute_v->data.value.nbytes = 0;
        attribute_v->data.value.bytes = NULL;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(attribute_vp) *attribute_vp = attribute_v;
done:
    return ACATCH(status);
} /*Attribute_read*/

ast_err
Attribute_reclaim(ast_runtime* rt, Attribute* attribute_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,attribute_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(attribute_v->data.defined) {
            status = ast_reclaim_bytes(rt,&attribute_v->data.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i;
        for(i=0;i<attribute_v->sdata.count;i++) {
            status = ast_reclaim_string(rt,attribute_v->sdata.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,attribute_v->sdata.values);
    }
    ast_free(rt,(void*)attribute_v);
    goto done;

done:
    return ACATCH(status);

} /*Attribute_reclaim*/

size_t
Attribute_get_size(ast_runtime* rt, Attribute* attribute_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&attribute_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_enum,&attribute_v->type);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,3);
        fieldsize += ast_get_size(rt,ast_uint32,&attribute_v->len);
        totalsize += fieldsize;
    }
    {
        if(attribute_v->data.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_bytes,&attribute_v->data.value);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<attribute_v->sdata.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_string,&attribute_v->sdata.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Attribute_get_size*/

ast_err
Dimension_write(ast_runtime* rt, Dimension* dimension_v)
{
    ast_err status = AST_NOERR;

    {
        if(dimension_v->name.defined) {
            status = ast_write_primitive(rt,ast_string,1,&dimension_v->name.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(dimension_v->length.defined) {
            status = ast_write_primitive(rt,ast_uint64,2,&dimension_v->length.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(dimension_v->isUnlimited.defined) {
            status = ast_write_primitive(rt,ast_bool,3,&dimension_v->isUnlimited.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(dimension_v->isVlen.defined) {
            status = ast_write_primitive(rt,ast_bool,4,&dimension_v->isVlen.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(dimension_v->isPrivate.defined) {
            status = ast_write_primitive(rt,ast_bool,5,&dimension_v->isPrivate.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Dimension_write*/

ast_err
Dimension_read(ast_runtime* rt, Dimension** dimension_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Dimension* dimension_v;
    unsigned long pos;

    dimension_v = (Dimension*)ast_alloc(rt,sizeof(Dimension));
    if(dimension_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Dimension|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            dimension_v->name.defined = 1;
            dimension_v->name.value = NULL;
            status = ast_read_primitive(rt,ast_string,1,&dimension_v->name.value);
            } break;
        case 2: {
            dimension_v->length.defined = 1;
            dimension_v->length.value = 0;
            status = ast_read_primitive(rt,ast_uint64,2,&dimension_v->length.value);
            } break;
        case 3: {
            dimension_v->isUnlimited.defined = 1;
            dimension_v->isUnlimited.value = 0;
            status = ast_read_primitive(rt,ast_bool,3,&dimension_v->isUnlimited.value);
            } break;
        case 4: {
            dimension_v->isVlen.defined = 1;
            dimension_v->isVlen.value = 0;
            status = ast_read_primitive(rt,ast_bool,4,&dimension_v->isVlen.value);
            } break;
        case 5: {
            dimension_v->isPrivate.defined = 1;
            dimension_v->isPrivate.value = 0;
            status = ast_read_primitive(rt,ast_bool,5,&dimension_v->isPrivate.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!dimension_v->name.defined) {
        dimension_v->name.value = NULL;
    }
    if(!dimension_v->length.defined) {
        dimension_v->length.defined = 1;
        dimension_v->length.value = 0;
    }
    if(!dimension_v->isUnlimited.defined) {
        dimension_v->isUnlimited.defined = 1;
        dimension_v->isUnlimited.value = 0;
    }
    if(!dimension_v->isVlen.defined) {
        dimension_v->isVlen.defined = 1;
        dimension_v->isVlen.value = 0;
    }
    if(!dimension_v->isPrivate.defined) {
        dimension_v->isPrivate.defined = 1;
        dimension_v->isPrivate.value = 0;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(dimension_vp) *dimension_vp = dimension_v;
done:
    return ACATCH(status);
} /*Dimension_read*/

ast_err
Dimension_reclaim(ast_runtime* rt, Dimension* dimension_v)
{
    ast_err status = AST_NOERR;

    {
        if(dimension_v->name.defined) {
            status = ast_reclaim_string(rt,dimension_v->name.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    ast_free(rt,(void*)dimension_v);
    goto done;

done:
    return ACATCH(status);

} /*Dimension_reclaim*/

size_t
Dimension_get_size(ast_runtime* rt, Dimension* dimension_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        if(dimension_v->name.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_string,&dimension_v->name.value);
        }
        totalsize += fieldsize;
    }
    {
        if(dimension_v->length.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_uint64,&dimension_v->length.value);
        }
        totalsize += fieldsize;
    }
    {
        if(dimension_v->isUnlimited.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_bool,&dimension_v->isUnlimited.value);
        }
        totalsize += fieldsize;
    }
    {
        if(dimension_v->isVlen.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_bool,&dimension_v->isVlen.value);
        }
        totalsize += fieldsize;
    }
    {
        if(dimension_v->isPrivate.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_bool,&dimension_v->isPrivate.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Dimension_get_size*/

ast_err
Variable_write(ast_runtime* rt, Variable* variable_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&variable_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,2,&variable_v->dataType);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        size_t size;
        int i;
        for(i=0;i<variable_v->shape.count;i++) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Dimension_get_size(rt,variable_v->shape.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Dimension_write(rt,variable_v->shape.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<variable_v->atts.count;i++) {
            status = ast_write_tag(rt,ast_counted,4);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Attribute_get_size(rt,variable_v->atts.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Attribute_write(rt,variable_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(variable_v->unsigned_.defined) {
            status = ast_write_primitive(rt,ast_bool,5,&variable_v->unsigned_.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(variable_v->data.defined) {
            status = ast_write_primitive(rt,ast_bytes,6,&variable_v->data.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(variable_v->enumType.defined) {
            status = ast_write_primitive(rt,ast_string,7,&variable_v->enumType.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<variable_v->dimIndex.count;i++) {
            status = ast_write_primitive(rt,ast_uint32,8,&variable_v->dimIndex.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Variable_write*/

ast_err
Variable_read(ast_runtime* rt, Variable** variable_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Variable* variable_v;
    unsigned long pos;

    variable_v = (Variable*)ast_alloc(rt,sizeof(Variable));
    if(variable_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Variable|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&variable_v->name);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_enum,2,&variable_v->dataType);
            } break;
        case 3: {
            size_t size;
            Dimension* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Dimension_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&variable_v->shape,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 4: {
            size_t size;
            Attribute* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Attribute_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&variable_v->atts,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 5: {
            variable_v->unsigned_.defined = 1;
            variable_v->unsigned_.value = 0;
            status = ast_read_primitive(rt,ast_bool,5,&variable_v->unsigned_.value);
            } break;
        case 6: {
            variable_v->data.defined = 1;
            variable_v->data.value.nbytes = 0;
            variable_v->data.value.bytes = NULL;
            status = ast_read_primitive(rt,ast_bytes,6,&variable_v->data.value);
            } break;
        case 7: {
            variable_v->enumType.defined = 1;
            variable_v->enumType.value = NULL;
            status = ast_read_primitive(rt,ast_string,7,&variable_v->enumType.value);
            } break;
        case 8: {
            uint32_t tmp;
            status = ast_read_primitive(rt,ast_uint32,8,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint32,&variable_v->dimIndex,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!variable_v->unsigned_.defined) {
        variable_v->unsigned_.defined = 1;
        variable_v->unsigned_.value = 0;
    }
    if(!variable_v->data.defined) {
        variable_v->data.value.nbytes = 0;
        variable_v->data.value.bytes = NULL;
    }
    if(!variable_v->enumType.defined) {
        variable_v->enumType.value = NULL;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(variable_vp) *variable_vp = variable_v;
done:
    return ACATCH(status);
} /*Variable_read*/

ast_err
Variable_reclaim(ast_runtime* rt, Variable* variable_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,variable_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i;
        for(i=0;i<variable_v->shape.count;i++) {
            status = Dimension_reclaim(rt,variable_v->shape.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,variable_v->shape.values);
    }
    {
        int i;
        for(i=0;i<variable_v->atts.count;i++) {
            status = Attribute_reclaim(rt,variable_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,variable_v->atts.values);
    }
    {
        if(variable_v->data.defined) {
            status = ast_reclaim_bytes(rt,&variable_v->data.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(variable_v->enumType.defined) {
            status = ast_reclaim_string(rt,variable_v->enumType.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    ast_free(rt,(void*)variable_v);
    goto done;

done:
    return ACATCH(status);

} /*Variable_reclaim*/

size_t
Variable_get_size(ast_runtime* rt, Variable* variable_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&variable_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_enum,&variable_v->dataType);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<variable_v->shape.count;i++) {
            fieldsize += Dimension_get_size(rt,variable_v->shape.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<variable_v->atts.count;i++) {
            fieldsize += Attribute_get_size(rt,variable_v->atts.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
        }
        totalsize += fieldsize;
    }
    {
        if(variable_v->unsigned_.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_bool,&variable_v->unsigned_.value);
        }
        totalsize += fieldsize;
    }
    {
        if(variable_v->data.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_bytes,&variable_v->data.value);
        }
        totalsize += fieldsize;
    }
    {
        if(variable_v->enumType.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_string,&variable_v->enumType.value);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<variable_v->dimIndex.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,8);
            fieldsize += ast_get_size(rt,ast_uint32,&variable_v->dimIndex.values[i]);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Variable_get_size*/

ast_err
Structure_write(ast_runtime* rt, Structure* structure_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&structure_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,2,&structure_v->dataType);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        size_t size;
        int i;
        for(i=0;i<structure_v->shape.count;i++) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Dimension_get_size(rt,structure_v->shape.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Dimension_write(rt,structure_v->shape.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<structure_v->atts.count;i++) {
            status = ast_write_tag(rt,ast_counted,4);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Attribute_get_size(rt,structure_v->atts.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Attribute_write(rt,structure_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<structure_v->vars.count;i++) {
            status = ast_write_tag(rt,ast_counted,5);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Variable_get_size(rt,structure_v->vars.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Variable_write(rt,structure_v->vars.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<structure_v->structs.count;i++) {
            status = ast_write_tag(rt,ast_counted,6);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Structure_get_size(rt,structure_v->structs.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Structure_write(rt,structure_v->structs.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Structure_write*/

ast_err
Structure_read(ast_runtime* rt, Structure** structure_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Structure* structure_v;
    unsigned long pos;

    structure_v = (Structure*)ast_alloc(rt,sizeof(Structure));
    if(structure_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Structure|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&structure_v->name);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_enum,2,&structure_v->dataType);
            } break;
        case 3: {
            size_t size;
            Dimension* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Dimension_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&structure_v->shape,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 4: {
            size_t size;
            Attribute* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Attribute_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&structure_v->atts,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 5: {
            size_t size;
            Variable* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Variable_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&structure_v->vars,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 6: {
            size_t size;
            Structure* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Structure_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&structure_v->structs,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(structure_vp) *structure_vp = structure_v;
done:
    return ACATCH(status);
} /*Structure_read*/

ast_err
Structure_reclaim(ast_runtime* rt, Structure* structure_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,structure_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i;
        for(i=0;i<structure_v->shape.count;i++) {
            status = Dimension_reclaim(rt,structure_v->shape.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,structure_v->shape.values);
    }
    {
        int i;
        for(i=0;i<structure_v->atts.count;i++) {
            status = Attribute_reclaim(rt,structure_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,structure_v->atts.values);
    }
    {
        int i;
        for(i=0;i<structure_v->vars.count;i++) {
            status = Variable_reclaim(rt,structure_v->vars.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,structure_v->vars.values);
    }
    {
        int i;
        for(i=0;i<structure_v->structs.count;i++) {
            status = Structure_reclaim(rt,structure_v->structs.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,structure_v->structs.values);
    }
    ast_free(rt,(void*)structure_v);
    goto done;

done:
    return ACATCH(status);

} /*Structure_reclaim*/

size_t
Structure_get_size(ast_runtime* rt, Structure* structure_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&structure_v->name);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_enum,&structure_v->dataType);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structure_v->shape.count;i++) {
            fieldsize += Dimension_get_size(rt,structure_v->shape.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structure_v->atts.count;i++) {
            fieldsize += Attribute_get_size(rt,structure_v->atts.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structure_v->vars.count;i++) {
            fieldsize += Variable_get_size(rt,structure_v->vars.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structure_v->structs.count;i++) {
            fieldsize += Structure_get_size(rt,structure_v->structs.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Structure_get_size*/

ast_err
EnumTypedef_write(ast_runtime* rt, EnumTypedef* enumtypedef_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&enumtypedef_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        size_t size;
        int i;
        for(i=0;i<enumtypedef_v->map.count;i++) {
            status = ast_write_tag(rt,ast_counted,2);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = EnumType_get_size(rt,enumtypedef_v->map.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = EnumType_write(rt,enumtypedef_v->map.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*EnumTypedef_write*/

ast_err
EnumTypedef_read(ast_runtime* rt, EnumTypedef** enumtypedef_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    EnumTypedef* enumtypedef_v;
    unsigned long pos;

    enumtypedef_v = (EnumTypedef*)ast_alloc(rt,sizeof(EnumTypedef));
    if(enumtypedef_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|EnumTypedef|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&enumtypedef_v->name);
            } break;
        case 2: {
            size_t size;
            EnumType* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = EnumType_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&enumtypedef_v->map,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(enumtypedef_vp) *enumtypedef_vp = enumtypedef_v;
done:
    return ACATCH(status);
} /*EnumTypedef_read*/

ast_err
EnumTypedef_reclaim(ast_runtime* rt, EnumTypedef* enumtypedef_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,enumtypedef_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i;
        for(i=0;i<enumtypedef_v->map.count;i++) {
            status = EnumType_reclaim(rt,enumtypedef_v->map.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,enumtypedef_v->map.values);
    }
    ast_free(rt,(void*)enumtypedef_v);
    goto done;

done:
    return ACATCH(status);

} /*EnumTypedef_reclaim*/

size_t
EnumTypedef_get_size(ast_runtime* rt, EnumTypedef* enumtypedef_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&enumtypedef_v->name);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<enumtypedef_v->map.count;i++) {
            fieldsize += EnumType_get_size(rt,enumtypedef_v->map.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*EnumTypedef_get_size*/

ast_err
EnumType_write(ast_runtime* rt, EnumType* enumtype_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_uint32,1,&enumtype_v->code);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_string,2,&enumtype_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*EnumType_write*/

ast_err
EnumType_read(ast_runtime* rt, EnumType** enumtype_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    EnumType* enumtype_v;
    unsigned long pos;

    enumtype_v = (EnumType*)ast_alloc(rt,sizeof(EnumType));
    if(enumtype_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|EnumType|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_uint32,1,&enumtype_v->code);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_string,2,&enumtype_v->value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(enumtype_vp) *enumtype_vp = enumtype_v;
done:
    return ACATCH(status);
} /*EnumType_read*/

ast_err
EnumType_reclaim(ast_runtime* rt, EnumType* enumtype_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,enumtype_v->value);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)enumtype_v);
    goto done;

done:
    return ACATCH(status);

} /*EnumType_reclaim*/

size_t
EnumType_get_size(ast_runtime* rt, EnumType* enumtype_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_uint32,&enumtype_v->code);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_string,&enumtype_v->value);
        totalsize += fieldsize;
    }
    return totalsize;

} /*EnumType_get_size*/

ast_err
Group_write(ast_runtime* rt, Group* group_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&group_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->dims.count;i++) {
            status = ast_write_tag(rt,ast_counted,2);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Dimension_get_size(rt,group_v->dims.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Dimension_write(rt,group_v->dims.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->vars.count;i++) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Variable_get_size(rt,group_v->vars.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Variable_write(rt,group_v->vars.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->structs.count;i++) {
            status = ast_write_tag(rt,ast_counted,4);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Structure_get_size(rt,group_v->structs.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Structure_write(rt,group_v->structs.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->atts.count;i++) {
            status = ast_write_tag(rt,ast_counted,5);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Attribute_get_size(rt,group_v->atts.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Attribute_write(rt,group_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->groups.count;i++) {
            status = ast_write_tag(rt,ast_counted,6);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Group_get_size(rt,group_v->groups.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Group_write(rt,group_v->groups.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        int i;
        for(i=0;i<group_v->enumTypes.count;i++) {
            status = ast_write_tag(rt,ast_counted,7);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = EnumTypedef_get_size(rt,group_v->enumTypes.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = EnumTypedef_write(rt,group_v->enumTypes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Group_write*/

ast_err
Group_read(ast_runtime* rt, Group** group_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Group* group_v;
    unsigned long pos;

    group_v = (Group*)ast_alloc(rt,sizeof(Group));
    if(group_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Group|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&group_v->name);
            } break;
        case 2: {
            size_t size;
            Dimension* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Dimension_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->dims,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 3: {
            size_t size;
            Variable* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Variable_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->vars,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 4: {
            size_t size;
            Structure* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Structure_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->structs,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 5: {
            size_t size;
            Attribute* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Attribute_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->atts,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 6: {
            size_t size;
            Group* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Group_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->groups,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 7: {
            size_t size;
            EnumTypedef* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = EnumTypedef_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&group_v->enumTypes,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(group_vp) *group_vp = group_v;
done:
    return ACATCH(status);
} /*Group_read*/

ast_err
Group_reclaim(ast_runtime* rt, Group* group_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,group_v->name);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i;
        for(i=0;i<group_v->dims.count;i++) {
            status = Dimension_reclaim(rt,group_v->dims.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->dims.values);
    }
    {
        int i;
        for(i=0;i<group_v->vars.count;i++) {
            status = Variable_reclaim(rt,group_v->vars.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->vars.values);
    }
    {
        int i;
        for(i=0;i<group_v->structs.count;i++) {
            status = Structure_reclaim(rt,group_v->structs.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->structs.values);
    }
    {
        int i;
        for(i=0;i<group_v->atts.count;i++) {
            status = Attribute_reclaim(rt,group_v->atts.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->atts.values);
    }
    {
        int i;
        for(i=0;i<group_v->groups.count;i++) {
            status = Group_reclaim(rt,group_v->groups.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->groups.values);
    }
    {
        int i;
        for(i=0;i<group_v->enumTypes.count;i++) {
            status = EnumTypedef_reclaim(rt,group_v->enumTypes.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,group_v->enumTypes.values);
    }
    ast_free(rt,(void*)group_v);
    goto done;

done:
    return ACATCH(status);

} /*Group_reclaim*/

size_t
Group_get_size(ast_runtime* rt, Group* group_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&group_v->name);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->dims.count;i++) {
            fieldsize += Dimension_get_size(rt,group_v->dims.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->vars.count;i++) {
            fieldsize += Variable_get_size(rt,group_v->vars.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->structs.count;i++) {
            fieldsize += Structure_get_size(rt,group_v->structs.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->atts.count;i++) {
            fieldsize += Attribute_get_size(rt,group_v->atts.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->groups.count;i++) {
            fieldsize += Group_get_size(rt,group_v->groups.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<group_v->enumTypes.count;i++) {
            fieldsize += EnumTypedef_get_size(rt,group_v->enumTypes.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Group_get_size*/

ast_err
Header_write(ast_runtime* rt, Header* header_v)
{
    ast_err status = AST_NOERR;

    {
        if(header_v->location.defined) {
            status = ast_write_primitive(rt,ast_string,1,&header_v->location.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(header_v->title.defined) {
            status = ast_write_primitive(rt,ast_string,2,&header_v->title.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(header_v->id.defined) {
            status = ast_write_primitive(rt,ast_string,3,&header_v->id.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        size_t size;
        status = ast_write_tag(rt,ast_counted,4);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        size = Group_get_size(rt,header_v->root);
        status = ast_write_size(rt,size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
        status = Group_write(rt,header_v->root);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(header_v->version.defined) {
            status = ast_write_primitive(rt,ast_uint32,5,&header_v->version.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Header_write*/

ast_err
Header_read(ast_runtime* rt, Header** header_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Header* header_v;
    unsigned long pos;

    header_v = (Header*)ast_alloc(rt,sizeof(Header));
    if(header_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Header|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            header_v->location.defined = 1;
            header_v->location.value = NULL;
            status = ast_read_primitive(rt,ast_string,1,&header_v->location.value);
            } break;
        case 2: {
            header_v->title.defined = 1;
            header_v->title.value = NULL;
            status = ast_read_primitive(rt,ast_string,2,&header_v->title.value);
            } break;
        case 3: {
            header_v->id.defined = 1;
            header_v->id.value = NULL;
            status = ast_read_primitive(rt,ast_string,3,&header_v->id.value);
            } break;
        case 4: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Group_read(rt,&header_v->root);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 5: {
            header_v->version.defined = 1;
            header_v->version.value = 0;
            status = ast_read_primitive(rt,ast_uint32,5,&header_v->version.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!header_v->location.defined) {
        header_v->location.value = NULL;
    }
    if(!header_v->title.defined) {
        header_v->title.value = NULL;
    }
    if(!header_v->id.defined) {
        header_v->id.value = NULL;
    }
    if(!header_v->version.defined) {
        header_v->version.defined = 1;
        header_v->version.value = 0;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(header_vp) *header_vp = header_v;
done:
    return ACATCH(status);
} /*Header_read*/

ast_err
Header_reclaim(ast_runtime* rt, Header* header_v)
{
    ast_err status = AST_NOERR;

    {
        if(header_v->location.defined) {
            status = ast_reclaim_string(rt,header_v->location.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(header_v->title.defined) {
            status = ast_reclaim_string(rt,header_v->title.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(header_v->id.defined) {
            status = ast_reclaim_string(rt,header_v->id.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = Group_reclaim(rt,header_v->root);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)header_v);
    goto done;

done:
    return ACATCH(status);

} /*Header_reclaim*/

size_t
Header_get_size(ast_runtime* rt, Header* header_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        if(header_v->location.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_string,&header_v->location.value);
        }
        totalsize += fieldsize;
    }
    {
        if(header_v->title.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,2);
            fieldsize += ast_get_size(rt,ast_string,&header_v->title.value);
        }
        totalsize += fieldsize;
    }
    {
        if(header_v->id.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_string,&header_v->id.value);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += Group_get_size(rt,header_v->root);
        fieldsize += ast_get_tagsize(rt,ast_counted,4);
        fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
        totalsize += fieldsize;
    }
    {
        if(header_v->version.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint32,&header_v->version.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Header_get_size*/

ast_err
Data_write(ast_runtime* rt, Data* data_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&data_v->varName);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        status = ast_write_primitive(rt,ast_enum,2,&data_v->dataType);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        size_t size;
        if(data_v->section.defined) {
            status = ast_write_tag(rt,ast_counted,3);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Section_get_size(rt,data_v->section.value);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Section_write(rt,data_v->section.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(data_v->bigend.defined) {
            status = ast_write_primitive(rt,ast_bool,4,&data_v->bigend.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(data_v->version.defined) {
            status = ast_write_primitive(rt,ast_uint32,5,&data_v->version.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(data_v->compress.defined) {
            status = ast_write_primitive(rt,ast_enum,6,&data_v->compress.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(data_v->crc32.defined) {
            status = ast_write_primitive(rt,ast_fixed32,7,&data_v->crc32.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Data_write*/

ast_err
Data_read(ast_runtime* rt, Data** data_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Data* data_v;
    unsigned long pos;

    data_v = (Data*)ast_alloc(rt,sizeof(Data));
    if(data_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Data|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&data_v->varName);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_enum,2,&data_v->dataType);
            } break;
        case 3: {
            size_t size;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            data_v->section.defined = 1;
            data_v->section.value = NULL;
            status = Section_read(rt,&data_v->section.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        case 4: {
            data_v->bigend.defined = 1;
            data_v->bigend.value = 0;
            status = ast_read_primitive(rt,ast_bool,4,&data_v->bigend.value);
            } break;
        case 5: {
            data_v->version.defined = 1;
            data_v->version.value = 0;
            status = ast_read_primitive(rt,ast_uint32,5,&data_v->version.value);
            } break;
        case 6: {
            data_v->compress.defined = 1;
            data_v->compress.value = 0;
            status = ast_read_primitive(rt,ast_enum,6,&data_v->compress.value);
            } break;
        case 7: {
            data_v->crc32.defined = 1;
            data_v->crc32.value = 0;
            status = ast_read_primitive(rt,ast_fixed32,7,&data_v->crc32.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!data_v->bigend.defined) {
        data_v->bigend.defined = 1;
        data_v->bigend.value = 1;
    }
    if(!data_v->version.defined) {
        data_v->version.defined = 1;
        data_v->version.value = 0;
    }
    if(!data_v->crc32.defined) {
        data_v->crc32.defined = 1;
        data_v->crc32.value = 0;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(data_vp) *data_vp = data_v;
done:
    return ACATCH(status);
} /*Data_read*/

ast_err
Data_reclaim(ast_runtime* rt, Data* data_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,data_v->varName);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(data_v->section.defined) {
            status = Section_reclaim(rt,data_v->section.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    ast_free(rt,(void*)data_v);
    goto done;

done:
    return ACATCH(status);

} /*Data_reclaim*/

size_t
Data_get_size(ast_runtime* rt, Data* data_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&data_v->varName);
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_enum,&data_v->dataType);
        totalsize += fieldsize;
    }
    {
        if(data_v->section.defined) {
            fieldsize += Section_get_size(rt,data_v->section.value);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
        }
        totalsize += fieldsize;
    }
    {
        if(data_v->bigend.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_bool,&data_v->bigend.value);
        }
        totalsize += fieldsize;
    }
    {
        if(data_v->version.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint32,&data_v->version.value);
        }
        totalsize += fieldsize;
    }
    {
        if(data_v->compress.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,6);
            fieldsize += ast_get_size(rt,ast_enum,&data_v->compress.value);
        }
        totalsize += fieldsize;
    }
    {
        if(data_v->crc32.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,7);
            fieldsize += ast_get_size(rt,ast_fixed32,&data_v->crc32.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Data_get_size*/

ast_err
Range_write(ast_runtime* rt, Range* range_v)
{
    ast_err status = AST_NOERR;

    {
        if(range_v->start.defined) {
            status = ast_write_primitive(rt,ast_uint64,1,&range_v->start.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_uint64,2,&range_v->size);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        if(range_v->stride.defined) {
            status = ast_write_primitive(rt,ast_uint64,3,&range_v->stride.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Range_write*/

ast_err
Range_read(ast_runtime* rt, Range** range_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Range* range_v;
    unsigned long pos;

    range_v = (Range*)ast_alloc(rt,sizeof(Range));
    if(range_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Range|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            range_v->start.defined = 1;
            range_v->start.value = 0;
            status = ast_read_primitive(rt,ast_uint64,1,&range_v->start.value);
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_uint64,2,&range_v->size);
            } break;
        case 3: {
            range_v->stride.defined = 1;
            range_v->stride.value = 0;
            status = ast_read_primitive(rt,ast_uint64,3,&range_v->stride.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!range_v->start.defined) {
        range_v->start.defined = 1;
        range_v->start.value = 0;
    }
    if(!range_v->stride.defined) {
        range_v->stride.defined = 1;
        range_v->stride.value = 1;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(range_vp) *range_vp = range_v;
done:
    return ACATCH(status);
} /*Range_read*/

ast_err
Range_reclaim(ast_runtime* rt, Range* range_v)
{
    ast_err status = AST_NOERR;

    ast_free(rt,(void*)range_v);
    goto done;

done:
    return ACATCH(status);

} /*Range_reclaim*/

size_t
Range_get_size(ast_runtime* rt, Range* range_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        if(range_v->start.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_uint64,&range_v->start.value);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_uint64,&range_v->size);
        totalsize += fieldsize;
    }
    {
        if(range_v->stride.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint64,&range_v->stride.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Range_get_size*/

ast_err
Section_write(ast_runtime* rt, Section* section_v)
{
    ast_err status = AST_NOERR;

    {
        size_t size;
        int i;
        for(i=0;i<section_v->range.count;i++) {
            status = ast_write_tag(rt,ast_counted,1);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            size = Range_get_size(rt,section_v->range.values[i]);
            status = ast_write_size(rt,size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = Range_write(rt,section_v->range.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*Section_write*/

ast_err
Section_read(ast_runtime* rt, Section** section_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Section* section_v;
    unsigned long pos;

    section_v = (Section*)ast_alloc(rt,sizeof(Section));
    if(section_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Section|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            size_t size;
            Range* tmp;
            if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}
            status = ast_read_size(rt,&size);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_mark(rt,size);
            status = Range_read(rt,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_message,&section_v->range,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            ast_unmark(rt);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(section_vp) *section_vp = section_v;
done:
    return ACATCH(status);
} /*Section_read*/

ast_err
Section_reclaim(ast_runtime* rt, Section* section_v)
{
    ast_err status = AST_NOERR;

    {
        int i;
        for(i=0;i<section_v->range.count;i++) {
            status = Range_reclaim(rt,section_v->range.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,section_v->range.values);
    }
    ast_free(rt,(void*)section_v);
    goto done;

done:
    return ACATCH(status);

} /*Section_reclaim*/

size_t
Section_get_size(ast_runtime* rt, Section* section_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<section_v->range.count;i++) {
            fieldsize += Range_get_size(rt,section_v->range.values[i]);
            fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*Section_get_size*/

ast_err
StructureData_write(ast_runtime* rt, StructureData* structuredata_v)
{
    ast_err status = AST_NOERR;

    {
        int i = 0;
        for(i=0;i<structuredata_v->member.count;i++) {
            status = ast_write_primitive(rt,ast_uint32,1,&structuredata_v->member.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        status = ast_write_primitive(rt,ast_bytes,2,&structuredata_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i = 0;
        for(i=0;i<structuredata_v->heapCount.count;i++) {
            status = ast_write_primitive(rt,ast_uint32,3,&structuredata_v->heapCount.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        int i = 0;
        for(i=0;i<structuredata_v->sdata.count;i++) {
            status = ast_write_primitive(rt,ast_string,4,&structuredata_v->sdata.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }
    {
        if(structuredata_v->nrows.defined) {
            status = ast_write_primitive(rt,ast_uint64,5,&structuredata_v->nrows.value);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
    }

done:
    return ACATCH(status);

} /*StructureData_write*/

ast_err
StructureData_read(ast_runtime* rt, StructureData** structuredata_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    StructureData* structuredata_v;
    unsigned long pos;

    structuredata_v = (StructureData*)ast_alloc(rt,sizeof(StructureData));
    if(structuredata_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|StructureData|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            uint32_t tmp;
            status = ast_read_primitive(rt,ast_uint32,1,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint32,&structuredata_v->member,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 2: {
            status = ast_read_primitive(rt,ast_bytes,2,&structuredata_v->data);
            } break;
        case 3: {
            uint32_t tmp;
            status = ast_read_primitive(rt,ast_uint32,3,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_uint32,&structuredata_v->heapCount,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 4: {
            char* tmp;
            status = ast_read_primitive(rt,ast_string,4,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            status = ast_repeat_append(rt,ast_string,&structuredata_v->sdata,&tmp);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
            } break;
        case 5: {
            structuredata_v->nrows.defined = 1;
            structuredata_v->nrows.value = 0;
            status = ast_read_primitive(rt,ast_uint64,5,&structuredata_v->nrows.value);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(!structuredata_v->nrows.defined) {
        structuredata_v->nrows.defined = 1;
        structuredata_v->nrows.value = 1;
    }
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(structuredata_vp) *structuredata_vp = structuredata_v;
done:
    return ACATCH(status);
} /*StructureData_read*/

ast_err
StructureData_reclaim(ast_runtime* rt, StructureData* structuredata_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_bytes(rt,&structuredata_v->data);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    {
        int i;
        for(i=0;i<structuredata_v->sdata.count;i++) {
            status = ast_reclaim_string(rt,structuredata_v->sdata.values[i]);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }
        ast_free(rt,structuredata_v->sdata.values);
    }
    ast_free(rt,(void*)structuredata_v);
    goto done;

done:
    return ACATCH(status);

} /*StructureData_reclaim*/

size_t
StructureData_get_size(ast_runtime* rt, StructureData* structuredata_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        int i;
        for(i=0;i<structuredata_v->member.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,1);
            fieldsize += ast_get_size(rt,ast_uint32,&structuredata_v->member.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        fieldsize += ast_get_tagsize(rt,ast_counted,2);
        fieldsize += ast_get_size(rt,ast_bytes,&structuredata_v->data);
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structuredata_v->heapCount.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,3);
            fieldsize += ast_get_size(rt,ast_uint32,&structuredata_v->heapCount.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        int i;
        for(i=0;i<structuredata_v->sdata.count;i++) {
            fieldsize += ast_get_tagsize(rt,ast_counted,4);
            fieldsize += ast_get_size(rt,ast_string,&structuredata_v->sdata.values[i]);
        }
        totalsize += fieldsize;
    }
    {
        if(structuredata_v->nrows.defined) {
            fieldsize += ast_get_tagsize(rt,ast_counted,5);
            fieldsize += ast_get_size(rt,ast_uint64,&structuredata_v->nrows.value);
        }
        totalsize += fieldsize;
    }
    return totalsize;

} /*StructureData_get_size*/

ast_err
Error_write(ast_runtime* rt, Error* error_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_write_primitive(rt,ast_string,1,&error_v->message);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }

done:
    return ACATCH(status);

} /*Error_write*/

ast_err
Error_read(ast_runtime* rt, Error** error_vp)
{
    ast_err status = AST_NOERR;
    uint32_t wiretype, fieldno;
    Error* error_v;
    unsigned long pos;

    error_v = (Error*)ast_alloc(rt,sizeof(Error));
    if(error_v == NULL) return AST_ENOMEM;

    while(status == AST_NOERR) {
        pos = (unsigned long)xpos(rt);
        status = ast_read_tag(rt,&wiretype,&fieldno);
        if(status == AST_EOF) {status = AST_NOERR; break;}
        if(status != AST_NOERR) break;
        {
        fprintf(stderr,"|Error|: before=%lu fieldno=%lu wiretype=%lu after=%lu\n",pos,(unsigned long)fieldno,(unsigned long)wiretype,(unsigned long)xpos(rt));
        }
        switch (fieldno) {
        case 1: {
            status = ast_read_primitive(rt,ast_string,1,&error_v->message);
            } break;
        default:
            status = ast_skip_field(rt,wiretype,fieldno);
            if(status != AST_NOERR) {ACATCH(status); goto done;}
        }; /*switch*/
    };/*while*/
    if(status != AST_NOERR) {ACATCH(status); goto done;}
    if(error_vp) *error_vp = error_v;
done:
    return ACATCH(status);
} /*Error_read*/

ast_err
Error_reclaim(ast_runtime* rt, Error* error_v)
{
    ast_err status = AST_NOERR;

    {
        status = ast_reclaim_string(rt,error_v->message);
        if(status != AST_NOERR) {ACATCH(status); goto done;}
    }
    ast_free(rt,(void*)error_v);
    goto done;

done:
    return ACATCH(status);

} /*Error_reclaim*/

size_t
Error_get_size(ast_runtime* rt, Error* error_v)
{
    size_t totalsize = 0;
    size_t fieldsize = 0;

    {
        fieldsize += ast_get_tagsize(rt,ast_counted,1);
        fieldsize += ast_get_size(rt,ast_string,&error_v->message);
        totalsize += fieldsize;
    }
    return totalsize;

} /*Error_get_size*/

