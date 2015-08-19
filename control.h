/*
 * Application state management
 *
 * Copyright 2015 Brian Starkey <stark3y@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef __CONTROL_H__
#define __CONTROL_H__
#include <stdbool.h>

struct ctrl_ctx;
/* TODO */
struct arg_list;

typedef int (*mode_proc_t)(struct ctrl_ctx *, struct arg_list *args);

struct mode {
	const char *name;
	mode_proc_t func;
};

struct ctrl_ctx *control_init(void);
void control_exit(struct ctrl_ctx *ctx);
int control_register_mode(struct ctrl_ctx *ctx, struct mode *mode);

/*
 * Enter the control loop. Will never return except for error
 * (A special mode could exit by returning -1 from its func)
 */
int control_loop(struct ctrl_ctx *ctx, const char *init_cmd);

/*
 * mode_proc_t implementations must call this as often as possible.
 * If it returns true, then the mode_proc_t should clean up and return
 * 0 for normal exit
 */
bool control_check_for_update(struct ctrl_ctx *ctx, int timeout);

#endif /* __CONTROL_H__ */
