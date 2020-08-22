

void NOISY_DEBUG(long t, int d) {
  for (int i = 0 ; i < t; i++) {
    MIDI.sendControlChange(7, i % 127, 1);
    delay(d);
  }
}

void NUMBER_DEBUG(byte channel, byte data1, byte data2) {
  MIDI.sendControlChange(data1, data2, channel);
}
