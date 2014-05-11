
/* 
 * This file is part of the concurrent programming in C term paper
 *
 * It handles messages from the client and starts the requested processing
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

#include <termPaperLib.h>
#include <concurrentLinkedList.h>

#include <string.h>

// Requests
#define READ   "READ"
#define LIST   "LIST"
#define CREATE "CREATE"
#define UPDATE "UPDATE"
#define DELETE "DELETE"

// Responses
// Errors
#define FILEEXISTS "FILEEXISTS\n"
#define NOSUCHFILE "NOSUCHFILE\n"
#define COMMAND_UNKNOWN "COMMAND_UNKNOWN\n"

// positive responses
#define ACK "ACK\n"
#define FILECREATED "FILECREATED\n"
#define FILECONTENT "FILECONTENT\n"
#define DELETED "DELETED\n"
#define UPDATED "UPDATED\n"


/*
 * List all files
 * Possible response:
 *  
 *      ACK NUM_FILES\n
 *      FILENAME\n
 */
char *list_files() {
  log_info("Performing LIST");

}

/*
 * Create a new file
 * Possible response:
 *  
 *      FILEEXISTS\n
 *  or
 *      FILECREATED\n
 */
char *create_file(char *filename, int length, char *content) {
  log_info("Performing CREATE %s", filename);

}

/*
 * Read the content of a file
 * Possible response:
 *  
 *      NOSUCHFILE\n
 *  or
 *      FILECONTENT FILENAME LENGTH\n
 *      CONTENT
 */
char *read_file(char *filename) {
  log_info("Performing READ %s", filename);

}

/*
 * Change the content of a file
 * Possible response:
 *
 *      NOSUCHFILE\n
 *  or
 *      UPDATED\n
 */
char *update_file(char *filename, int length, char *content) {
  log_info("Performing UPDATE %s", filename);

}

/*
 * Delete a file
 * Possible response:
 *
 *      NOSUCHFILE\n
 *  or
 *      DELETED\n
 */
char *delete_file(char *filename) {
  log_info("Performing DELETE %s", filename);
}

char *handle_message(size_t msg_size, char *msg) {
  
  // With tailing \n
  if( msg_size < 5) {
    log_error( "Command unknown: '%s'", msg);
    return COMMAND_UNKNOWN;
  }

  char *lines = malloc(msg_size);
  char *tokens = malloc(msg_size);
  strncpy(lines, msg, msg_size);
  strncpy(tokens, msg, msg_size);

  char *line = strtok(lines, "\n");
  size_t line_len = strlen(line);
  
  if( token == NULL) {
    log_error( "Command unknown: '%s'", msg);
    return COMMAND_UNKNOWN;
  }

  if (line_len == 4 && strncmp(token, LIST, 4) == 0) {
// LIST
    return list_files();
  }

  char *tokens = strtok(token, " ");
  size_t token_len = strlen(token);

  if (token_len == 4) {
    if (strncmp(token, READ, 4) == 0) {
// READ

      // TODO PARAMETER
      token = strtok(NULL, " ");
      if( token == NULL) {
        log_error( "Parameter unknown: '%s'", msg);
        return COMMAND_UNKNOWN;
      }


      return read_file();
    }
  } else if (token_len == 6) {
    if (strncmp(token, CREATE , 6) == 0) {
// CREATE
    } else if (strncmp(token, UPDATE, 6) == 0) {
// UPDATE
    } else if (strncmp(token, DELETE, 6) == 0) {
// DELETE
    }

    free(lines);
    free(tokens);
  } 

  // save  default
  log_error( "Command unknown: '%s'", msg);
  return COMMAND_UNKNOWN;
}
