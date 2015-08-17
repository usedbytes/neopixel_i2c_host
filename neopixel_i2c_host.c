
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "neopixel_i2c_host.h"

/* For casting colors to char arrays (little endian only) */
#define G_INDEX    (G_SHIFT >> 3)
#define R_INDEX    (R_SHIFT >> 3)
#define B_INDEX    (B_SHIFT >> 3)

#define N_GLB_REG 4

struct led_ctx {
	int fd;
	size_t n_leds;
	bool global;
	uint32_t global_color;
	struct i2c_msg msg;
};

struct led_ctx *init_leds(uint8_t device_no, uint8_t addr, size_t n_leds)
{
	char filename[FILENAME_MAX];
	struct led_ctx *ctx;
	size_t n_bytes = sizeof(*ctx) + (3 * n_leds);

	ctx = malloc(n_bytes);
	if (!ctx)
		return NULL;

	memset(ctx, 0, n_bytes);

	ctx->n_leds = n_leds;

	ctx->msg.addr = addr;
	ctx->msg.buf = (unsigned char *)ctx + (3 * n_leds);

	snprintf(filename, FILENAME_MAX, "/dev/i2c-%d", device_no);
	ctx->fd = open(filename, O_RDWR);
	if (ctx->fd < 0) {
		fprintf(stderr, "Couldn't open '%s'\n", filename);
		goto fail;
	}

	return ctx;

fail:
	if (ctx->fd >= 0)
		close(ctx->fd);
	free(ctx);
	return NULL;
}

int exit_leds(struct led_ctx *ctx)
{
	int res = 0;

	close(ctx->fd);
	free(ctx);

	return res;
}

static void colors_to_array(uint8_t *buf, uint32_t *colors, size_t n)
{
	uint8_t *c = (uint8_t *)colors;
	while (n--) {
		buf[G_INDEX] = c[G_INDEX];
		buf[R_INDEX] = c[R_INDEX];
		buf[B_INDEX] = c[B_INDEX];
		c += 4;
		buf += 3;
	}
}

static int send_msg(int fd, struct i2c_msg *msg)
{
	struct i2c_rdwr_ioctl_data arg = { msg, 1 };
	int res = ioctl(fd, I2C_RDWR, &arg);
	if (res != 1) {
		fprintf(stderr, "ioctl returned %d (%s)\n", res, strerror(errno));
		return -1;
	}

	return 0;
}

int set_leds(struct led_ctx *ctx, uint32_t *colors, size_t n_leds)
{
	struct i2c_msg *msg = &ctx->msg;

	assert(n_leds <= ctx->n_leds);

	msg->len = (n_leds * 3) + 1;
	msg->buf[0] = N_GLB_REG;

	if (ctx->global) {
		msg->len += N_GLB_REG;
		msg->buf[0] = 0;
		msg->buf[1] = 0;
		colors_to_array(ctx->msg.buf + 2, &ctx->global_color, 1);
		ctx->global = false;
	}

	colors_to_array(ctx->msg.buf + N_GLB_REG, colors, n_leds);

	return send_msg(ctx->fd, msg);
}

int set_leds_global(struct led_ctx *ctx, uint32_t color)
{
	struct i2c_msg *msg = &ctx->msg;

	msg->len = N_GLB_REG + 1;
	msg->buf[0] = 0;
	msg->buf[1] = 2;
	colors_to_array(ctx->msg.buf + 2, &color, 1);
	ctx->global = true;
	ctx->global_color = color;

	return send_msg(ctx->fd, msg);
}

int reset_leds(struct led_ctx *ctx)
{
	struct i2c_msg *msg = &ctx->msg;

	msg->len = 2;
	msg->buf[0] = 0;
	msg->buf[1] = 1;
	ctx->global = false;

	return send_msg(ctx->fd, msg);
}
