#ifndef AST_INTERNAL_H
#define AST_INTERNAL_H

#include <ast_debug.h>

typedef struct ast_optional_field {
    int exists;
    void* value;
} ast_optional_field;

typedef struct ast_repeated_field {
    size_t count;
    void* values;
} ast_repeated_field;

/* Procedures from ast_internal.c */

extern uint64_t ast_create_unique_id(const char suid[8]);

extern uint32_t zigzag32(int32_t v);
extern uint64_t zigzag64(int64_t v);
extern int32_t unzigzag32(uint32_t v);
extern int64_t unzigzag64(uint64_t v);

extern void copy_to_little_endian_32(void *out, const void *in, unsigned N);
extern void copy_to_little_endian_64(void *out, const void *in, unsigned N);

/* The term "pack" here is misleading; it really mean encode into protobuf format */
extern size_t uint32_encode(uint32_t value, uint8_t *out);
extern size_t uint64_encode(uint64_t value, uint8_t *out);
extern size_t int32_encode(int32_t value, uint8_t *out);
extern size_t int64_encode(int64_t value, uint8_t *out);

extern size_t sint32_encode(int32_t value, uint8_t *out);
extern size_t sint64_encode(int64_t value, uint8_t *out);
extern size_t fixed32_encode(uint32_t value, uint8_t *out);
extern size_t fixed64_encode(uint64_t value, uint8_t *out);
extern size_t boolean_encode(bool_t value, uint8_t* out);

/* Varint decodings; when in doubt use varint_decode */
extern uint64_t varint_decode(const size_t, const uint8_t*, size_t*);
extern uint32_t uint32_decode(const size_t len, const uint8_t*);
extern uint64_t uint64_decode(const size_t len, const uint8_t*);
extern int32_t int32_decode(const size_t len, const uint8_t*);
extern int64_t int64_decode(const size_t len, const uint8_t* data);
/* Fixed size decodes; may need subsequent unzigzag */
extern uint32_t fixed32_decode(const uint8_t*);
extern uint64_t fixed64_decode(const uint8_t*);
extern bool_t boolean_decode(const size_t len, const uint8_t*);

extern size_t uint32_size(uint32_t v);
extern size_t int32_size(int32_t v);
extern size_t uint64_size(uint64_t v);
extern size_t int64_size(int64_t v);
extern size_t sint32_size(int32_t v);
extern size_t sint64_size(int64_t v);

#ifdef IGNORE /* For now. */
extern bool_t count_encodeed_elements(ast_sort type, size_t len, const uint8_t *data, size_t *count_out);
extern unsigned get_type_min_size(ast_sort type);
extern size_t sizeof_elt_in_repeated_array(ast_sort, type);
extern size_t parse_tag_and_wiretype(size_t len, const uint8_t *data, uint32_t *tag_out, ast_sort* wiretype_out);
extern size_t binary_data_encode(const byte_t* bd, uint8_t *out);
extern unsigned scan_varint(unsigned len, const uint8_t *data);
extern uint32_t scan_length_prefixed_data(size_t len, const uint8_t *data, size_t *prefix_len_out);
extern size_t max_b128_numbers(size_t len, const uint8_t *data);
extern size_t get_tag_size(unsigned number);
extern size_t unknown_field_get_encodeed_size(const ProtobufCMessageUnknownField *field);
extern size_t string_encode(const char * str, uint8_t *out);
extern size_t tag_encode(uint32_t id, uint8_t *out);
#endif /*IGNORE*/

#endif /*AST_INTERNAL_H*/
