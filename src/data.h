#ifndef DATA_H_
#define DATA_H_

#include <stddef.h>
#include <stdint.h>

void *load_resource(const char *path, size_t *size);
void free_resource(void *data);

#endif // DATA_H_
