#ifndef NCCRDUMP_H
#define NCCRDUMP_H 1

extern ast_err nccr_dumpheader(Header*);

extern ast_err nccr_data_dump(Data*, Variable*, int bigendian, bytes_t* data);

#endif /*NCCRDUMP_H*/
