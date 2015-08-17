#ifndef _NEOPIXEL_I2C_HOST_
#define _NEOPIXEL_I2C_HOST_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define G_SHIFT	  0
#define R_SHIFT	  8
#define B_SHIFT	 16
#define GREEN(__color) (((__color) >> G_SHIFT) & 0xFF)
#define RED(__color)   (((__color) >> R_SHIFT) & 0xFF)
#define BLUE(__color)  (((__color) >> B_SHIFT) & 0xFF)
#define COLOR(__red, __green, __blue) ((__green << G_SHIFT) | \
		(__red << R_SHIFT) | (__blue << B_SHIFT))

struct led_ctx;

struct led_ctx *init_leds(uint8_t device_no, uint8_t addr, size_t n_leds);
int exit_leds(struct led_ctx *ctx);

int set_leds(struct led_ctx *ctx, uint32_t *colors, size_t n_leds);
int set_leds_global(struct led_ctx *ctx, uint32_t color);
int reset_leds(struct led_ctx *ctx);

#endif /* _NEOPIXEL_I2C_HOST_ */
