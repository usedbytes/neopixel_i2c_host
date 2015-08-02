#ifndef _PCM_H_
#define _PCM_H_

#include <alsa/asoundlib.h>

struct pcm_ctx;

/* Only SND_PCM_FORMAT_S16_LE supported */
struct pcm_ctx *pcm_init(const char *dev, snd_pcm_format_t format,
		unsigned int rate);
void pcm_exit(struct pcm_ctx *ctx);

int pcm_get_level(struct pcm_ctx *ctx, int n_samp);

#endif /* _PCM_H_ */
