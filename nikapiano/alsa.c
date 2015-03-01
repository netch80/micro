#include <stdint.h>
#include <err.h>
#include <alsa/asoundlib.h>
#include "defs.h"

const char *snd_dev_name = "default";
snd_pcm_t *pcm = NULL;
snd_pcm_hw_params_t *pcm_params = NULL;

void
audio_open(void)
{
  int rc;
  rc = snd_pcm_open(&pcm, snd_dev_name, SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    err(1, "snd_pcm_open");
  }
  rc = snd_pcm_hw_params_malloc(&pcm_params);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_malloc");
  }
  rc = snd_pcm_hw_params_any(pcm, pcm_params);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_any");
  }
}

static void
audio_basic_config(void)
{
  int rc, speed, dir;

  rc = snd_pcm_hw_params_set_access(pcm, pcm_params,
      SND_PCM_ACCESS_RW_INTERLEAVED);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_set_access");
  }

  rc = snd_pcm_hw_params_set_format(pcm, pcm_params, SND_PCM_FORMAT_S16);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_set_format");
  }

  rc = snd_pcm_hw_params_set_channels(pcm, pcm_params, 1);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_set_channels");
  }

  // NB new ALSA interface passes rate & dir by pointer but some
  // tutorials still describe 0.9 with passing by value
  speed = 48000;
  dir = 0;
  rc = snd_pcm_hw_params_set_rate_near(pcm, pcm_params, &speed, &dir);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params_set_rate_near");
  }

  rc = snd_pcm_hw_params(pcm, pcm_params);
  if (rc < 0) {
    err(1, "snd_pcm_hw_params");
  }
}

// Speed probe. Format and channels shall be already configured.
// Returned value: 0 - failed, 1 - OK exactly, 2 - OK approximately
int
setSpeed(int speed, int *rcp, int *newspeedp)
{
  int rc, val = speed, dir = 0;
  double d;
  rc = snd_pcm_hw_params_set_rate_near(pcm, pcm_params, &speed, &dir);
  if (rc < 0) {
    if (rcp != NULL)
      *rcp = errno;
    return 0;
  }
  if (val == 0) {
    if (rcp != NULL)
      *rcp = EIO;
    return 0;
  }
  if (newspeedp != NULL)
    *newspeedp = val;
  if (val == speed)
    return 1;
  d = (speed*1.0)/val;
  if (d >= 0.95 && d <= 1.05)
    return 2;
  if (rcp != NULL)
    *rcp = ENOENT;
  return 0;
}

int
cmdProbe(void)
{
  int rc, mask, speed, dir;

  if (target == TARGET_FILE) {
    printf("For file, there is no hardware capabilities.\n");
    printf("File is written with: 48kHz/16bit/mono.\n");
    return 0;
  }

  printf("Selecting S16_NE and mono\n");
  audio_basic_config();
  // Required order is: format, channels, speed
  // Probe speed
  rc = snd_pcm_hw_params_get_rate(pcm_params, &speed, &dir);
  if (rc >= 0) {
    printf("Current speed: %d (dir=%d)\n", speed, dir);
  } else {
    warn("snd_pcm_hw_params_get_rate");
  }
  // Test both standard value set and some "left" values
  doProbeSpeed(96000);
  doProbeSpeed(88200);
  doProbeSpeed(48000);
  doProbeSpeed(44100);
  doProbeSpeed(24000);
  doProbeSpeed(22050);
  doProbeSpeed(16000);
  doProbeSpeed(11025);
  doProbeSpeed(8000);

  return 0;
}

void
initDevice(void)
{
  int val;
  static const int speed_list[] = {
    48000, 24000, 16000, 8000, 44100, 22050, 11025, 96000,
    0
  };

  if (target == TARGET_FILE) {
    // Use defaults
    snd_dev_speed = 48000;
    return;
  }

  // Initialize output device and select speed among preferred values
  audio_basic_config();
  do {
    int i, ret;
    for (i = 0; (val = speed_list[i]) != 0; ++i) {
      ret = setSpeed(val, NULL, NULL);
      if (ret == 1)
        goto speed_is_set;
    }
    for (i = 0; (val = speed_list[i]) != 0; ++i) {
      ret = setSpeed(val, NULL, NULL);
      if (ret > 0)
        goto speed_is_set;
    }
    errx(1, "cannot select speed");
  } while(0);
speed_is_set:
  snd_dev_speed = val;

}

void writeFrames(const uint16_t *wt, size_t wtsize)
{
  const char* buf = (const char*) wt;
  size_t pos, rest;
  ssize_t written;
  pos = 0;
  rest = wtsize;
  while (rest > 0) {
    written = snd_pcm_writei(pcm, &buf[pos], rest);
    if (written <= 0)
      err(1, "write");
    pos += written;
    rest -= written;
  }
}

// vim:ts=2:sts=2:sw=2:et:si:
