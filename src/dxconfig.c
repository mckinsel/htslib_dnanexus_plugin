#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "jansson.h"
#include "dxconfig.h"

/* 
 * Get parent PID of the given PID. Note that this will only work on Linux,
 * probably only a subset of distros.
 *
 * Returns 0 if there is no parent or if something goes wrong.
 */
pid_t get_parent_pid(pid_t pid)
{
  char path[256], contents[1024];
  pid_t output = 0;
  sprintf(path, "/proc/%d/stat", pid);

  FILE* fp = NULL;
  if((fp = fopen(path, "r")) != NULL) {
    if((fgets(contents, 1024, fp)) == NULL) {
      return 0;
    }
    char* token = strtok(contents, " ");
    int counter = 0;
    while(token != NULL) {
      token = strtok(NULL, " ");
      counter++;
      if(counter == 3) {
        output = (pid_t) atoi(token);
        break;
      }
    } 
  }
  fclose(fp);
  
  return output;
}

char SESSION_PATH_PATTERN[] = "%s/.dnanexus_config/sessions/%d/environment.json";

/* 
 * Get the path of the environment.json file for the current session.
 *
 * Returns NULL if it can't find it.
 */
char* get_session_path()
{
  pid_t pid = getpid();
  FILE* fp = NULL;
  char* output = NULL;
  char* home = getenv("HOME");

  while(1) {
    char session_path[2048];
    sprintf(session_path, SESSION_PATH_PATTERN, home, (int) pid);
    if((fp = fopen(session_path, "r")) != NULL) {
      fclose(fp);
      output = malloc(strlen(session_path) + 1);
      strncpy(output, session_path, strlen(session_path) + 1);
      break;
    } else {
      pid = get_parent_pid(pid);
      if(pid == 0) {
        break;
      }
    }
  }
  
  return output;  
}

char* read_string_from_session(const char* key)
{
  char* session_path = get_session_path();
  char* output = NULL;
  if(session_path != NULL) {
    json_error_t error;
    json_t* json_root = json_load_file(session_path, 0, &error);
    free(session_path);
    if(!json_root) {
      return NULL;
    }
    json_t* value = json_object_get(json_root, key);

    const char* string_value = json_string_value(value);
    output = malloc(strlen(string_value) + 1);
    strncpy(output, string_value, strlen(string_value) + 1);

    json_decref(json_root);
  }
  return output;
}

char* lookup_string_value(const char* key)
{
  /* First, see if there's an environment variable, that takes precedence. */
  char* env_setting = getenv(key);
  if(env_setting != NULL) {
    size_t env_setting_size = strlen(env_setting) + 1;
    char* output = malloc(env_setting_size);
    strncpy(output, env_setting, env_setting_size);
    return output;
  }
  
  /* Now check for the sessions json file. */
  char* session_setting = read_string_from_session(key);
  if(session_setting != NULL) {
    size_t session_setting_size = strlen(session_setting) + 1;
    char* output = malloc(session_setting_size);
    strncpy(output, session_setting, session_setting_size);
    free(session_setting);
    return output;
  }

  /* Oh no, nothing worked. Return NULL */
  return NULL; 
}

char* DXConfig_get_security_context()
{
  return lookup_string_value("DX_SECURITY_CONTEXT");
}

char* DXConfig_get_apiserver_host()
{
  return lookup_string_value("DX_APISERVER_HOST");
}

char* DXConfig_get_apiserver_protocol()
{
  return lookup_string_value("DX_APISERVER_PROTOCOL");
}

char* DXConfig_get_apiserver_port()
{
  return lookup_string_value("DX_APISERVER_PORT");
}

char* DXConfig_get_project_context_id()
{
  return lookup_string_value("DX_PROJECT_CONTEXT_ID");
}
