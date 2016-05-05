#include <stdio.h>
#include <stdlib.h>

#include "minunit.h"

#include "dxrequest.h"

char* test_file_name_lookup()
{
  char* file_id = DXRequest_lookup_file("SRR497884.bam");
  if(file_id == NULL) return "x";

  mu_assert(strcmp(file_id, "file-Bj2Yx1009G0XkV2zvVgxyPXZ") == 0,
      "Got incorrect file_id.");

  if(file_id != NULL) free(file_id);

  return NULL;
}

char* test_download()
{
  char* value = DXRequest_download("SRR497884.bam", 1000, 2000);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  //mu_run_test(test_file_name_lookup);
  mu_run_test(test_download);
  return NULL;
}

RUN_TESTS(all_tests);
