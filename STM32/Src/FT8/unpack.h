#ifndef UNPACK_H_
#define UNPACK_H_

#include <stdint.h>

// field1 - at least 14 bytes
// field2 - at least 14 bytes
// field3 - at least 7 bytes
int unpack77_fields(const uint8_t *a77, char *field1, char *field2, char *field3);

#endif