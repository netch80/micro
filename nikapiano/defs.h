#ifndef _DEFS_H_included
#define _DEFS_H_included 1

extern const char *snd_dev_name;
extern int snd_dev;
extern int snd_dev_speed;
extern int f_verbose;
extern double note_table[108];
extern int volume;

int setSpeed(int speed, int *rcp, int *newspeedp);
void initDevice(void);
int beep(double sfreq, int slen);
int beepnote(int notenum, int slen);
int cmdSpkr(int argc, char *argv[]);
int cmdSpkrI(int argc, char *argv[]);
int cmdPiano(void);

#endif

// vim:ts=2:sts=2:sw=2:et:si:
