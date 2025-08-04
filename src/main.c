#include "../include/editor.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  char *file = load_file(argv[1]);
  edit_buffer(file, argv[1]);
  free(file);
  return 0;
}
