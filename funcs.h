void* check_malloc(unsigned int size)
{
  void* ptr = malloc(size);
  if (ptr == NULL)
  {
    perror("malloc failure");
    exit(1);
  }

  return ptr;
}

void usage()
{
  printf("Please specify the path of the chip-8 file as a command line argument.\n");
  printf("USAGE:\n");
  printf("./chip8 [FILENAME]\n");
}
