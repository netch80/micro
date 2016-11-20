#ifndef _DEFS_H_included
#define _DEFS_H_included 1

#include <stdio.h>
#include <stdint.h>

enum target { TARGET_DEVICE, TARGET_FILE };

extern int target;
extern const char *snd_dev_name;
extern int snd_dev;
extern int snd_dev_speed;
extern int f_verbose;
extern double note_table[108];
extern int volume;
extern char *ofilepath;
extern FILE *ofile;

void audio_open(void);
void initDevice(void);
void doProbeSpeed(int speed);
void writeFrames(const uint16_t *buf, size_t nframes);
void drain();
int setSpeed(int speed, int *rcp, int *newspeedp);
int beep(double sfreq, int slen);
int beepnote(int notenum, int slen);
int cmdProbe();
int cmdSpkr(int argc, char *argv[]);
int cmdSpkrI(int argc, char *argv[]);
int cmdPiano(void);

#endif

// vim:ts=2:sts=2:sw=2:et:si:
