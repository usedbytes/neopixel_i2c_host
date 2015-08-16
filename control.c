#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control.h"

struct list_head {
	struct list_head *next;
};

struct mode_node {
	struct list_head head;
	struct mode mode;
};

struct host_ctx {
	struct pollfd *fds;
	nfds_t nfds;
	/* TODO: How do we pass in args? */
	mode_proc_t current;
	mode_proc_t next;
	struct list_head mode_list;
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
	if (name)
		goto fail;
	strcpy(name, mode->name);
	node->mode.name = name;
	node->mode.func = mode->func;

	return node;

fail:
	destroy_node(node);
	return NULL;
}

int control_register_mode(struct host_ctx *ctx, struct mode *mode)
{
	struct mode_node *node = create_node(mode);
	if (!node)
		return -1;

	node->head.next = ctx->mode_list.next;
	ctx->mode_list.next = &node->head;

	return 0;
}

void control_enter_mode(struct host_ctx *ctx, const char *mode)
{

}

static int update_ctx(struct host_ctx *ctx, struct pollfd *fd)
{

}

bool check_for_update(struct host_ctx *ctx, int timeout)
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
			fprintf(stderr, "fd %i HUP\n", i);
		}
		if (fd->revents & POLLIN) {
			ret = update_ctx(ctx, fd);
			if (!ret)
				return true;
			/* TODO: Handle multiple fds (still need to drain them all) */
		}
	}

	return false;
}

struct host_ctx *control_init(void)
{
	struct host_ctx *ctx;

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

fail:
	if (ctx->fds)
		free(ctx->fds);
	if (ctx)
		free(ctx);
	return NULL;
}

void control_exit(struct host_ctx *ctx)
{
	struct list_head *next = ctx->mode_list.next;
	while (next) {
		struct mode_node *del = (struct mode_node *)next;
		next = del->head.next;
		destroy_node(del);
	}

	if (ctx->fds)
		free(ctx->fds);
	if (ctx)
		free(ctx);
}
