/*
Author: Salvador Felipe
Date: 3/5/2023
Course: CPSC 351-01
Professor: Dr. Shilpa Lakhanpal
Project 1
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

int parentToChildPipe[2]; /* The pipe for parent-to-child communications */
int childToParentPipe[2]; /* The pipe for the child-to-parent communication */
#define READ_END 0 /* The read end of the pipe */
#define WRITE_END 1 /* The write end of the pipe */
#define HASH_PROG_ARRAY_SIZE 6 /* The maximum size of the array of hash programs */
#define HASH_VALUE_LENGTH 1000 /* The maximum length of the hash value */
#define MAX_FILE_NAME_LENGTH 1000 /* The maximum length of the file name */
const string hashProgs[] = {"md5sum", "sha1sum", "sha224sum", "sha256sum",
                            "sha384sum", "sha512sum"}; /* The array of names of hash programs */
string fileName;

/**
 * The function called by a child
 * @param hashProgName - the name of the hash program
 */
void computeHash(const string& hashProgName)
{

  char hashValue[HASH_VALUE_LENGTH]; /* The hash value buffer */
  memset(hashValue, (char)NULL, HASH_VALUE_LENGTH); /* Reset the value buffer */
  char fileNameRecv[MAX_FILE_NAME_LENGTH]; /* The received file name string */
  memset(fileNameRecv, (char)NULL, MAX_FILE_NAME_LENGTH); /* Fill the buffer with 0's */

  /** Now, lets read a message from the parent **/
  if (read(parentToChildPipe[READ_END], fileNameRecv, MAX_FILE_NAME_LENGTH) < 0)
  {
    perror("read1");
    exit(-1);
  }

  fprintf(stderr, "Child received a string from the parent: %s\n",
          fileNameRecv);
  /* Glue together a command line <PROGRAM NAME>.
   * For example, sha512sum fileName.*/
  string cmdLine(hashProgName);
  cmdLine += " ";
  cmdLine += fileNameRecv;
  printf("cmdLine: %s\n", cmdLine.c_str());

  /* Open the pipe to the program (specified in cmdLine)
  * using popen() and save the ouput into hashValue. See popen.cpp
  * for examples using popen.*/
  FILE *hashOut = popen(cmdLine.c_str(), "r");

  /* Make sure the file pointer is valid */
  if (!hashOut)
  {
    perror("popen");
    exit(-1);
  }

  /* Reset the hash value buffer */
  memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

  /* Read the program output into the hashValue */
  if (fread(hashValue, sizeof(char), sizeof(char) * HASH_VALUE_LENGTH, hashOut) <
      0)
  {
    perror("fread");
    exit(-1);
  }
  /* Close the file pointer representing the hash output */
  if (pclose(hashOut) < 0)
  {
    perror("pclose");
    exit(-1);
  }

  fprintf(stdout, "Program output: %s\n", hashValue);
  fflush(stdout);

  /* Send a string to the parent*/
  if (write(childToParentPipe[WRITE_END], hashValue, HASH_VALUE_LENGTH) < 0)
  {
    perror("write1");
    exit(-1);
  }

  exit(0); /* The child terminates */
}

/**
 * The function called by the parent
 * @param hashProgName - the name of the hash program
 */
void parentFunc(const string& hashProgName)
{
  /* I am the parent */
  /** Close the unused ends of two pipes. **/
  /* Close the write end of the child-to-parent pipe */
  if (close(childToParentPipe[WRITE_END]) < 0)
  {
    perror("close4");
    exit(-1);
  }

  /* Close the read end of the parent-to-child pipe */
  if (close(parentToChildPipe[READ_END]) < 0)
  {
    perror("close5");
    exit(-1);
  }

  char hashValue[HASH_VALUE_LENGTH]; /* The buffer to hold the string received from the child */
  memset(hashValue, (char)NULL, HASH_VALUE_LENGTH); /* Reset the hash buffer */
  /* Send the string to the child*/
  if (write(parentToChildPipe[WRITE_END], fileName.c_str(),
            sizeof(fileName)) < 0){
    perror("write2");
    exit(-1);
  }

  /* Read the string sent by the child*/
  if (read(childToParentPipe[READ_END], hashValue, HASH_VALUE_LENGTH) < 0){
    perror("read2");
    exit(-1);
  }

  /* Print the hash value */
  fprintf(stdout, "%s HASH VALUE: %s\n", hashProgName.c_str(), hashValue);
  fflush(stdout);
}

int main(int argc, char** argv)
{
  /* Check for errors */
  if(argc < 2)
  {
    fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
    exit(-1);
  }

  fileName = argv[1]; /* Save the name of the file */
  pid_t pid; /* The process id */

  /* Run a program for each type of hashing algorithm hash algorithm */
  for (int hashAlgNum = 0; hashAlgNum < HASH_PROG_ARRAY_SIZE; ++hashAlgNum)
  {
  /** Create two pipes **/
  /* Create the parent-to-child pipe */
  if (pipe(parentToChildPipe) < 0)
  {
    perror("pipe");
    exit(-1);
  }

  /* Create the child-to-parent pipe */
  if (pipe(childToParentPipe) < 0)
  {
    perror("pipe");
    exit(-1);
  }

  /* Fork a child process and save the id */
  if ((pid = fork()) < 0)
  {
    perror("fork");
    exit(-1);
  }
  /* I am a child */
  else if (pid == 0)
  {
  /** Close the unused ends of two pipes **/
    /* Close the read end of the child-to-parent pipe */
    if (close(childToParentPipe[READ_END]) < 0)
    {
      perror("close6");
      exit(-1);
    }

    /* Close the write end of the parent-to-child pipe */
    if (close(parentToChildPipe[WRITE_END]) < 0)
    {
      perror("close7");
      exit(-1);
    }

    /* Compute the hash */
    computeHash(hashProgs[hashAlgNum]);
  }
  else
  {
    parentFunc(hashProgs[hashAlgNum]);
    /* Wait for the child to terminate */
    if (wait(NULL) < 0)
    {
      perror("wait");
      exit(-1);
    }
  }
  }
  return 0;
}
