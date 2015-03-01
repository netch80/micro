#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <err.h>

#include "defs.h"

int target;

int snd_dev_speed = 0; // device speed (AKA sampling rate)
int f_verbose = 0;
int volume = 50;
int allow_harmonics = 1;
char *ofilepath = NULL;
FILE *ofile = NULL;

// Verbose attempt to set speed for probe mode.
void
doProbeSpeed(int speed)
{
  int ret, rc, newspeed;
  ret = setSpeed(speed, &rc, &newspeed);
  if (ret == 1) {
    printf("Speed %d is supported exactly\n", speed);
    return;
  }
  if (ret == 2) {
    printf("Speed %d is supported almost (%d)\n", speed, newspeed);
    return;
  }
  if (ret == 0) {
    warnx("setSpeed(%d): %s", speed, strerror(rc));
    return;
  }
}

int
beep(double sfreq, int slen)
{
  // Needed arguments: frequency (in Hz) and length (in milliseconds)
  uint16_t *wt;
  size_t wtsize;

  // Generate wave table. We know speed and frequency.
  ;{
    int i;
    const double pi = 3.14159265358979;
    wtsize = (int)(0.5 + snd_dev_speed*(slen/1000.0));
    if (f_verbose) {
      printf("beep(): sfreq=%g slen=%d snd_dev_speed=%d wtsize=%d\r\n",
          sfreq, slen, snd_dev_speed, wtsize);
    }
    wt = alloca(sizeof(uint16_t) * wtsize);
    if( !wt )
      err(1, "alloca");
    for( i = 0; i < wtsize; ++i ) {
      double r1 = (2*pi*sfreq*i)/snd_dev_speed;
      double s = 240 * volume * sin(r1);
      if (allow_harmonics)
        s += 60 * volume * sin(2*r1) + 20 * volume * sin(3*r1);
      wt[i] = (int)s;
    }
  }

  // Play the table
  if (target == TARGET_FILE)
    fflush(ofile);

  writeFrames(wt, wtsize);

  return 0;
}

int
beepnote(int notenum, int slen)
{
  if (f_verbose) {
    printf("beepnote: notenum=%d slen=%d\r\n", notenum, slen);
  }
  if (notenum < 1 || notenum > 108)
    return -1;
  return beep(note_table[notenum-1], slen);
}

int
cmdBeep(int argc, char *argv[])
{
  // Needed arguments: frequency (in Hz) and length (in milliseconds)
  int sfreq, slen;

  // Get arguments.
  if (argc != 2)
    errx(1, "usage: [global options] beep <freq> <len>");
  sfreq = (int) strtol(argv[0], NULL, 0);
  if (sfreq < 100 || sfreq > 10000)
    errx(1, "beep: invalid frequency");
  slen = (int) strtol(argv[1], NULL, 0);
  if (slen < 1)
    errx(1, "beep: invalid length");

  initDevice();

  return beep(sfreq, slen);
}

double note_table[108] = {
  // Prefilled with integer values. It will be refilled later.
   16,   17,   18,   19,   20,   21,   23,   24,   25,   27,   29,   30,
   33,   35,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62,
   65,   69,   73,   78,   82,   87,   92,   98,  104,  110,  117,  123,
  131,  139,  147,  156,  165,  175,  185,  196,  208,  220,  233,  247,
  262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,
  523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,
 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902
};

void
initNoteTable(void)
{
  // Fill note table. Use octave cycle. Note 58 is scale base (440 Hz).
  // Hence, note 106 is 7040 Hz. Calculate all others from it.
  int obase, la;
  double logstep = log(2.0) / 12.0;
  for( obase = 105, la = 7040; obase >= 1; obase -= 12, la >>= 1 )
  {
    int j;
    note_table[obase] = la;
    for( j = -9; j <= 2; ++j ) {
      if( j == 0 )
        continue;
      note_table[obase+j] = la * exp(j*logstep);
    }
  }
}

int
cmdTable(int argc, char *argv[])
{
  int i, j, octave;
  static const char* octave_names[] = {
    "SC", "C", "M", "m", "1", "2", "3", "4", "5"
  };
  //initNoteTable();
  printf(
      "Octave    C         D         E    F         G         A         B\n");
  for( i = 0, octave = 0; i < 108; i += 12, ++octave ) {
    printf("O%1.1d(%2.2s)", octave, octave_names[octave]);
    for( j = 0; j < 12; ++j ) {
      printf(" %4d", (int)(0.5+note_table[i+j]));
    }
    printf(" <<%03d-%03d\n", i+1, i+12);
  }
  return 0;
}

int
cmdBeepNote(int argc, char *argv[])
{
  // Needed arguments: frequency (in Hz) and length (in milliseconds)
  int note, slen;
  double sfreq;

  // Get arguments.
  if (argc != 2)
    errx(1, "usage: [global options] beepnote <notenum> <len>");
  note = (int) strtol(argv[0], NULL, 0);
  if (note < 1 || note > 108)
    errx(1, "beepnote: invalid note");
  slen = (int) strtol(argv[1], NULL, 0);
  if (slen < 1)
    errx(1, "beepnote: invalid length");

  initDevice();
  initNoteTable();
  sfreq = note_table[note-1];
  return beep(sfreq, slen);
}

int
cmdHelp(void)
{
  printf(
"Usage: nikapiano [-d <devname>] [--vol=<volume>] <command>\n"
"Available commands:\n\n"
"probe   - probe device and print its capabilities\n"
"beep <freq> <len> - do one beep generation, len=ms\n"
"beepnote <notenum> <len> - do one note generation, notenum=1..108, len=ms;\n"
"        la1 is note 58\n"
"table - print freq table\n"
"spkr    - process speaker command line from cmdline\n"
"spkri   - read speaker commands from stdin and play them\n"
"\n"
"To play standard gamma:\n"
"nikapiano spkr 'c d e f g a b > c'\n"
"or\n"
"nikapiano spkr 'n49 n51 n53 n54 n56 n58 n60 n61'\n"
  );
  return 0;
}

#define LOPT_VERBOSE            1001
#define LOPT_VOLUME             1002
#define LOPT_OFILE              1003
#define LOPT_NO_HARMONICS       1004

int
main(int argc, char *argv[])
{
  struct option myoptions[] = {
    { "device", 1, 0, 'd' },
    { "verbose", 0, 0, LOPT_VERBOSE },
    { "vol", 1, 0, LOPT_VOLUME },
    { "ofile", 1, 0, LOPT_OFILE },
    { "no-harmonics", 0, 0, LOPT_NO_HARMONICS },
    { NULL, 0, 0, 0 }
  };
  const char *mode = NULL;
  int opt;

  while ((opt = getopt_long(argc, argv, "d:W;", myoptions, NULL)) != -1)
  {
    switch (opt) {
      case 'd':
        snd_dev_name = optarg;
      break;
      default:
      case '?':
        errx(1, "usage");
      break;
      case LOPT_VERBOSE:
        f_verbose = 1;
      break;
      case LOPT_VOLUME:
        volume = strtol(optarg, NULL, 0);
        if (volume < 0 || volume > 100)
          errx(1, "incorrect volume value (allowed=0..100)");
      break;
      case LOPT_OFILE:
        ofilepath = optarg;
      break;
      case LOPT_NO_HARMONICS:
        allow_harmonics = 0;
      break;
    }
  }
  argc -= optind;
  argv += optind;
  if (argc <= 0)
    errx(1, "usage: no working mode");
  mode = argv[0];
  --argc; ++argv;

  audio_open();

  if (!strcmp(mode, "help")) {
    return cmdHelp();
  }
  if (!strcmp(mode, "probe")) {
    printf("Doing probe on %s\n", snd_dev_name);
    return cmdProbe();
  }

  if (!strcmp(mode, "beep")) {
    return cmdBeep(argc, argv);
  }

  if (!strcmp(mode, "beepnote")) {
    return cmdBeepNote(argc, argv);
  }

  if (!strcmp(mode, "table")) {
    return cmdTable(argc, argv);
  }

  if (!strcmp(mode, "spkr")) {
    return cmdSpkr(argc, argv);
  }

  if (!strcmp(mode, "spkri")) {
    return cmdSpkrI(argc, argv);
  }

  if (!strcmp(mode, "piano")) {
    return cmdPiano();
  }

  errx(1, "fatal: unknown mode");

  return 1;
}

// vim:ts=2:sts=2:sw=2:et:si:
