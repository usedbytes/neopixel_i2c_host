/*
 * Helper routines for i2c-dev interaction with neopixel_i2c client
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
