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
