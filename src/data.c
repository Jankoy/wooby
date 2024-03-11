#include "data.h"
#include <raylib.h>
#include <stdlib.h>

#define BUNDLE

#ifdef BUNDLE

#include "bundle.h"
#include <string.h>

void *load_resource_data(const char *file_path, size_t *size) {
  for (size_t i = 0; i < resources_count; ++i) {
    if (strcmp(resources[i].file_path, file_path) == 0) {
      *size = resources[i].size;
      return (void *)&bundle[resources[i].offset];
    }
  }
  return NULL;
}

void free_resource_data(void *data) { (void)data; }

#else

void *load_resource_data(const char *file_path, size_t *size) {
  int file_size;
  void *data = LoadFileData(file_path, &file_size);
  *size = (size_t)file_size;
  return data;
}

void free_resource_data(void *data) { free(data); }

#endif // BUNDLE
