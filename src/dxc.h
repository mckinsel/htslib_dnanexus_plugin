#ifndef DXC_H
#define DXC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DXFile
 */

typedef struct DXFile DXFile;

DXFile* DXFile_create(const char* dxid, const char* proj);
void    DXFile_destroy(DXFile* dxfile);
void    DXFile_read(DXFile* dxfile, char* ptr, int64_t n);
int64_t DXFile_gcount(const DXFile* dxfile); 
int     DXFile_eof(const DXFile* dxfile);
void    DXFile_seek(DXFile* dxfile, int64_t pos);
int64_t DXFile_size(const DXFile* dxfile);

char*   DXFile_resolve_filename(const char* filename, const char* proj);

#ifdef __cplusplus
}
#endif

#endif
