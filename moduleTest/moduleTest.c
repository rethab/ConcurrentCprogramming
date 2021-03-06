/* 
 * This file is part of the concurrent programming in C term paper
 *
 * It provides a testsuite for the project
 * Copyright (C) 2014 Max Schrimpf
 *
 * The file is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * The file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the project. if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <arpa/inet.h>  
#include <stdio.h>
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     

#include <termPaperLib.h>

char *server_ip;
unsigned short server_port;
int num_testcases;
int num_testcases_success;
int num_testcases_fail;

void runTestcase(const char *input, const char *expected) {
  num_testcases++;

  int sock = create_client_socket(server_port, server_ip);

  write_to_socket(sock, input);

  char *buffer_ptr[0];

  size_t received_msg_size = read_from_socket(sock, buffer_ptr);

  int result = strcmp(*buffer_ptr, expected);

  if(result == 0) {
    log_info("Testcase %zu: OK!", num_testcases);
    num_testcases_success++;  
  } else {
    log_info("Testcase %zu: FAILED!", num_testcases);
    log_debug("send: '%s'", input);
    log_debug("Expected: '%s'", expected);
    log_debug("Recived : '%s'", *buffer_ptr);
    
    num_testcases_fail++;  
  }

  free(*buffer_ptr);
  close(sock);
  // sleep since the requests went to fast and randomly crashed
  sleep(1);
}

void create_real_long_String(size_t len, char **result, char c) {
  char to_return[len+1];

  int i; 
  for(i = 0; i < len; i++) {
    to_return[i] = c;
  }
  to_return[len] = '\000';

  // \000
  *result = (char *) malloc(len+1);
  strncpy(*result, to_return, len+1);
}

void runFilencontentSizeTestFail(size_t in) {
  char real_long_string[MAX_MSG_LEN + MAX_MSG_LEN];
  char *bad_string;
  create_real_long_String(in, &bad_string, 'n');

  sprintf(real_long_string,"CREATE blub %zu\n%s\n", in, bad_string);
  runTestcase(real_long_string, "CONTENT_TO_LONG\n");

  runTestcase("READ blub\n","NOSUCHFILE\n");

  runTestcase("LIST\n", "ACK 0\n");
  
  sprintf(real_long_string,"UPDATE blub %zu\n%s\n", in, bad_string);
  runTestcase(real_long_string, "CONTENT_TO_LONG\n");
}

void runFilenameSizeTestFail(size_t in) {
  char real_long_string[MAX_MSG_LEN + 100];
  char *bad_string;
  create_real_long_String(in, &bad_string, 'n');

  sprintf(real_long_string,"CREATE %s 3\n123\n", bad_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");

  sprintf(real_long_string,"READ %s\n", bad_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");

  runTestcase("LIST\n", "ACK 0\n");
  
  sprintf(real_long_string,"UPDATE %s %zu\n%s\n", bad_string, in, bad_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");
}

void runExtremeLenTestsFail(size_t in) {
  char real_long_string[MAX_MSG_LEN + 100];
  char *name_string;
  char *bad_string;
  create_real_long_String(in, &name_string, 'r');
  create_real_long_String(in, &bad_string, 'n');

  sprintf(real_long_string,"CREATE %s %zu\n%s\n", name_string, in, bad_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");

  sprintf(real_long_string,"READ %s\n", name_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");

  runTestcase("LIST\n", "ACK 0\n");
  
  sprintf(real_long_string,"UPDATE %s %zu\n%s\n", name_string, in, bad_string);
  runTestcase(real_long_string, "FILENAME_TO_LONG\n");
}

void runRoundtripTest(char* filename, size_t length, char *content, size_t length_update, char *update) {
  char request[MAX_MSG_LEN + 100];
  char response[MAX_MSG_LEN + 100];

  sprintf(request,"CREATE %s %zu\n%s\n", filename, length, content);
  runTestcase(request, "FILECREATED\n");

  sprintf(response,"ACK 1\n%s\n", filename);
  runTestcase("LIST\n", response);

  sprintf(request,"READ %s\n", filename);
  sprintf(response,"FILECONTENT %s %zu\n%s\n", filename, length, content);
  runTestcase(request, response);

  sprintf(request,"UPDATE %s %zu\n%s\n", filename, length_update, update);
  runTestcase(request, "UPDATED\n");

  sprintf(response,"ACK 1\n%s\n", filename);
  runTestcase("LIST\n", response);

  sprintf(request,"READ %s\n", filename);
  sprintf(response,"FILECONTENT %s %zu\n%s\n", filename, length_update, update);
  runTestcase(request, response);

  sprintf(request,"DELETE %s\n", filename);
  runTestcase(request, "DELETED\n");

  runTestcase("LIST\n", "ACK 0\n");
}

void runLenTest(size_t filename_len, size_t content_len) {
  char *name_string;
  char *content_string;
  char *update_string;
  create_real_long_String(filename_len, &name_string, 'r');
  create_real_long_String(content_len, &content_string, 'q');
  create_real_long_String(content_len, &update_string, 'g');

  runRoundtripTest(name_string, content_len, content_string, content_len, update_string);
}

void runTestcases() {
  char *bad_string ;

// garbage 
  log_debug("---------------------------------------------------");
  log_debug("garbage as input");

  runTestcase("Lorem Ipsum set dolo\n", "COMMAND_UNKNOWN\n");
  runTestcase("qqqqqqqqqqqqqqqqqqqqqqqqq\n", "COMMAND_UNKNOWN\n");
  runTestcase("Lorem Ipsum set dolo\n", "COMMAND_UNKNOWN\n");
  create_real_long_String(2048, &bad_string, 'q');
  runTestcase(bad_string, "COMMAND_UNKNOWN\n");
  free(bad_string);
  create_real_long_String(3072, &bad_string, 'q');
  runTestcase(bad_string, "COMMAND_UNKNOWN\n");
  free(bad_string);

// test the instructor:
// Do you read this?
  runTestcase("Cdist\n", "FTW ;-)\n");

// filename with spaces 
  log_debug("---------------------------------------------------");
  log_debug("filenames with spaces");
  runTestcase("CREATE im possible 3\n123\n", "COMMAND_UNKNOWN\n");
  runTestcase("UPDATE im possible 3\n123\n", "COMMAND_UNKNOWN\n");
  runTestcase("DELETE im possible 3\n123\n", "COMMAND_UNKNOWN\n");
  runTestcase("READ im possible 3\n123\n", "COMMAND_UNKNOWN\n");

// filename with special characters
  log_debug("---------------------------------------------------");
  log_debug("filename with special characters");
  runRoundtripTest("file1", 3, "bnm", 5, "blube");
  runRoundtripTest("file?", 5, "nwwee", 5, "blube");
  runRoundtripTest("file!", 5, "ttttt", 3, "abc");
  runRoundtripTest("file_", 3, "bnm", 5, "blube");
  runRoundtripTest("file-", 3, "bnm", 5, "blube");

//content with spaces 
  log_debug("---------------------------------------------------");
  log_debug("content with spaces");
  runRoundtripTest("file", 3, "i i", 5, "i   i");
  runRoundtripTest("file", 3, "   ", 5, "    i");
  runRoundtripTest("file", 3, "i i", 5, "i    ");
  runRoundtripTest("file", 3, "i i", 5, "     ");

//content with spaces 
  log_debug("---------------------------------------------------");
  log_debug("content special characters");
  runRoundtripTest("file", 3, "123", 5, "12345");
  runRoundtripTest("file", 3, "!ab", 5, "cdis!");
  runRoundtripTest("file", 3, "dj$", 5, "bf$ss");

// messing arround with the lens
  log_debug("---------------------------------------------------");
  log_debug("runFilenameSizeTest");
  runLenTest((MAX_BUFLEN - 1), 33);
  runLenTest(MAX_BUFLEN, 33);

  runFilenameSizeTestFail((MAX_BUFLEN+1)  );
  runFilenameSizeTestFail((MAX_BUFLEN+5)  );

  log_debug("---------------------------------------------------");
  log_debug("runFile length tests");
  runTestcase("CREATE lol 0\n123\n", "COMMAND_UNKNOWN\n");
  runTestcase("UPDATE lol 0\nqwertz\n", "COMMAND_UNKNOWN\n");
  runTestcase("LIST\n", "ACK 0\n");

  // length > content => length = strlen(content)
  runTestcase("CREATE lol 12356\n123\n", "FILECREATED\n");
  runTestcase("READ lol\n", "FILECONTENT lol 3\n123\n");
  runTestcase("UPDATE lol 1337\nqwertz\n", "UPDATED\n");
  runTestcase("READ lol\n", "FILECONTENT lol 6\nqwertz\n");
  runTestcase("LIST\n", "ACK 1\nlol\n");
  runTestcase("DELETE lol\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 0\n");

  // content > length => length = strlen(content)
  runTestcase("CREATE lol 1\n123\n", "FILECREATED\n");
  runTestcase("READ lol\n", "FILECONTENT lol 1\n1\n");
  runTestcase("UPDATE lol 2\nqwertz\n", "UPDATED\n");
  runTestcase("READ lol\n", "FILECONTENT lol 2\nqw\n");
  runTestcase("LIST\n", "ACK 1\nlol\n");
  runTestcase("DELETE lol\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 0\n");

  log_debug("---------------------------------------------------");
  log_debug("runFilencontentSizeTest");
  runLenTest(33, (MAX_BUFLEN - 1));
  runLenTest(33, MAX_BUFLEN);

  runFilencontentSizeTestFail((MAX_BUFLEN+1)  );
  runFilencontentSizeTestFail((MAX_BUFLEN+5)  );

  log_debug("---------------------------------------------------");
  log_debug("runExtremeLenTests");
  runLenTest((MAX_BUFLEN - 1), (MAX_BUFLEN - 1));
  runLenTest(MAX_BUFLEN, MAX_BUFLEN);

  runExtremeLenTestsFail((MAX_BUFLEN+1)  );
  runExtremeLenTestsFail((MAX_BUFLEN+5)  );

  log_debug("---------------------------------------------------");
  log_debug("General Tests");

  runTestcase("DELETE lol\n", "NOSUCHFILE\n");
  runTestcase("READ lol\n", "NOSUCHFILE\n");
  runTestcase("LIST\n", "ACK 0\n");
  runTestcase("CREATE lol 3\n123\n", "FILECREATED\n");
  runTestcase("CREATE lol 3\n472\n", "FILEEXISTS\n");
  runTestcase("READ lol\n", "FILECONTENT lol 3\n123\n");
  runTestcase("LIST\n", "ACK 1\nlol\n");
  runTestcase("DELETE lol\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 0\n");
  runTestcase("READ lol\n", "NOSUCHFILE\n");
  runTestcase("DELETE lol\n", "NOSUCHFILE\n");
  runTestcase("CREATE lol 3\n123\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 1\nlol\n");
  runTestcase("READ lol\n", "FILECONTENT lol 3\n123\n");
  runTestcase("CREATE rofl 7\nasdifgj\n", "FILECREATED\n");
  runTestcase("READ lol\n", "FILECONTENT lol 3\n123\n");
  runTestcase("READ rofl\n", "FILECONTENT rofl 7\nasdifgj\n");
  runTestcase("LIST\n", "ACK 2\nlol\nrofl\n");
  runTestcase("CREATE hack1 0\nasdfgh\n", "COMMAND_UNKNOWN\n");
  runTestcase("LIST\n", "ACK 2\nlol\nrofl\n");
  runTestcase("CREATE hack1 1\nasdfgh\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 3\nlol\nrofl\nhack1\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\na\n");
  runTestcase("DELETE hack1\n", "DELETED\n");
  runTestcase("DELETE hack1\n", "NOSUCHFILE\n");
  runTestcase("READ hack1\n", "NOSUCHFILE\n");
  runTestcase("CREATE hack1 1\nasdfgh\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 3\nlol\nrofl\nhack1\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\na\n");
  runTestcase("CREATE hack3 0\n\n", "COMMAND_UNKNOWN\n");
  runTestcase("LIST\n", "ACK 3\nlol\nrofl\nhack1\n");
  runTestcase("CREATE hack3 3\n1\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 4\nlol\nrofl\nhack1\nhack3\n");
  runTestcase("READ hack3\n", "FILECONTENT hack3 1\n1\n");
  runTestcase("CREATE hack2 1337\n\n", "COMMAND_UNKNOWN\n");
  runTestcase("READ hack2\n", "NOSUCHFILE\n");
  runTestcase("CREATE hack2 1337\n1\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 5\nlol\nrofl\nhack1\nhack3\nhack2\n");
  runTestcase("READ hack2\n", "FILECONTENT hack2 1\n1\n");
  runTestcase("CREATE hack4 1337\nqqqqqqqq\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 6\nlol\nrofl\nhack1\nhack3\nhack2\nhack4\n");
  runTestcase("READ hack4\n", "FILECONTENT hack4 8\nqqqqqqqq\n");
  runTestcase("DELETE hack4\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 5\nlol\nrofl\nhack1\nhack3\nhack2\n");
  runTestcase("DELETE hack4\n", "NOSUCHFILE\n");
  runTestcase("LIST\n", "ACK 5\nlol\nrofl\nhack1\nhack3\nhack2\n");
  runTestcase("CREATE anotherFile 10\n12erhl7d9k\n", "FILECREATED\n");
  runTestcase("LIST\n", "ACK 6\nlol\nrofl\nhack1\nhack3\nhack2\nanotherFile\n");
  runTestcase("READ anotherFile\n", "FILECONTENT anotherFile 10\n12erhl7d9k\n");
  runTestcase("DELETE lol\n", "DELETED\n");
  runTestcase("DELETE lol\n", "NOSUCHFILE\n");
  runTestcase("READ rofl\n", "FILECONTENT rofl 7\nasdifgj\n");
  runTestcase("LIST\n", "ACK 5\nrofl\nhack1\nhack3\nhack2\nanotherFile\n");
  runTestcase("DELETE hack3\n", "DELETED\n");
  runTestcase("DELETE hack3\n", "NOSUCHFILE\n");
  runTestcase("LIST\n", "ACK 4\nrofl\nhack1\nhack2\nanotherFile\n");
  runTestcase("READ anotherFile\n", "FILECONTENT anotherFile 10\n12erhl7d9k\n");
  runTestcase("READ rofl\n", "FILECONTENT rofl 7\nasdifgj\n");
  runTestcase("READ hack4\n", "NOSUCHFILE\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\na\n");
  runTestcase("READ hack2\n", "FILECONTENT hack2 1\n1\n");

// UPDATE TESTS
  runTestcase("UPDATE hack1 0\nasdfgh\n", "COMMAND_UNKNOWN\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\na\n");
  runTestcase("UPDATE hack1 1\nzzzzzzzz\n", "UPDATED\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\nz\n");
  runTestcase("UPDATE hack1 13371\n\n", "COMMAND_UNKNOWN\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 1\nz\n");
  runTestcase("UPDATE hack1 13371\nxy\n", "UPDATED\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 2\nxy\n");
  runTestcase("UPDATE hack1 3\nlkjh\n", "UPDATED\n");
  runTestcase("READ hack1\n", "FILECONTENT hack1 3\nlkj\n");

  runTestcase("READ rofl\n", "FILECONTENT rofl 7\nasdifgj\n");
  runTestcase("READ hack4\n", "NOSUCHFILE\n");
  runTestcase("READ hack2\n", "FILECONTENT hack2 1\n1\n");
  runTestcase("LIST\n", "ACK 4\nrofl\nhack1\nhack2\nanotherFile\n");
  runTestcase("DELETE anotherFile\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 3\nrofl\nhack1\nhack2\n");
  runTestcase("DELETE hack1\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 2\nrofl\nhack2\n");
  runTestcase("UPDATE hack1 3\nlkjh\n", "NOSUCHFILE\n");
  runTestcase("DELETE rofl\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 1\nhack2\n");
  runTestcase("DELETE hack2\n", "DELETED\n");
  runTestcase("LIST\n", "ACK 0\n");
}

void usage(const char *argv0, const char *msg) {
  if (msg != NULL && strlen(msg) > 0) {
    printf("%s\n\n", msg);
  }
  printf("Usage:\n");

  char *usage = "<Server IP> [-p Port]";
  char *log_help = get_logging_help(&usage);
  printf("%s %s\n\n", argv0, usage);

  printf("Executes various tests on the fileserver\n");
  printf("A running server at the given address is needed\n\n\n");

  printf("<Server IP> Mandatory: Tries to connect to a server with the given IP\n\n");
  printf("[-p Port] Optional: Tries to connect to a server on the given port.\n");
  printf("          Default: 7000\n\n");
  printf("%s\n\n", log_help);

  printf("(c) Max Schrimpf - ZHAW 2014\n");
  exit(1);
}

int main(int argc, char *argv[]) {

  int retcode;
  if (is_help_requested(argc, argv)) {
    usage(argv[0], "Help:");
  }

  if (argc < 2 ) {  
    usage(argv[0], "wrong number of arguments please provide an IP at least");
  }

  server_port = 7000;  
  server_ip = argv[1];             

  int i;
  for (i = 2; i < argc; i++)  {
    if (strcmp(argv[i], "-p") == 0)  {
      if (i + 2 <= argc )  {
        i++;
        server_port = atoi(argv[i]);  
      } else {
        usage(argv[0], "please provide a port if you're using -p");
      }
    } 
  }
  get_logging_properties(argc, argv);

  num_testcases = 0;
  num_testcases_success = 0;
  num_testcases_fail = 0;

  runTestcases();

  log_info("Testsuite done!");
  log_info("Ran %d Tests", num_testcases);
  log_info("%d Tests succeded", num_testcases_success);
  log_info("%d Tests failed", num_testcases_fail);
}
