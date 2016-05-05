#include <stdio.h>
#include <stdlib.h>

#include "minunit.h"
#include "jansson.h"

#include "dxconfig.h"

char* test_security_context()
{
  char* sec = DXConfig_get_security_context();
  json_error_t error;
  json_t* sec_json = json_loads(sec, 0, &error);
  mu_assert(json_object_get(sec_json, "auth_token_type") != NULL,
      "Failed to get auth_token_type from session information.");
  mu_assert(json_object_get(sec_json, "auth_token") != NULL,
      "Failed to get auth_token from session information.");

  if(sec != NULL) free(sec);
  json_decref(sec_json);

  return NULL;
}

char* test_apiserver()
{
  char* host = DXConfig_get_apiserver_host();
  char* protocol = DXConfig_get_apiserver_protocol();
  char* port = DXConfig_get_apiserver_port();
  
  mu_assert(strcmp(host, "api.dnanexus.com") == 0,
      "Failed to get correct apiserver host.");
  mu_assert(strcmp(protocol, "https") == 0,
      "Failed to get correct apiserver protocol.");
  mu_assert(strcmp(port, "443") == 0,
      "Failed to get correct apiserver port.");

  if(host != NULL) free(host);
  if(protocol != NULL) free(protocol);
  if(port != NULL) free(port);

  return NULL;
}

char* test_project()
{

  char* project_id = DXConfig_get_project_context_id();
  mu_assert(strstr(project_id, "project-") == project_id,
      "Failed to get a valid project ID.");

  if(project_id != NULL) free(project_id);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  mu_run_test(test_security_context);
  mu_run_test(test_apiserver);
  return NULL;
}

RUN_TESTS(all_tests);
