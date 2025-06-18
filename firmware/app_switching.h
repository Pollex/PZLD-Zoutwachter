#ifndef APP_SWITCHING_H
#define APP_SWITCHING_H

#include <stdint.h>

int app_switching_init(void);
int app_switching_connect(uint8_t requested_a_to_b_args[4]);

#endif /* end of include guard: APP_SWITCHING_H */
