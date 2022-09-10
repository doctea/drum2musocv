#include "Harmony.hpp"


int scale_offset[NUM_SCALES][SCALE_SIZE] = {
  { 0, 2, 4, 5, 7, 9, 11 },     // major scale
  { 0, 2, 3, 5, 7, 8, 10 },     // natural minor scale
  { 0, 2, 3, 5, 7, 9, 11 },     // melodic minor scale 
  { 0, 2, 3, 5, 7, 8, 11 },     // harmonic minor scale
  { 0, 2, 4, 6, 7, 9, 11 },     // lydian
  { 0, 2, 4, 6, 8, 10, (12) },  // whole tone - 6 note scale - flavours for matching melody to chords
  { 0, 3, 5, 6, 7, 10, (12) },  // blues - flavours for matching melody to chords
  { 0, 2, 3, 6, 7, 8, 11 },     // hungarian minor scale

  // minor pent = natural minor but miss out 2nd and 8th
  // major pent = major but miss out 5th and 11th

  // mode of C - use chord but use the scale of
  // dorian D E F G A B C D
  //         2 1 2 2 2 2 2 
  // 6 modes per scale

  // relative major/minor are modes of each other
  //    C maj is also A minor
};

ChannelState autobass_input = ChannelState();   // global tracking notes that are held on incoming bass channel

extern Harmony harmony = Harmony(autobass_input);

// for use by qsort
int sort_pitch(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  //if (a==b==-1) return 0;
  if (b==-1 && a>b) return -1;
  if (a==-1 && b>a) return 1;
  
  return a < b ? 
          -1 : // a > b
          (a > b ? 
              1 : // a < b
              0); // equal
}

void sort_pitches(int pitches[], int len) {
  qsort(pitches, len, sizeof(pitches[0]), sort_pitch);
}
