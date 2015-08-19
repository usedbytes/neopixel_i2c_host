/*
 * Main application for neopixel_i2c driver
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
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "brightness.h"
#include "control.h"
#include "neopixel_i2c_host.h"
#include "pcm.h"

#define N_LEDS 16
#define DEVICENO 1
#define ADDR 0x40

#define PCM_DEV "hw:Loopback,1,0"
#define PCM_FORMAT SND_PCM_FORMAT_S16_LE
#define PCM_RATE 44100

#define IDLE_LEVEL 64

int startup_func(struct ctrl_ctx *ctrl_ctx, struct arg_list *args)
{
	struct led_ctx *led_ctx;
	const struct timespec frame_duration = {
		.tv_sec = 0,
		.tv_nsec = 16666667,
	};
	const int n_frames = 60;
	const int brightness_step = (256 - IDLE_LEVEL) / (n_frames / 2);
	int level = IDLE_LEVEL;
	uint32_t color;
	uint8_t brightness;

	led_ctx = init_leds(DEVICENO, ADDR, N_LEDS);
	if (!led_ctx)
		return -1;

	for (level = IDLE_LEVEL; level <= 255; level += brightness_step) {
		brightness = linear_map[level];

		color = COLOR(brightness, 0, 0);
		set_leds_global(led_ctx, color);
		nanosleep(&frame_duration, NULL);
	}

	nanosleep(&frame_duration, NULL);

	for (level = 255; level >= IDLE_LEVEL; level -= brightness_step) {
		brightness = linear_map[level];

		color = COLOR(brightness, 0, 0);
		set_leds_global(led_ctx, color);
		nanosleep(&frame_duration, NULL);
	}

	exit_leds(led_ctx);

	/* Wait on idle level forever */
	return !control_check_for_update(ctrl_ctx, -1);
}

int audio_func(struct ctrl_ctx *ctrl_ctx, struct arg_list *args)
{
	struct pcm_ctx *pcm_ctx;
	struct led_ctx *led_ctx;
	int last_mean = 0;
	int mean;
	int level;
	uint32_t color;

	pcm_ctx = pcm_init(PCM_DEV, PCM_FORMAT, PCM_RATE);
	if (!pcm_ctx)
		return -1;

	led_ctx = init_leds(DEVICENO, ADDR, N_LEDS);
	if (!led_ctx) {
		pcm_exit(pcm_ctx);
		return -1;
	}

	while (1) {
		mean = pcm_get_level(pcm_ctx, 1024);
		level = (mean + last_mean) / 256;
		last_mean = mean;
		level = level > IDLE_LEVEL ? level: IDLE_LEVEL;
		level = level < 256 ? level: 255;

		color = COLOR(linear_map[level], 0, 0);
		set_leds_global(led_ctx, color);

		if (control_check_for_update(ctrl_ctx, 0))
			break;
	}

	exit_leds(led_ctx);
	pcm_exit(pcm_ctx);

	return 0;
}

int exit_func(struct ctrl_ctx *ctx, struct arg_list *args)
{
	return -1;
}

int main(int argc, char *argv[])
{
	int i;
	struct ctrl_ctx *ctrl_ctx;
	struct mode modes[] = {
		{
			.name = "startup",
			.func = startup_func,
		},
		{
			.name = "audio",
			.func = audio_func,
		},
		{
			.name = "exit",
			.func = exit_func,
		},

	};

	ctrl_ctx = control_init();
	if (!ctrl_ctx)
		return 1;

	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
		if (control_register_mode(ctrl_ctx, &modes[i]))
			goto exit;
	}

	control_loop(ctrl_ctx, "startup");

exit:
	control_exit(ctrl_ctx);
	return 0;
}

