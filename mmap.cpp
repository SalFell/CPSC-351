#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OUTPUT_MODE 0700

int main(int argc, char **argv) {
  /* Make sure the command line is correct */
  if (argc < 3) {
    std::cout << "FILE NAME missing\n";

    exit(1);
  }

  /* Open the specified file */
  int inFd = open(argv[1], O_RDWR);
  if(inFd < 0) {
    std::cout << "\ninput file cannot be opened\n";
    exit(1);
  }

  int outFd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664);
  // int outFd = creat(argv[2], OUTPUT_MODE);

  if (outFd < 0) {
    std::cout << "\noutput file cannot be opened\n";
    exit(1);
  }

  struct stat stats;
  if (stat(argv[1], &stats) != 0){
    std::cout << "Unable to get file properties.\n";
  }

  int filesize = stats.st_size;

  /* Get the page size in bits*/
  int pagesize = getpagesize();
  pagesize *= 100;  // 100 pages

  /* Set the output file size to the expected size */
  if (ftruncate(outFd, filesize) != 0) {
    std::cout << "\nunable to set output file size\n";
    exit(1);
  }

  // initializing the offset to 0
  off_t offset = 0;

  // copying from inFd to outFd
  while (filesize > 0) {
    // adjusting the filesize and pagesize
    if (filesize < pagesize) {
      pagesize = filesize;
      filesize = 0;
    } else {
      filesize -= pagesize;
    }

    /* map the file into memory */
    char *src =
        (char *)mmap(NULL, pagesize, PROT_READ | PROT_WRITE,
                              MAP_SHARED, inFd, offset);

    /* Did the mapping fail ? */
    if (src == MAP_FAILED) {
      std::cout << "\nsource mapping did not succeed\n";
      exit(1);
    }

    /* map the file into memory */
    char *dest = (char *)mmap(NULL, pagesize, PROT_WRITE,
                              MAP_SHARED, outFd, offset);

    /* Did the mapping fail ? */
    if (dest == MAP_FAILED) {
      std::cout << "\ndestination mapping did not succeed\n";
      exit(1);
    }

    // copying pagesize bytes from src to dest
    memcpy(dest, src, pagesize);

    /* Unmap the shared memory region */
    munmap(src, pagesize);

    /* Unmap the shared memory region */
    munmap(dest, pagesize);

    // moving the read/write file offsets of inFd and outFd
    lseek(inFd, pagesize, SEEK_SET);
    lseek(outFd, pagesize, SEEK_SET);

    // updating file offset
    offset += pagesize;
  }

  /* Close the file */
  close(inFd);
  close(outFd);

  // end of program
  return 0;
}
