

void NOISY_DEBUG(long t, int d) {
#ifdef USB_NATIVE
  Serial.print("NOISY_DEBUG: ");
  Serial.print(" t:");
  Serial.print(t);
  Serial.print(" d:");
  Serial.println(d);
#endif
  for (int i = 0 ; i < t; i++) {
    MIDIOUT.sendControlChange(7, i % 127, 1);
    delay(d);
  }
}

void NUMBER_DEBUG(byte channel, byte data1, byte data2) {
#ifdef USB_NATIVE
  Serial.print("NUMBER_DEBUG: ");
  Serial.print("channel ");
  Serial.print(channel);
  Serial.print("data1 ");
  Serial.print(data1);
  Serial.print("data2 ");
  Serial.print(data2);
  Serial.println(" <");
#endif
  MIDIOUT.sendControlChange(data1, data2, channel);
}
