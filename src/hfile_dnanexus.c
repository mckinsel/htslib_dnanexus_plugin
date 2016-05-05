#include "dxrequest.h"

#include <curl/curl.h>

#include "hfile_internal.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  hFILE base;
  char* file_id;
  off_t pos;
} hFILE_dnanexus;

ssize_t dnanexus_read(hFILE *fpv, void *bufferv, size_t nbytes)
{
    fprintf(stderr, "Calling dnanexus read %zu\n", nbytes);
    hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;
    char* buffer = (char*) bufferv;
    char* read_bytes = DXRequest_download(fp->file_id, fp->pos, fp->pos + nbytes);
    fprintf(stderr, "Read bytes at %p\n", read_bytes);
    strncpy(buffer, read_bytes, nbytes);
    free(read_bytes);
    fp->pos += nbytes;
    return nbytes;
}

off_t dnanexus_seek(hFILE* fpv, off_t offset, int whence)
{
  hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;

  switch(whence) {
    case SEEK_SET:
      fp->pos = offset;
      break;
    case SEEK_CUR:
      fp->pos += offset;
      break;
    case SEEK_END:
      errno = ENOSYS;
      return -1;
    default:
      errno = EINVAL;
      return -1;
  }
  return fp->pos;
}

int dnanexus_close(hFILE* fpv)
{
  hFILE_dnanexus* fp = (hFILE_dnanexus*) fpv;
  fp->file_id = "";
  fp->pos = 0;
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
  
  char* stripped_file_name = malloc(strlen(file_name));
  strcpy(stripped_file_name, file_name + 3);
  /*fp->file_id = DXRequest_lookup_file(stripped_file_name);*/
  fp->file_id = stripped_file_name;
  fp->pos = 0;

  fp->base.backend = &dnanexus_backend;
  return &fp->base;

}

int always_remote(const char *fname) { (void) fname; return 1; }

int hfile_plugin_init()
{
  static const struct hFILE_scheme_handler handler =
      { hopen_dnanexus, always_remote, "dnanexus", 50 };
    
  CURLcode err;
  err = curl_global_init(CURL_GLOBAL_ALL);
  if(err != CURLE_OK) {
    return -1;
  }

  hfile_add_scheme_handler("dx", &handler);

  return 0;
}
