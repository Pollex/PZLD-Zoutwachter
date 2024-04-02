#ifndef STWS281X_H
#define STWS281X_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void stws281x_init(void);
void stws281x_set(uint8_t n, uint8_t R, uint8_t G, uint8_t B);
void stws281x_clear(uint8_t n);
void stws281x_write(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* end of include guard: STWS281X_H */
