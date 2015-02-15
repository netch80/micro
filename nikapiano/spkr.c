#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <err.h>

#include "defs.h"

#define ARTICULATION_PAUSE      (-1)
#define ARTICULATION_NORMAL     0
#define ARTICULATION_LEGATO     1
#define ARTICULATION_STACCATO   2

static void
playNote(int notenum, int notelen, int sustain, int tempo, int articulation)
{
  // Tempo is defined as quarters per minute => "one" length in seconds
  // is 240/tempo. Then divide by notelen, and then divide by
  int totlenms, notelenms, pauselenms, pausepart;
  if (0) {
    printf("_: playNote(): notenum=%d notelen=%d sustain=%d"
        " tempo=%d articulation=%d\n",
        notenum, notelen, sustain, tempo, articulation);
  }
  totlenms = (int) (240000.0 / (tempo * notelen));
  if (sustain > 0)
    totlenms += totlenms - (totlenms >> sustain);
  if (notenum == 0 || articulation == ARTICULATION_PAUSE) {
    beep(0, totlenms);
    return;
  }
  if (notenum < 1 || notenum > 108)
    return;
  if (articulation == ARTICULATION_LEGATO) {
    beep(note_table[notenum-1], totlenms);
    return;
  }
  pausepart = ((articulation == ARTICULATION_STACCATO) ? 4 : 8);
  notelenms = (totlenms*(pausepart-1))/pausepart;
  pauselenms = totlenms-notelenms;
  if (0) {
    printf("_: notenum=%d notelenms=%d pauselenms=%d\n",
        notenum, notelenms, pauselenms);
  }
  beep(note_table[notenum-1], notelenms);
  beep(0, pauselenms);
}

void
speaker(const char *line)
{
  int octave, len, tempo, articulation;
  int cmd, subcmd, number, acci, sustain, legato1;
  int notenum, notelen, a1;
  static const int noteidx[7] = { 9, 11, 0, 2, 4, 5, 7 };
  // Global player values.
  //printf("_: speaker(<%s>)\n", line);
  octave = 4;
  len = 4;
  tempo = 120;
  articulation = 0;     // normal
  // One command parameters.
  cmd = subcmd = number = acci = sustain = legato1 = 0;
  // Begin.
  if (!line)
    return;
  for(;;) {
    if (*line && strchr(" \t\r\n\v\f", *line)) {
      ++line; continue;
    } else if (*line >= 48 && *line <= 57) {
      char *newpos;
      number = (int) strtol(line, &newpos, 10);
      line = newpos;
      continue;
    } else if (*line == '.') {
      ++sustain;
      ++line; continue;
    } else if (*line == '#' || *line == '+') {
      acci = 1;
      ++line; continue;
    } else if (*line == '-') {
      acci = -1;
      ++line; continue;
    } else if (*line  == '_') {
      legato1 = 1;
      ++line; continue;
    } else if(cmd == 'M' && subcmd == 0 && *line && strchr("NnLlSs", *line)) {
      subcmd = toupper(*line);
      ++line; continue;
    } else if (!*line || strchr("AaBbCcDdEeFfGgOoNnLlPpTtMm<>", *line)) {
      // Execute previous command
      if (cmd != 0) {
        cmd = toupper(cmd);
        //printf("_: executing cmd %C\n", cmd);
        if (cmd >= 'A' && cmd <= 'G') {
          // Play note
          notenum = octave * 12 + noteidx[cmd-'A'] + acci + 1;
          //printf("_: playing note %d\n", notenum);
          notelen = (number > 0) ? number : len;
          a1 = legato1 ? ARTICULATION_LEGATO : articulation;
          playNote(notenum, notelen, sustain, tempo, a1);
        }
        else if (cmd == 'O') {
          octave = number;
        }
        else if (cmd == '>') {
          ++octave;
        }
        else if (cmd == '<') {
          --octave;
        }
        else if (cmd == 'N') {
          notenum = number;
          notelen = len;
          a1 = legato1 ? ARTICULATION_LEGATO : articulation;
          playNote(notenum, notelen, sustain, tempo, a1);
        }
        else if (cmd == 'L') {
          if (number > 0)
            len = number;
        }
        else if (cmd == 'P') {
          // Pause
          notelen = (number > 0) ? number : len;
          playNote(0, notelen, sustain, tempo, -1);
        }
        else if (cmd == 'T') {
          if (number >= 10 && number <= 300)
            tempo = number;
        }
        else if (cmd == 'M') {
          if (subcmd == 'L')
            articulation = ARTICULATION_LEGATO;
          else if (subcmd == 'S')
            articulation = ARTICULATION_STACCATO;
          else
            articulation = ARTICULATION_NORMAL;
        }
      }
      // Clear after previous
      cmd = subcmd = number = acci = sustain = legato1 = 0;
      if (*line) {
        cmd = toupper(*line);
        //printf("_: located cmd %C\n", cmd);
        ++line;
      }
      else
        break;
    } else {
      // Syntax error?
      break;
    }
  }
}

// Emulate FreeBSD /dev/speaker.
int
cmdSpkr(int argc, char *argv[])
{
  int i;
  if( argc < 1 )
    errx(1, "usage: spkr <line>");
  initDevice();
  for (i = 0; i < argc; ++i)
    speaker(argv[i]);
  return 0;
}

// Emulate FreeBSD /dev/speaker.
int
cmdSpkrI(int argc, char *argv[])
{
  char line[256];
  initDevice();
  // Retry input
  while (fgets(line, sizeof(line)-1, stdin)) {
    line[sizeof(line)-1] = 0;
    speaker(line);
  }
  if (ferror(stdin)) {
    warn("fgets");
    return 1;
  }
  return 0;
}

// vim:ts=2:sts=2:sw=2:et:si:
