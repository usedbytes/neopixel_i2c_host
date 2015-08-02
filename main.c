#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "brightness.h"
#include "neopixel_i2c_host.h"
#include "pcm.h"

#define N_LEDS 16
#define DEVICENO 1
#define ADDR 0x40

#define PCM_DEV "hw:Loopback,1,0"
#define PCM_FORMAT SND_PCM_FORMAT_S16_LE
#define PCM_RATE 44100

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
	int last_mean = 0;
	int mean;
	int level;
	uint32_t color;
	struct led_ctx *led_ctx = init_leds(DEVICENO, ADDR, N_LEDS);
	struct pcm_ctx *pcm_ctx = pcm_init(PCM_DEV, PCM_FORMAT, PCM_RATE);
	startup(led_ctx);

	for (;;) {
		mean = pcm_get_level(pcm_ctx, 1024);
		level = (mean + last_mean) / 256;
		last_mean = mean;
		level = level > IDLE_LEVEL ? level: IDLE_LEVEL;
		level = level < 256 ? level: 255;

		color = COLOR(linear_map[level], 0, 0);
		set_leds_global(led_ctx, color);
	}

	return 0;
}

