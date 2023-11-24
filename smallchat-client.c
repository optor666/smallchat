/* smallchat-client.c -- Client program for smallchat-server.
 *
 * Copyright (c) 2023, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the project name of nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "chatlib.h"

/* ============================================================================
 * Low level terminal handling.
 * ========================================================================== */

void disableRawModeAtExit(void);

/* Raw mode: 1960 magic shit. */
int setRawMode(int fd, int enable) {
    /* We have a bit of global state (but local in scope) here.
     * This is needed to correctly set/undo raw mode. */
    static struct termios orig_termios; // Save original terminal status here.
    static int atexit_registered = 0; // Avoid registering atexit() many times.
    static int rawmode_is_set = 0; // True if raw mode was enabled.
                                   
    struct termios raw;

    /* If enable is zero, we just have to disable raw mode if it is 
     * currently set. */
    if (enable == 0) {
        /* Don't even check the return value as it's too late. */
        if (rawmode_is_set && tcsetattr(fd, TCSAFLUSH, &orig_termios) != -1) 
            rawmode_is_set = 0;
        return 0;
    }

/* At exit we'll try to fix the terminal to the initial conditions. */
void disableRawModeAtExit(void) {
    setRawMode(STDIN_FILENO, 0);
}

/* =============================================================================
 * Main program logic, finally :)
 * ========================================================================== */
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }

    /* Create a TCP connection with the server. */
    int s = TCPConnect(argv[1], atoi(argv[2]), 0);
    if (s == -1) {
        perror("Connecting to server");
        exit(1);
    }

    /* Put the terminal in raw mode: this way we will receive every 
     * single key stroke as soon as the user tyeps it. No buffering 
     * nor translation of escape sequences of any kind. */
    setRawMode(fileno(stdin), 1);

    /* Wait for the standard input or the server socket to 
     * have some data. */
    fd_set readfds;
    int stdin_fd = fileno(stdin);

    struct InputBuffer ib;
    inputBufferClear(&ib);

    close(s);
    return 0;
}
