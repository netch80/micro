#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <err.h>
#include "defs.h"

static struct termios ti0;

static int octave;

#define DEFLENGTH 180

static void
playChar(int c)
{
  //- printf("playchar: c=%d\r\n", c);
  switch(c) {
    case 'a': beepnote(octave*12 - 2, DEFLENGTH); break;
    case 'w': beepnote(octave*12 - 1, DEFLENGTH); break;
    case 's': beepnote(octave*12,     DEFLENGTH); break;
    case 'd': beepnote(octave*12 + 1, DEFLENGTH); break;
    case 'r': beepnote(octave*12 + 2, DEFLENGTH); break;
    case 'f': beepnote(octave*12 + 3, DEFLENGTH); break;
    case 't': beepnote(octave*12 + 4, DEFLENGTH); break;
    case 'g': beepnote(octave*12 + 5, DEFLENGTH); break;
    case 'h': beepnote(octave*12 + 6, DEFLENGTH); break;
    case 'u': beepnote(octave*12 + 7, DEFLENGTH); break;
    case 'j': beepnote(octave*12 + 8, DEFLENGTH); break;
    case 'i': beepnote(octave*12 + 9, DEFLENGTH); break;
    case 'k': beepnote(octave*12 + 10, DEFLENGTH); break;
    case 'o': beepnote(octave*12 + 11, DEFLENGTH); break;
    case 'l': beepnote(octave*12 + 12, DEFLENGTH); break;
    case ';': beepnote(octave*12 + 13, DEFLENGTH); break;
    case '[': beepnote(octave*12 + 14, DEFLENGTH); break;
    case '\'': beepnote(octave*12 + 15, DEFLENGTH); break;
    case ',': case '<':
      if (octave > 0) --octave;
      break;
    case '.': case '>':
      if (octave < 8) ++octave;
      break;
  }
}

int
cmdPiano(void)
{
  struct termios ti;
  if (!isatty(0))
    errx(1, "piano: usage: must be on terminal");
  if (tcgetattr(0, &ti0) < 0)
    errx(1, "piano: cannot get terminal attrs");
  printf("Entering piano mode. To exit, press <Return>.\n");
  fflush(stdout);
  initDevice();
  ti = ti0;
  cfmakeraw(&ti);
  ti.c_cc[VMIN] = 1;
  ti.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSADRAIN, &ti) < 0)
    errx(1, "piano: cannot get terminal attrs");
  setvbuf(stdin, NULL, _IONBF, 0);
  octave = 4;
  for(;;) {
    int c = getc(stdin);
    if (c == EOF)
      break;
    if (c == 10 || c == 13)
      break;
    playChar(c);
  }
  tcsetattr(0, TCSANOW, &ti0);
  return 0;
}
// vim:ts=2:sts=2:sw=2:et:si:
