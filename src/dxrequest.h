#ifndef _dxrequest_H_
#define _dxrequest_H_

#include "jansson.h"

char* DXRequest_download(const char* file_name, size_t range_start, size_t range_end);
char* DXRequest_lookup_file(const char* file_name);


#endif
