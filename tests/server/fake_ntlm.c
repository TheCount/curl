/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2010, Mandy Wu, <mandy.wu@intel.com>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

/*
 * This is a fake ntlm_auth, which is used for testing NTLM single-sign-on.
 * When DEBUGBUILD is defined, libcurl invoke this tool instead of real winbind
 * daemon helper /usr/bin/ntlm_auth. This tool will accept commands and
 * responses with a pre-written string saved in test case test2005.
 */

#define CURL_NO_OLDIES

#include "setup.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define ENABLE_CURLX_PRINTF
#include "curlx.h" /* from the private lib dir */
#include "getpart.h"
#include "util.h"

/* include memdebug.h last */
#include "memdebug.h"

#ifndef DEFAULT_LOGFILE
#define DEFAULT_LOGFILE "log/fake_ntlm.log"
#endif

const char *serverlogfile = DEFAULT_LOGFILE;

int main(int argc, char *argv[])
{
  char buf[1024];
  FILE *stream;
  char *filename;
  int error;
  char *type1_input = NULL, *type3_input = NULL;
  char *type1_output = NULL, *type3_output = NULL;
  size_t size = 0;
  int testnum;
  const char *env;
  int arg = 1;
  char *helper_user = (char *)"unknown";
  char *helper_proto = (char *)"unknown";
  char *helper_domain = (char *)"unknown";
  bool use_cached_creds = FALSE;

  while(argc > arg) {
    if(!strcmp("--use-cached-creds", argv[arg])) {
      use_cached_creds = TRUE;
      arg++;
    }
    else if(!strcmp("--helper-protocol", argv[arg])) {
      arg++;
      if(argc > arg)
        helper_proto = argv[arg++];
    }
    else if(!strcmp("--username", argv[arg])) {
      arg++;
      if(argc > arg)
        helper_user = argv[arg++];
    }
    else if(!strcmp("--domain", argv[arg])) {
      arg++;
      if(argc > arg)
        helper_domain = argv[arg++];
    }
    else {
      puts("Usage: fake_ntlm [option]\n"
           " --use-cached-creds\n"
           " --helper-protocol [protocol]\n"
           " --username [username]\n"
           " --domain [domain]");
      exit(1);
    }
  }

  env = getenv("NTLM_AUTH_TESTNUM");
  if (env) {
    testnum = strtoul(env, NULL, 10);
  } else {
    logmsg("Test number not specified in NTLM_AUTH_TESTNUM");
    exit(1);
  }

  env = getenv("NTLM_AUTH_SRCDIR");
  if (env) {
    path = env;
  }

  filename = test2file(testnum);
  stream=fopen(filename, "rb");
  if(!stream) {
    error = ERRNO;
    logmsg("fopen() failed with error: %d %s", error, strerror(error));
    logmsg("Error opening file: %s", filename);
    logmsg("Couldn't open test file %ld", testnum);
    exit(1);
  }
  else {
    /* get the ntlm_auth input/output */
    error = getpart(&type1_input, &size, "ntlm_auth_type1", "input", stream);
    fclose(stream);
    if(error || size == 0) {
      logmsg("getpart() type 1 input failed with error: %d", error);
      exit(1);
    }
  }

  stream=fopen(filename, "rb");
  if(!stream) {
    error = ERRNO;
    logmsg("fopen() failed with error: %d %s", error, strerror(error));
    logmsg("Error opening file: %s", filename);
    logmsg("Couldn't open test file %ld", testnum);
    exit(1);
  }
  else {
    size = 0;
    error = getpart(&type3_input, &size, "ntlm_auth_type3", "input", stream);
    fclose(stream);
    if(error || size == 0) {
      logmsg("getpart() type 3 input failed with error: %d", error);
      exit(1);
    }
  }

  while(fgets(buf, 1024, stdin)) {
    if(strcmp(buf, type1_input) == 0) {
      stream=fopen(filename, "rb");
      if(!stream) {
        error = ERRNO;
        logmsg("fopen() failed with error: %d %s", error, strerror(error));
        logmsg("Error opening file: %s", filename);
        logmsg("Couldn't open test file %ld", testnum);
        exit(1);
      }
      else {
        size = 0;
        error = getpart(&type1_output, &size, "ntlm_auth_type1", "output", stream);
        fclose(stream);
        if(error || size == 0) {
          logmsg("getpart() type 1 output failed with error: %d", error);
          exit(1);
        }
      }
      printf("%s", type1_output);
      fflush(stdout);
    }
    else if(strncmp(buf, type3_input, strlen(type3_input)) == 0) {
      stream=fopen(filename, "rb");
      if(!stream) {
        error = ERRNO;
        logmsg("fopen() failed with error: %d %s", error, strerror(error));
        logmsg("Error opening file: %s", filename);
        logmsg("Couldn't open test file %ld", testnum);
        exit(1);
      }
      else {
        size = 0;
        error = getpart(&type3_output, &size, "ntlm_auth_type3", "output", stream);
        fclose(stream);
        if(error || size == 0) {
          logmsg("getpart() type 3 output failed with error: %d", error);
          exit(1);
        }
      }
      printf("%s", type3_output);
      fflush(stdout);
    }
    else {
      printf("Unknown request\n");
      logmsg("invalid input: %s\n", buf);
      exit(1);
    }
  }
  return 1;
}