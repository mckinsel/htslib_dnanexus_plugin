#include "dxrequest.h"

#include "dxconfig.h"
#include "dbg.h"

#include "jansson.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

/* Struct to hold CURL responses */
struct Payload {
  char* data;
  size_t size;
};

/* Callback function CURL uses when receiving data */
size_t curl_callback(char* contents, size_t size, size_t nmemb, void* userdata)
{
  size_t realsize = size * nmemb;
  struct Payload* payload = (struct Payload*) userdata;

  debug("Callback got size %zu, nmemb %zu, contents at %p, and userdata at %p",
        size, nmemb, contents, userdata);

  payload->data = (char*) realloc(payload->data, payload->size + realsize + 1);
  check(payload->data,
        "Could not allocate enough memory for the server's response.")

  debug("Payload data is at %p, and size is %zu", payload->data, payload->size);

  memcpy(&(payload->data[payload->size]), contents, realsize);
  payload->size += realsize;
  payload->data[payload->size] = '\0';
  
  if(payload->size <= 2048) {
    debug("Payload data %s", payload->data);
  }

  return realsize;

error:
  return 0;
}

/* Create a new CURL handle that will access the given URL and write the
 * response to the given Payload */
CURL* create_curl_handle(const char* url, struct Payload* payload)
{
  CURL* curl = curl_easy_init();
  curl_version_info_data *info;
  info = curl_version_info(CURLVERSION_NOW);
  char useragent[1024];
  sprintf(useragent, "htslib libcurl/%s hFILE_dnanexus",
          info->version);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) payload);
  return curl;
}

/* Create a new CURL handle that will POST to the URL with the give post data
 * and then write the response to the payload. */
CURL* create_post_curl_handle(
    const char* url, const json_t* post_fields, struct Payload* payload)
{
  CURL* curl = create_curl_handle(url, payload);

  char* port = DXConfig_get_apiserver_port();
  curl_easy_setopt(curl, CURLOPT_PORT, atol(port));
  free(port);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

  char* json_debug_string = json_dumps(post_fields, 0);
  debug("Setting post fields to %s", json_debug_string);
  free(json_debug_string);
  
  char* post_fields_string = json_dumps(post_fields, 0);
  curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, post_fields_string);
  free(post_fields_string);

  return curl;
}

/* Make a request to a DNAnexus API route */
struct Payload* dx_http_request(const char* resource, const json_t* data,
                                const struct curl_slist* headers)
{
  char url[2048];
  char* port = DXConfig_get_apiserver_port();
  char* protocol = DXConfig_get_apiserver_protocol();
  char* host = DXConfig_get_apiserver_host();
  sprintf(url, "%s://%s:%s/%s",
          protocol, host, port, resource); 
  debug("Request URL: %s", url);
  free(port); free(protocol); free(host);

  struct Payload* payload_ptr = calloc(1, sizeof(struct Payload));

  CURL* curl_handle = create_post_curl_handle(url, data, payload_ptr);
  debug("Created CURL handle at %p", curl_handle);

  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);
  return payload_ptr;
}

struct curl_slist* add_authorization(struct curl_slist* headers)
{
  char auth_header[200] = "Authorization: Bearer ";
  json_error_t error;
  char* security_context_string = DXConfig_get_security_context();
  json_t* security_context = json_loads(security_context_string, 0, &error);
  const char* auth_token = json_string_value(json_object_get(security_context, "auth_token"));
  strcat(auth_header, auth_token);
  headers = curl_slist_append(headers, auth_header);
  free(security_context_string);
  json_decref(security_context);
  return headers;
}

struct curl_slist* add_content_types(struct curl_slist* headers)
{
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  return headers;
}

struct curl_slist* add_range(struct curl_slist* headers, size_t start, size_t end)
{
  char range_header[256];
  sprintf(range_header, "Range: bytes=%zu-%zu", start, end);
  headers = curl_slist_append(headers, range_header);
  return headers;
}

struct curl_slist* add_get_headers(struct curl_slist* headers, json_t* response_headers)
{
  const char* key;
  json_t* value;
  json_object_foreach(response_headers, key, value) {
    char header[1024];
    const char* value_string = json_string_value(value);
    sprintf(header, "%s: %s", key, value_string);
    debug("GET header value %s", header);
    headers = curl_slist_append(headers, header);
  }
  return headers;
}

char* DXRequest_lookup_file(const char* file_name)
{
  char resource[] = "system/findDataObjects";
  json_t* request_data = json_object();
  json_t* json_file_string = json_string("file");
  json_object_set(request_data, "class", json_file_string);
  json_t* scope = json_object();
  char* project_id = DXConfig_get_project_context_id();
  json_t* json_project_id = json_string(project_id);
  json_object_set(scope, "project", json_project_id);
  free(project_id);
  json_object_set(request_data, "scope", scope);
  json_t* json_file_name = json_string(file_name);
  json_object_set(request_data, "name", json_file_name);
  
  struct curl_slist* headers = NULL;
  headers = add_authorization(headers);
  headers = add_content_types(headers);

  struct Payload* response = dx_http_request(resource, request_data, headers);
  curl_slist_free_all(headers);
  
  json_error_t error;
  json_t* response_json = json_loads(response->data, 0, &error);
  json_t* results = json_object_get(response_json, "results");
  const char* file_id = json_string_value(json_object_get(json_array_get(results, 0), "id"));

  char* output = malloc(strlen(file_id) + 1);
  strcpy(output, file_id);

  free(response->data); 
  free(response);
  json_decref(response_json);
  json_decref(request_data);
  json_decref(scope);
  json_decref(json_file_string);
  json_decref(json_file_name);
  json_decref(json_project_id);

  return output;
}

char* DXRequest_download(const char* file_name, size_t range_start, size_t range_end)
{
  /* First get the file ID associated with the file name. */
  char* file_id = DXRequest_lookup_file(file_name);

  /* Second, get the GET url via the file-xxxx/download route. */
  char resource[256];
  sprintf(resource, "%s/download", file_id);

  json_t* request_data = json_object();
  json_t* json_duration = json_integer(180);
  json_object_set(request_data, "duration", json_duration);
  
  struct curl_slist* headers = NULL;
  headers = add_authorization(headers);
  headers = add_content_types(headers);
  if(range_end > range_start) {
    headers = add_range(headers, range_start, range_end);
  }
   
  struct Payload* response = dx_http_request(resource, request_data, headers);
  curl_slist_free_all(headers);

  json_error_t error;
  json_t* response_json = json_loads(response->data, 0, &error);
  json_t* get_url_json = json_object_get(response_json, "url");
  const char* get_url = json_string_value(get_url_json);
  
  json_t* get_headers_json = json_object_get(response_json, "headers");
  struct curl_slist* get_headers = NULL;
  get_headers = add_get_headers(get_headers, get_headers_json);
  add_authorization(get_headers);
  if(range_end > range_start) {
    headers = add_range(get_headers, range_start, range_end);
  }
  
  struct Payload* get_payload = calloc(1, sizeof(struct Payload));
  CURL* curl = create_curl_handle(get_url, get_payload);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, get_headers);
  curl_easy_perform(curl);
  
  char* output = malloc(get_payload->size + 1);
  strncpy(output, get_payload->data, get_payload->size);
  output[get_payload->size] = '\0';
  return output;
}
