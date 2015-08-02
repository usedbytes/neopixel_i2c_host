#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "brightness.h"
#include "neopixel_i2c_host.h"

#define N_LEDS 16
#define DEVICENO 1
#define ADDR 0x40

#define IDLE_LEVEL 64

void startup(struct led_ctx *ctx)
{
	const struct timespec frame_duration = {
		.tv_sec = 0,
		.tv_nsec = 16666667,
	};
	const int n_frames = 60;
	const int brightness_step = (256 - IDLE_LEVEL) / (n_frames / 2);
	int level = IDLE_LEVEL;
	uint32_t color;
	uint8_t brightness;

	for (level = IDLE_LEVEL; level <= 255; level += brightness_step) {
		brightness = linear_map[level];

		color = COLOR(brightness, 0, 0);
		set_leds_global(ctx, color);
		nanosleep(&frame_duration, NULL);
	}

	nanosleep(&frame_duration, NULL);

	for (level = 255; level >= IDLE_LEVEL; level -= brightness_step) {
		brightness = linear_map[level];

		color = COLOR(brightness, 0, 0);
		set_leds_global(ctx, color);
		nanosleep(&frame_duration, NULL);
	}
}

int main(int argc, char *argv[])
{
	struct led_ctx *led_ctx = init_leds(DEVICENO, ADDR, N_LEDS);
	startup(led_ctx);

	return 0;
}

