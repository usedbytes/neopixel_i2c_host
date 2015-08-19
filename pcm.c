/*
 * Barebones interface to libasound
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
#include "pcm.h"

struct pcm_ctx {
	snd_pcm_t *handle;
	snd_pcm_format_t format;
	unsigned int rate;
};

struct pcm_ctx *pcm_init(const char *dev, snd_pcm_format_t format,
		unsigned int rate)
{
	int res;
	struct pcm_ctx *ctx;
	snd_pcm_hw_params_t *hw_params = NULL;

	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		return NULL;
	}
	memset(ctx, 0, sizeof(*ctx));

	res = snd_pcm_open(&ctx->handle, dev, SND_PCM_STREAM_CAPTURE, 0);
	if (res < 0) {
		fprintf(stderr, "%s: Failed to open audio device '%s' (%s)\n",
				__func__, dev, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_malloc(&hw_params);
	if (res < 0) {
		fprintf(stderr, "%s: Failed to alloc hw_params (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_any(ctx->handle, hw_params);
	if (res < 0) {
		fprintf(stderr, "%s: Failed to init hw_params (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_set_access(ctx->handle, hw_params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (res < 0) {
		fprintf(stderr, "%s: Couldn't set access mode (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_set_format(ctx->handle, hw_params, format);
	if (res < 0) {
		fprintf(stderr, "%s: Couldn't set format (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_set_rate_near(ctx->handle, hw_params, &rate, 0);
	if (res < 0) {
		fprintf(stderr, "%s: Couldn't set rate (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params_set_channels(ctx->handle, hw_params, 2);
	if (res < 0) {
		fprintf(stderr, "%s: Couldn't set channels (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	res = snd_pcm_hw_params(ctx->handle, hw_params);
	if (res < 0) {
		fprintf(stderr, "%s: Couldn't set params (%s)\n",
				__func__, snd_strerror(res));
		goto fail;
	}

	snd_pcm_hw_params_free(hw_params);

	return ctx;

fail:
	if (hw_params)
		snd_pcm_hw_params_free(hw_params);
	if (ctx->handle)
		snd_pcm_close(ctx->handle);
	free(ctx);
	return NULL;
}

void pcm_exit(struct pcm_ctx *ctx)
{
	snd_pcm_close(ctx->handle);
	free(ctx);
}

static int32_t mabs(int32_t v)
{
	return v < 0 ? -v : v;
}

int pcm_get_level(struct pcm_ctx *ctx, int n_samp)
{
	int16_t buffer[n_samp * 2];
	int mean = 0;
	int res;
	int i;

	res = snd_pcm_readi(ctx->handle, buffer, n_samp);
	if (res != n_samp) {
		fprintf(stderr, "Read failed (%s)\n", snd_strerror(res));
		return -1;
	}

	for (i = 0; i < n_samp; i++) {
		mean += mabs(buffer[i * 2]);
	}
	mean /= n_samp;

	return mean;
}
