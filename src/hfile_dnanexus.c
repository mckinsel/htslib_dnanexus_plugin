#include "dxc.h"
#include "hfile_internal.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <curl/curl.h>

typedef struct {
  hFILE base;
  DXFile* dxfile;
} hFILE_dnanexus;

ssize_t dnanexus_read(hFILE *fpv, void *bufferv, size_t nbytes)
{
    hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;
    char* buffer = (char*) bufferv;
    DXFile_read(fp->dxfile, buffer, nbytes);
    return DXFile_gcount(fp->dxfile);
}

off_t dnanexus_seek(hFILE* fpv, off_t offset, int whence)
{
  hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;

  switch(whence) {
    case SEEK_SET:
      DXFile_seek(fp->dxfile, (int64_t) offset);
      break;
    case SEEK_CUR:
      errno = ENOSYS;
      return -1;
    case SEEK_END:
      errno = ENOSYS;
      return -1;
    default:
      errno = EINVAL;
      return -1;
  }
  return offset;
}

int dnanexus_close(hFILE* fpv)
{
  hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;
  DXFile_destroy(fp->dxfile);
  return 0;
}

static const struct hFILE_backend dnanexus_backend =
{
  dnanexus_read, NULL, dnanexus_seek, NULL, dnanexus_close
};


hFILE* hopen_dnanexus(const char* file_name, const char* modes)
{
  hFILE_dnanexus* fp;
  fp = (hFILE_dnanexus*) hfile_init(sizeof (hFILE_dnanexus), modes, 0);
  if (fp == NULL) return NULL;
  
  /* Remove the "dx:" from the name of the file */ 
  char* stripped_file_name = malloc(strlen(file_name));
  strcpy(stripped_file_name, file_name + 3);

  char* file_id = DXFile_resolve_filename(stripped_file_name, NULL);
  fp->dxfile = DXFile_create(file_id, NULL);
  free(file_id);

  fp->base.backend = &dnanexus_backend;
  return &fp->base;
}

int hfile_plugin_init()
{
  static const struct hFILE_scheme_handler handler =
      { hopen_dnanexus, hfile_always_remote, "dnanexus", 50 };
    
  CURLcode err;
  err = curl_global_init(CURL_GLOBAL_ALL);
  if(err != CURLE_OK) {
    return -1;
  }

  hfile_add_scheme_handler("dx", &handler);

  return 0;
}
