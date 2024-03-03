#include "data.h"
#include <raylib.h>
#include <stdlib.h>

void *load_resource(const char *path, size_t *size) {
  int data_size;
  void *data = LoadFileData(path, &data_size);
  *size = (size_t)data_size;
  return data;
}

void free_resource(void *data) { free(data); }
