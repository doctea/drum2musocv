#ifndef SCREEN_INCLUDED
#define SCREEN_INCLUDED

#include "lcdgfx.h"
#include "lcdgfx_gui.h"
//#include "owl.h"

// from lcdgfx library ssd1306_demo
//DisplaySSD1306_128x64_I2C display(-1); // This line is suitable for most platforms by default
//DisplaySSD1306_128x64_I2C display(-1); // This line is suitable for most platforms by default
DisplaySSD1306_128x64_I2C display(-1, {-1,0x3C,-1,-1,3400000}); 

void initialise_screen() {
  
    /* Select the font to use with menu and all font functions */
    display.setFixedFont( ssd1306xled_font6x8 );

    display.begin();

    display.clear();
}

void screen_update() {

  //display.clear();
  display.printFixed(0,  8, "Normal text", STYLE_NORMAL);
  display.printFixed(0, 16, "Bold text", STYLE_BOLD);
  display.printFixed(0, 24, "Italic text", STYLE_ITALIC);
  display.invertColors();
  display.printFixed(0, 32, "Inverted bold", STYLE_BOLD);
  display.invertColors();

/*
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  char output[32];

  sprintf(output, "%2i:%i:%i:%2i ", current_phrase, current_bar, current_beat, current_step);
  display.write(output);
  
  sprintf(output, "Mode: %s\n", 
    demo_mode==MODE_STANDBY ? "stby" : 
    demo_mode==MODE_EUCLIDIAN ? "Eucl" : 
    demo_mode==MODE_EUCLIDIAN_MUTATION ? "EuMu" : 
    demo_mode==MODE_RANDOM ? "RnDm" : 
    "????");
  display.write(output);

  display.setTextSize(2);
  
  sprintf(output, "%cBPM: %3.1f\n", bpm_internal_mode?'i':'e', bpm_current);
  display.write(output);

  //display.setTextSize(1);

  display.write(get_bass_info());
  display.write("\n");
  display.setTextSize(1);
  display.write(get_bass_info_2());
  display.write("\n");

  display.display();*/
}


#endif
