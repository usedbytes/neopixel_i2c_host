#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "control.h"

struct list_head {
	struct list_head *next;
};

struct mode_node {
	struct list_head head;
	struct mode mode;
};

struct arg_list {
	struct list_head head;
};

struct mode_ctx {
	struct mode *mode;
	struct arg_list *args;
};

struct ctrl_ctx {
	struct pollfd *fds;
	nfds_t nfds;
	struct list_head mode_list;

	struct mode_ctx *current;
	struct mode_ctx *next;
};

static void destroy_node(struct mode_node *node)
{
	if (node->mode.name)
		free((void *)node->mode.name);
	free(node);
}

static struct mode_node *create_node(struct mode *mode)
{
	char *name;
	struct mode_node *node = malloc(sizeof(*node));
	if (!node)
		return NULL;
	memset(node, 0, sizeof(*node));

	name = malloc(strlen(mode->name) + 1);
	if (!name) {
		destroy_node(node);
		return NULL;
	}

	strcpy(name, mode->name);
	node->mode.name = name;
	node->mode.func = mode->func;

	return node;
}

int control_register_mode(struct ctrl_ctx *ctx, struct mode *mode)
{
	struct mode_node *node = create_node(mode);
	if (!node)
		return -1;

	node->head.next = ctx->mode_list.next;
	ctx->mode_list.next = &node->head;

	return 0;
}

static void destroy_mode_ctx(struct mode_ctx *ctx)
{
	free(ctx);
}

static struct mode_ctx *parse_cmd(struct list_head *modes, const char *buf)
{
	struct mode_ctx *new_mode = NULL;
	struct list_head *next = modes->next;

	while (next) {
		struct mode_node *node = (struct mode_node *)next;
		if (!strcmp(node->mode.name, buf)) {
			new_mode = malloc(sizeof(*new_mode));
			if (new_mode) {
				memset(new_mode, 0, sizeof(*new_mode));
				new_mode->mode = &node->mode;
				/* TODO: Parse args */
				printf("Matched on %s\n", node->mode.name);
			}
			break;
		}
		next = node->head.next;
	}

	return new_mode;
}

static int flush_fd(char **buf, struct pollfd *fd)
{
	while(poll(fd, 1, 0) > 0) {
		if (fd->revents & POLLERR)
			return -1;

		if (fd->revents & POLLIN) {
			int ret = read(fd->fd, *buf, 1);
			if (ret != 1) {
				return -1;
			}

			if (**buf == '\n') {
				**buf = '\0';
				return 1;
			} else {
				(*buf)++;
			}
		}
	}
	return 0;
}

static int handle_input(struct ctrl_ctx *ctx, struct pollfd *fd)
{
	static char buf[4096];
	static char *p = buf;
	int ret;

	ret = flush_fd(&p, fd);
	if (ret > 0) {
		struct mode_ctx *new_mode;
		new_mode = parse_cmd(&ctx->mode_list, buf);
		if (new_mode) {
			assert(!ctx->next);
			ctx->next = new_mode;
		} else {
			ret = -1;
		}
		memset(buf, 0, sizeof(buf));
		p = buf;
	}

	return ret;
}

bool control_check_for_update(struct ctrl_ctx *ctx, int timeout)
{
	int i;
	int ret = poll(ctx->fds, ctx->nfds, timeout);
	if (ret == 0)
		return false;

	if (ret < 0) {
		/* FIXME */
		fprintf(stderr, "Poll went wrong.\n");
		return false;
	}

	for (i = 0; i < ctx->nfds; i++) {
		struct pollfd *fd = &ctx->fds[i];
		if (fd->revents & POLLHUP) {
			/* FIXME */
			fprintf(stderr, "fd %i HUP\n", i);
		}
		if (fd->revents & POLLIN) {
			ret = handle_input(ctx, fd);
			if (ret > 0)
				return true;
			/* TODO: Handle multiple fds (still need to drain them all) */
		}
	}

	return false;
}

int control_loop(struct ctrl_ctx *ctx, const char *init_cmd)
{
	int ret;
	struct mode_ctx *init_mode;
	init_mode = parse_cmd(&ctx->mode_list, init_cmd);
	if (!init_mode)
		return -1;

	assert(!ctx->current);
	ctx->current = init_mode;

	while (true) {
		ret = ctx->current->mode->func(ctx, ctx->current->args);
		destroy_mode_ctx(ctx->current);
		if (ret < 0)
			break;
		ctx->current = ctx->next;
		ctx->next = NULL;
	}

	return ret;
}

struct ctrl_ctx *control_init(void)
{
	struct ctrl_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));

	ctx->fds = malloc(sizeof(*ctx->fds));
	if (!ctx->fds)
		goto fail;

	ctx->nfds = 1;
	ctx->fds[0].fd = fileno(stdin);
	ctx->fds[0].events = POLLIN | POLLERR | POLLHUP;

	return ctx;

fail:
	if (ctx->fds)
		free(ctx->fds);
	if (ctx)
		free(ctx);
	return NULL;
}

void control_exit(struct ctrl_ctx *ctx)
{
	struct list_head *next = ctx->mode_list.next;
	while (next) {
		struct mode_node *del = (struct mode_node *)next;
		next = del->head.next;
		destroy_node(del);
	}

	if (ctx->current)
		destroy_mode_ctx(ctx->current);
	if (ctx->next)
		destroy_mode_ctx(ctx->next);
	if (ctx->fds)
		free(ctx->fds);
	if (ctx)
		free(ctx);
}
