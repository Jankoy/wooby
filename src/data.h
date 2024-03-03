#ifndef DATA_H_
#define DATA_H_

#include "types.h"

void *load_resource(const char *path, size_t *size);
void free_resource(void *data);

#endif // DATA_H_
