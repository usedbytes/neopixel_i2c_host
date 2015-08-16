#ifndef __CONTROL_H__
#define __CONTROL_H__
#include <stdbool.h>

struct host_ctx;

typedef int (*mode_proc_t)(struct host_ctx *);

struct mode {
	const char *name;
	mode_proc_t func;
};

struct host_ctx *control_init(void);
void control_exit(struct host_ctx *ctx);
int control_register_mode(struct host_ctx *ctx, struct mode *mode);
void control_enter_mode(struct host_ctx *ctx, const char *mode);

/*
 * mode_proc_t implementations must call this as often as possible.
 * If it returns true, then the mode_proc_t should clean up and return
 * 0 for normal exit
 */
bool check_for_update(struct host_ctx *ctx, int timeout);

#endif /* __CONTROL_H__ */
