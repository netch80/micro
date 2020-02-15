#ifndef spkr_HXX_included
#define spkr_HXX_included

class Speaker {
public:
  void speak(const char *);
private:
  int mOctave = 4;
  bool mOctaveTracking = false;
  int mLen = 4;
  int mTempo = 120;
  int mArticulation = 0; // ARTICULATION_NORMAL
  int mPreviousNoteIndex = -1;
};

#endif

// vim:ts=2:sts=2:sw=2:et:si:
