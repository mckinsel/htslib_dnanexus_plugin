#include "dxcpp/bindings.h"
#include "dxjson/dxjson.h"

#include <cstdlib>
#include <iostream>
#include <string>

#include "dxc.h"

extern "C" {

DXFile* DXFile_create(const char* dxid, const char* proj)
{
  return reinterpret_cast<DXFile*>(new dx::DXFile(dxid, proj));
}

void DXFile_destroy(DXFile* dxfile)
{
  if(!dxfile) return;
  delete reinterpret_cast<dx::DXFile*>(dxfile);
}

void DXFile_read(DXFile* dxfile, char* ptr, int64_t n)
{
  reinterpret_cast<dx::DXFile*>(dxfile)->read(ptr, n);
}

int64_t DXFile_gcount(const DXFile* dxfile)
{
  return reinterpret_cast<const dx::DXFile*>(dxfile)->gcount();
}

int DXFile_eof(const DXFile* dxfile)
{
  bool eof = reinterpret_cast<const dx::DXFile*>(dxfile)->eof();
  if(eof) {
    return 0;
  } else {
    return 1;
  }
}

void DXFile_seek(DXFile* dxfile, int64_t pos)
{
  reinterpret_cast<dx::DXFile*>(dxfile)->seek(pos);
}

char* DXFile_resolve_filename(const char* filename, const char* proj)
{
  std::string project;
  if(proj != NULL) {
    project = std::string(proj);
  } else {
    project = dx::config::CURRENT_PROJECT();
  }

  std::string json_string("{\"class\": \"file\", \"name\": \"");
  json_string += filename;
  json_string += "\", \"scope\": {\"project\": \"";
  json_string += project;
  json_string += "\"}}";
  
  dx::JSON json_query(dx::JSON_HASH);
  json_query.readFromString(json_string);

  dx::JSON json_result = dx::DXSystem::findDataObjects(json_query);

  if(json_result["results"].size() != 1) {
    return NULL;
  }
  
  std::string result = json_result["results"][0]["id"].toString();
  result.erase(0, 1);
  result.erase(result.size() - 1);
  char* output = (char*) malloc(result.size() + 1);
  strcpy(output, result.c_str());
  return output;
} 

} // extern C
