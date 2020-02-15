// Speaker line parser.
// See https://www.freebsd.org/cgi/man.cgi?query=speaker&sektion=4&manpath=FreeBSD+5.1-release
// for reference. Some minor features could be omitted.

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
  const int MIN_OCTAVE = 0;
  const int MAX_OCTAVE = 6;

  int octave, octave_tracking, len, tempo, articulation;
  int cmd, subcmd, number, acci, sustain, legato1;
  int notenum, notelen, a1;
  int previous_note_index;
  static const int noteidx[7] = { 9, 11, 0, 2, 4, 5, 7 };
  // Global player values.
  //printf("_: speaker(<%s>)\n", line);
  octave = 4;
  len = 4;
  tempo = 120;
  articulation = 0;     // normal
  octave_tracking = 0;
  previous_note_index = noteidx[0];
  // One command parameters.
  cmd = subcmd = number = acci = sustain = legato1 = 0;
  // Begin.
  if (!line)
    return;
  for(;;) {
    if (*line && strchr(" \t\r\n\v\f", *line)) {
      ++line; continue;
    } else if (*line >= 48 && *line <= 57) {
      // Full number, allow strtol to parse it and go to next position
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
    } else if (cmd == 'O' && subcmd == 0 && *line && strchr("LlNn", *line)) {
      subcmd = toupper(*line);
    } else if (!*line || strchr("AaBbCcDdEeFfGgOoNnLlPpTtMm<>~", *line)) {
      // Execute previous command
      if (cmd != 0) {
        cmd = toupper(cmd);
        //printf("_: executing cmd %C\n", cmd);
        if (cmd >= 'A' && cmd <= 'G') {
          // Play note
          // TODO add octave tracking
          int curr_idx = noteidx[cmd-'A'];
          if (octave_tracking && previous_note_index >= 0) {
            int idx_diff = curr_idx - previous_note_index;
            if (idx_diff < -6 && octave < MAX_OCTAVE) {
              ++octave;
            }
            else if (idx_diff > 6 && octave > MIN_OCTAVE) {
              --octave;
            }
          }
          notenum = octave * 12 + curr_idx + acci + 1;
          //printf("_: playing note %d\n", notenum);
          notelen = (number > 0) ? number : len;
          a1 = legato1 ? ARTICULATION_LEGATO : articulation;
          playNote(notenum, notelen, sustain, tempo, a1);
          previous_note_index = curr_idx;
        }
        else if (cmd == 'O') {
          if (subcmd != 0) {
            octave_tracking = (subcmd == 'L');
          } else {
            if (number < MIN_OCTAVE) {
              number = MIN_OCTAVE;
            }
            if (number > MAX_OCTAVE) {
              number = MAX_OCTAVE;
            }
            octave = number;
            previous_note_index = -1;
          }
        }
        else if (cmd == '>') {
          ++octave;
          previous_note_index = -1;
        }
        else if (cmd == '<') {
          --octave;
          previous_note_index = -1;
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
        else if (cmd == 'P' || cmd == '~') {
          // Pause
          notelen = (number > 0) ? number : len;
          playNote(0, notelen, sustain, tempo, -1);
        }
        else if (cmd == 'T') {
          if (number < 10) {
            number = 10;
          }
          if (number > 300) {
            number = 300;
          }
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
  drain();
  return 0;
}

// Emulate FreeBSD /dev/speaker.
int
cmdSpkrI(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
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
  drain();
  return 0;
}

// vim:ts=2:sts=2:sw=2:et:si:
