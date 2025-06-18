#include "app_leds.h"
#include "app_switching.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef APP_DO_FAT
#include "app_functional_tests.h"
#endif

static int _cmd_configure(int argc, char **argv) {
    (void)argv;
    if (argc <= 4) {
        puts("usage: configure <E1> <E2> <E3> <E4>");
        return 1;
    }
    static uint8_t field_pins[4];
    field_pins[0] = atoi(argv[1]) - 1;
    field_pins[1] = atoi(argv[2]) - 1;
    field_pins[2] = atoi(argv[3]) - 1;
    field_pins[3] = atoi(argv[4]) - 1;
    app_switching_connect(field_pins);

    return 0;
}

static const shell_command_t shell_commands[] = {
    {"configure", "Connect 4 F electrodes to M side", _cmd_configure},
    {NULL, NULL, NULL}};

int run_shell(void) {
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

int main(void) {
#ifdef APP_DO_FAT
    app_fat_begin();
    return 0;
#endif
    // TODO: These init functions always return 0 or they get stuck
    app_leds_init();
    app_switching_init();
    run_shell();
    return 0;
}
