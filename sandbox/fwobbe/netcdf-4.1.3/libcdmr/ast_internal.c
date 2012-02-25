/* --- protobuf-c.c: public protobuf c runtime implementation --- */
/*
 * Copyright 2008, Dave Benson.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with
 * the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless
 * required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * Modified 1/20/2011 to support Unidata AST compiler.
 * Author: Dennis Heimbigner (dennis.heimbigner@ieee.org).
 */

/* TODO items:
     * 64-BIT OPTIMIZATION: certain implementations use 32-bit math even on 64-bit platforms
        (uint64_size, uint64_pack, parse_uint64)
     * get_packed_size and pack seem to use type-prefixed names,
       whereas parse uses type-suffixed names.  pick one and stick with it.
       Decision:  go with type-suffixed, since the type (or its instance)
       is typically the object of the verb.
       NOTE: perhaps the "parse" methods should be reanemd to "unpack"
       at the same time.(this only affects internal(static) functions)
     * use TRUE and FALSE instead of 1 and 0 as appropriate
     * use size_t consistently
 */
#include <stdio.h>                      /* for occasional printf()s */
#include <stdlib.h>                     /* for abort(), malloc() etc */
#include <string.h>                     /* for strlen(), memcpy(), memmove() */
#include <endian.h>

#include "config.h"
#include <ast_runtime.h>
#include <ast_internal.h>

/* Must be lub(64/7) = 10 */
#define MAX_UINT64_ENCODED_SIZE 10

/* Create a uniqueid from an 8-char id string */
uint64_t
ast_create_unique_id(const char suid[8])
{
    union ast_union_uid {uint64_t uid; char uidstring[8];} uuid;
    memcpy(uuid.uidstring,suid,8);
    return uuid.uid;        
}

/* === get_packed_size() === */
/* Return the number of bytes required to store the
   tag for the field(which includes 3 bits for
   the wire-type, and a single bit that denotes the end-of-tag. */


/* Pack an unsigned 32-bit integer in base-128 encoding, and return the number of bytes needed:
   this will be 5 or less. */
size_t
uint32_encode(uint32_t value, uint8_t *out)
{
  unsigned rv = 0;
  if(value >= 0x80)
    {
      out[rv++] = value | 0x80;
      value >>= 7;
      if(value >= 0x80)
        {
          out[rv++] = value | 0x80;
          value >>= 7;
          if(value >= 0x80)
            {
              out[rv++] = value | 0x80;
              value >>= 7;
              if(value >= 0x80)
                {
                  out[rv++] = value | 0x80;
                  value >>= 7;
                }
            }
        }
    }
  /* assert: value<128 */
  out[rv++] = value;
  return rv;
}

/* Pack a 32-bit signed integer, returning the number of bytes needed.
   Negative numbers are packed as twos-complement 64-bit integers. */

size_t
int32_encode(int32_t value, uint8_t *out)
{
  if(value < 0)
    {
      out[0] = value | 0x80;
      out[1] =(value>>7) | 0x80;
      out[2] =(value>>14) | 0x80;
      out[3] =(value>>21) | 0x80;
      out[4] =(value>>28) | 0x80;
      out[5] = out[6] = out[7] = out[8] = 0xff;
      out[9] = 0x01;
      return 10;
    }
  else
    return uint32_encode(value, out);
}

/* Pack a 32-bit integer in zigwag encoding. */
size_t
sint32_encode(int32_t value, uint8_t *out)
{
  return uint32_encode(zigzag32(value), out);
}

/* Pack a 64-bit unsigned integer that fits in a 64-bit uint,
   using base-128 encoding. */

size_t
int64_encode(int64_t value, uint8_t *out)
{
    return (int64_t)uint64_encode((uint64_t)value,out);
}

size_t
uint64_encode(uint64_t value, uint8_t *out)
{
  uint32_t hi = value>>32;
  uint32_t lo = value;
  unsigned rv;
  if(hi == 0)
    return uint32_encode((uint32_t)lo, out);
  out[0] =(lo) | 0x80;
  out[1] =(lo>>7) | 0x80;
  out[2] =(lo>>14) | 0x80;
  out[3] =(lo>>21) | 0x80;
  if(hi < 8)
    {
      out[4] =(hi<<4) |(lo>>28);
      return 5;
    }
  else
    {
      out[4] =((hi&7)<<4) |(lo>>28) | 0x80;
      hi >>= 3;
    }
  rv = 5;
  while(hi >= 128)
    {
      out[rv++] = hi | 0x80;
      hi >>= 7;
    }
  out[rv++] = hi;
  return rv;
}
/* Pack a 64-bit signed integer in zigzan encoding,
   return the size of the packed output.
  (Max returned value is 10) */

size_t
sint64_encode(int64_t value, uint8_t *out)
{
  return uint64_encode(zigzag64(value), out);
}

/* Pack a 32-bit value, little-endian.
   Used for fixed32, sfixed32, float) */

size_t
fixed32_encode(uint32_t value, uint8_t *out)
{
#ifdef LITTLE_ENDIAN
  memcpy(out, &value, 4);
#else
  out[0] = value;
  out[1] = value>>8;
  out[2] = value>>16;
  out[3] = value>>24;
#endif
  return 4;
}
/* Pack a 64-bit fixed-length value.
  (Used for fixed64, sfixed64, double) */
/* XXX: the big-endian impl is really only good for 32-bit machines,
   a 64-bit version would be appreciated, plus a way
   to decide to use 64-bit math where convenient. */

size_t
fixed64_encode(uint64_t value, uint8_t *out)
{
#ifdef LITTLE_ENDIAN
  memcpy(out, &value, 8);
#else
  fixed32_encode(value, out);
  fixed32_encode(value>>32, out+4);
#endif
  return 8;
}

/* Pack a boolean as 0 or 1, even though the bool_t
   can really assume any integer value. */
/* XXX: perhaps on some platforms "*out = !!value" would be
   a better impl, b/c that is idiotmatic c++ in some stl impls. */

size_t
boolean_encode(bool_t value, uint8_t *out)
{
  *out = value ? 1 : 0;
  return 1;
}

/* Decode a 32 bit varint */
uint32_t
uint32_decode(const size_t len0, const uint8_t* data)
{
  uint32_t rv;
  size_t len = len0;

  if(len > 5) len = 5;
  rv = data[0] & 0x7f;
  if(len > 1) {
      rv |=((data[1] & 0x7f) << 7);
      if(len > 2) {
          rv |=((data[2] & 0x7f) << 14);
          if(len > 3) {
              rv |=((data[3] & 0x7f) << 21);
              if(len > 4)
                rv |=(data[4] << 28);
            }
        }
  }
  return rv;
}

int32_t
int32_decode(const size_t len, const uint8_t* data)
{
  return (int32_t)uint32_decode(len,data);
}

/* Decode possibly 64-bit varint*/
uint64_t
uint64_decode(const size_t len, const uint8_t* data)
{
  unsigned shift, i;
  uint64_t rv;

  if(len < 5) {
    rv = uint32_decode(len, data);
  } else {
    rv =((data[0] & 0x7f))
              |((data[1] & 0x7f)<<7)
              |((data[2] & 0x7f)<<14)
              |((data[3] & 0x7f)<<21);
    shift = 28;
    for(i = 4; i < len; i++) {
      rv |=(((uint64_t)(data[i]&0x7f)) << shift);
      shift += 7;
    }
  }
  return rv;
}

int64_t
int64_decode(const size_t len, const uint8_t* data)
{
  return (int64_t)uint64_decode(len, data);
}

/* Decode arbitrary varint upto 64bit */
uint64_t
varint_decode(const size_t buflen, const uint8_t* buffer, size_t* countp)
{
  unsigned shift, i;
  uint64_t rv = 0;
  size_t count = 0;

  if(buflen == 0 || buffer == NULL) {goto done;}

  for(count=0,shift=0,i=0;i<buflen;i++,shift+=7) {
    uint8_t byte = buffer[i];
    count++;
    rv |= ((byte & 0x7f) << shift);
    if((byte & 0x80)==0) break;
  }

done:
  if(countp) *countp = count;    
  return rv;
}

uint32_t
fixed32_decode(const uint8_t* data)
{
  uint32_t rv;
#ifdef LITTLE_ENDIAN
   memcpy(&rv,data,4);
#else
  rv = (data[0] |(data[1] << 8) |(data[2] << 16) |(data[3] << 24));
#endif
  return rv;
}

uint64_t
fixed64_decode(const uint8_t* data)
{
  uint64_t rv;

#ifdef LITTLE_ENDIAN
  memcpy(&rv,data,8);
#else
  rv = fixed32_decode(data);
  rv2 = fixed32_decode(data+4);
  rv = (rv | (rv2 <<32));
#endif
  return rv;
}

bool_t
boolean_decode(const size_t len, const uint8_t* data)
{
  int i;
  bool_t tf;

  tf = 0;
  for(i = 0; i < len; i++) {
    if(data[i] & 0x7f) tf = 1;
  }
  return tf;
}

/* return the zigzag-encoded 32-bit unsigned int from a 32-bit signed int */

uint32_t
zigzag32(int32_t v)
{
  if(v < 0)
    return((uint32_t)(-v)) * 2 - 1;
  else
    return v * 2;
}
/* return the zigzag-encoded 64-bit unsigned int from a 64-bit signed int */

uint64_t
zigzag64(int64_t v)
{
  if(v < 0)
    return((uint64_t)(-v)) * 2 - 1;
  else
    return v * 2;
}

int32_t
unzigzag32(uint32_t v)
{
  if(v&1)
    return -(v>>1) - 1;
  else
    return v>>1;
}

int64_t
unzigzag64(uint64_t v)
{
  if(v&1)
    return -(v>>1) - 1;
  else
    return v>>1;
}


size_t
get_tag_size(unsigned number)
{
  if(number <(1<<4))
    return 1;
  else if(number <(1<<11))
    return 2;
  else if(number <(1<<18))
    return 3;
  else if(number <(1<<25))
    return 4;
  else
    return 5;
}

/* Return the number of bytes required to store
   a variable-length unsigned integer that fits in 32-bit uint
   in base-128 encoding. */

size_t
uint32_size(uint32_t v)
{
  if(v <(1<<7))
    return 1;
  else if(v <(1<<14))
    return 2;
  else if(v <(1<<21))
    return 3;
  else if(v <(1<<28))
    return 4;
  else
    return 5;
}

/* Return the number of bytes required to store
   a variable-length signed integer that fits in 32-bit int
   in base-128 encoding. */
size_t
int32_size(int32_t v)
{
  if(v < 0)
    return 10;
  else if(v <(1<<7))
    return 1;
  else if(v <(1<<14))
    return 2;
  else if(v <(1<<21))
    return 3;
  else if(v <(1<<28))
    return 4;
  else
    return 5;
}

/* Return the number of bytes required to store
   a variable-length unsigned integer that fits in 64-bit uint
   in base-128 encoding. */
size_t
uint64_size(uint64_t v)
{
  uint32_t upper_v =(v>>32);
  if(upper_v == 0)
    return uint32_size((uint32_t)v);
  else if(upper_v <(1<<3))
    return 5;
  else if(upper_v <(1<<10))
    return 6;
  else if(upper_v <(1<<17))
    return 7;
  else if(upper_v <(1<<24))
    return 8;
  else if(upper_v <(1U<<31))
    return 9;
  else
    return 10;
}

/* Return the number of bytes required to store
   a variable-length unsigned integer that fits in 64-bit int
   in base-128 encoding. */
size_t
int64_size(int64_t v)
{
  uint32_t upper_v =(v>>32);
  if(upper_v == 0)
    return int32_size((int32_t)v);
  else if(upper_v <(1<<3))
    return 5;
  else if(upper_v <(1<<10))
    return 6;
  else if(upper_v <(1<<17))
    return 7;
  else if(upper_v <(1<<24))
    return 8;
  else if(upper_v <(1U<<31))
    return 9;
  else
    return 10;
}

/* Return the number of bytes required to store
   a variable-length signed integer that fits in 32-bit int,
   converted to unsigned via the zig-zag algorithm,
   then packed using base-128 encoding. */

size_t
sint32_size(int32_t v)
{
  return uint32_size(zigzag32(v));
}


/* Return the number of bytes required to store
   a variable-length signed integer that fits in 64-bit int,
   converted to unsigned via the zig-zag algorithm,
   then packed using base-128 encoding. */
size_t
sint64_size(int64_t v)
{
  return uint64_size(zigzag64(v));
}

#ifdef IGNORE

/* Pack a length-prefixed string.
   The input string is NUL-terminated.
   The NULL pointer is treated as an empty string.
   This isn't really necessary, but it allows people
   to leave required strings blank.
  (See Issue 13 in the bug tracker for a
   little more explanation).
 */

size_t
string_encode(const char * str, uint8_t *out)
{
  if(str == NULL)
    {
      out[0] = 0;
      return 1;
    }
  else
    {
      size_t len = strlen(str);
      size_t rv = uint32_encode(len, out);
      memcpy(out + rv, str, len);
      return rv + len;
    }
}

size_t
binary_data_encode(const ProtobufCBinaryData *bd, uint8_t *out)
{
  size_t len = bd->len;
  size_t rv = uint32_encode(len, out);
  memcpy(out + rv, bd->data, len);
  return rv + len;
}

/* wire-type will be added in required_field_encode() */
/* XXX: just call uint64_pack on 64-bit platforms. */

size_t
tag_encode(uint32_t id, uint8_t *out)
{
  if(id <(1<<(32-3)))
    return uint32_encode(id<<3, out);
  else
    return uint64_encode(((uint64_t)id) << 3, out);
}

/* Get the packed size of a unknown field(meaning one that
   is passed through mostly uninterpreted... this is done
   for forward compatibilty with the addition of new fields). */

size_t
unknown_field_get_packed_size(const ProtobufCMessageUnknownField *field)
{
  return get_tag_size(field->tag) + field->len;
}



/* === pack() === */
/* TODO: implement as a table lookup */

size_t
sizeof_elt_in_repeated_array(ProtobufCType type)
{
  switch(type)
    {
    case PROTOBUF_C_TYPE_SINT32:
    case PROTOBUF_C_TYPE_INT32:
    case PROTOBUF_C_TYPE_UINT32:
    case PROTOBUF_C_TYPE_SFIXED32:
    case PROTOBUF_C_TYPE_FIXED32:
    case PROTOBUF_C_TYPE_FLOAT:
    case PROTOBUF_C_TYPE_ENUM:
      return 4;
    case PROTOBUF_C_TYPE_SINT64:
    case PROTOBUF_C_TYPE_INT64:
    case PROTOBUF_C_TYPE_UINT64:
    case PROTOBUF_C_TYPE_SFIXED64:
    case PROTOBUF_C_TYPE_FIXED64:
    case PROTOBUF_C_TYPE_DOUBLE:
      return 8;
    case PROTOBUF_C_TYPE_BOOL:
      return sizeof(bool_t);
    case PROTOBUF_C_TYPE_STRING:
    case PROTOBUF_C_TYPE_MESSAGE:
      return sizeof(void *);
    case PROTOBUF_C_TYPE_BYTES:
      return sizeof(ProtobufCBinaryData);
    }
  PROTOBUF_C_ASSERT_NOT_REACHED();
  return 0;
}

void
copy_to_little_endian_32(void *out, const void *in, unsigned N)
{
#ifdef LITTLE_ENDIAN
  memcpy(out, in, N * 4);
#else
  unsigned i;
  const uint32_t *ini = in;
  for(i = 0; i < N; i++)
    fixed32_encode(ini[i],(uint32_t*)out + i);
#endif
}

void
copy_to_little_endian_64(void *out, const void *in, unsigned N)
{
#ifdef LITTLE_ENDIAN
  memcpy(out, in, N * 8);
#else
  unsigned i;
  const uint64_t *ini = in;
  for(i = 0; i < N; i++)
    fixed64_encode(ini[i],(uint64_t*)out + i);
#endif
}

unsigned
get_type_min_size(ProtobufCType type)
{
  if(type == PROTOBUF_C_TYPE_SFIXED32
   || type == PROTOBUF_C_TYPE_FIXED32
   || type == PROTOBUF_C_TYPE_FLOAT)
    return 4;
  if(type == PROTOBUF_C_TYPE_SFIXED64
   || type == PROTOBUF_C_TYPE_FIXED64
   || type == PROTOBUF_C_TYPE_DOUBLE)
    return 8;
  return 1;
}


/* === unpacking === */
size_t
parse_tag_and_wiretype(size_t len,
                        const uint8_t *data,
                        uint32_t *tag_out,
                        ProtobufCWireType *wiretype_out)
{
  unsigned max_rv = len > 5 ? 5 : len;
  uint32_t tag =(data[0]&0x7f) >> 3;
  unsigned shift = 4;
  unsigned rv;
  *wiretype_out = data[0] & 7;
  if((data[0] & 0x80) == 0)
    {
      *tag_out = tag;
      return 1;
    }
  for(rv = 1; rv < max_rv; rv++)
    if(data[rv] & 0x80)
      {
        tag |=(data[rv] & 0x7f) << shift;
        shift += 7;
      }
    else
      {
        tag |= data[rv] << shift;
        *tag_out = tag;
        return rv + 1;
      }
  return 0;                   /* error: bad header */
}

uint32_t
scan_length_prefixed_data(size_t len, const uint8_t *data, size_t *prefix_len_out)
{
  unsigned hdr_max = len < 5 ? len : 5;
  unsigned hdr_len;
  uint32_t val = 0;
  unsigned i;
  unsigned shift = 0;
  for(i = 0; i < hdr_max; i++)
    {
      val |=(data[i] & 0x7f) << shift;
      shift += 7;
      if((data[i] & 0x80) == 0)
        break;
    }
  if(i == hdr_max)
    {
      UNPACK_ERROR(("error parsing length for length-prefixed data"));
      return 0;
    }
  hdr_len = i + 1;
  *prefix_len_out = hdr_len;
  if(hdr_len + val > len)
    {
      UNPACK_ERROR(("data too short after length-prefix of %u",
                     val));
      return 0;
    }
  return hdr_len + val;
}

size_t
max_b128_numbers(size_t len, const uint8_t *data)
{
  size_t rv = 0;
  while(len--)
    if((*data++ & 0x80) == 0)
      ++rv;
  return rv;
}

unsigned
scan_varint(unsigned len, const uint8_t *data)
{
  unsigned i;
  if(len > 10)
    len = 10;
  for(i = 0; i < len; i++)
    if((data[i] & 0x80) == 0)
      break;
  if(i == len)
    return 0;
  return i + 1;
}

#endif /*IGNORE*/
