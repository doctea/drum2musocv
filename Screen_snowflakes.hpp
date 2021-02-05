/*
    MIT License

    Copyright (c) 2018-2019, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
/**
 *   Attiny85 PINS
 *             ____
 *   RESET   -|_|  |- 3V
 *   SCL (3) -|    |- (2)
 *   SDA (4) -|    |- (1)
 *   GND     -|____|- (0)
 *
 *   Attiny85  I2C PINS: see picture above
 *   Digispark I2C PINS: D0/D2
 *   Atmega328 I2C PINS: connect LCD to A4/A5
 *   ESP8266   I2C PINS: GPIO4/GPIO5
 *   ESP32     I2C PINS: 21/22
 */

#include "lcdgfx.h"

DisplaySSD1306_128x64_I2C display(-1, {-1,0x3C,-1,-1,3400000}); 

/*
 * Define snowflake images directly in flash memory.
 * This reduces SRAM consumption.
 * The image is defined from bottom to top (bits), from left to right (bytes).
 */
const PROGMEM uint8_t snowFlakeImage[8][8] =
{
    {
        0B00111000,
        0B01010100,
        0B10010010,
        0B11111110,
        0B10010010,
        0B01010100,
        0B00111000,
        0B00000000
    },
    {
        0B00010000,
        0B01010100,
        0B00111000,
        0B11101110,
        0B00111000,
        0B01010100,
        0B00010000,
        0B00000000
    },
    {
        0B00111000,
        0B00010000,
        0B10111010,
        0B11101110,
        0B10111010,
        0B00010000,
        0B00111000,
        0B00000000
    },
    {
        0B00011000,
        0B01011010,
        0B00100100,
        0B11011011,
        0B11011011,
        0B00100100,
        0B01011010,
        0B00011000
    },
    {
        0B00010000,
        0B00111000,
        0B01010100,
        0B11101110,
        0B01010100,
        0B00111000,
        0B00010000,
        0B00000000
    },
    {
        0B10000010,
        0B00101000,
        0B01101100,
        0B00010000,
        0B01101100,
        0B00101000,
        0B10000010,
        0B00000000
    },
    {
        0B01000100,
        0B10101010,
        0B01101100,
        0B00010000,
        0B01101100,
        0B10101010,
        0B01000100,
        0B00000000
    },
    {
        0B00101000,
        0B01010100,
        0B10111010,
        0B01101100,
        0B10111010,
        0B01010100,
        0B00101000,
        0B00000000
    },
};

typedef NanoEngine1<DisplaySSD1306_128x64_I2C> NanoEngineDemo;

NanoEngineDemo engine(display);

class SnowFlake: public NanoFixedSprite<NanoEngineDemo::TilerT>
{
public:
    SnowFlake(): NanoFixedSprite<NanoEngineDemo::TilerT>({0, 0}, {8, 8}, nullptr)
    {
    }

    bool isAlive()
    {
        return hasTiler();
    }

    void generate()
    {
        setBitmap( &snowFlakeImage[lcd_random(8)][0] );
        /* Set initial position in scaled coordinates */
        /* We do not use getTiler().getDisplay() here, because if snowflake is not placed to
           engine, it has no tiler */
        scaled_position = { lcd_random(display.width() * 8), -8 * 8 };
        /* Use some random speed */
        speed = { lcd_random(-16, 16), lcd_random(4, 12) };
        /* After countdown timer ticks to 0, change X direction */
        timer = lcd_random(24, 48);
        moveTo( scaled_position/8 );
    }

    void update() override
    {
        scaled_position += speed;
        timer--;
        if (0 == timer)
        {
            /* Change movement direction */
            speed.x = lcd_random(-16, 16);
            timer = lcd_random(24, 48);
        }
        moveTo( scaled_position/8 );
        if (y() >= static_cast<lcdint_t>(getTiler().getDisplay().height()) )
        {
            getTiler().remove( *this );
        }
    }

private:
    NanoPoint scaled_position;
    NanoPoint speed;
    uint8_t timer;
};

static const uint8_t maxCount = 3;

/* These are our snow flakes */
SnowFlake snowFlakes[maxCount];

void initialise_screen()
{
  //delay(1000);
    display.begin();

//    display.128x64_i2c_init();
//    display.96x64_spi_init(3,4,5);
//    ssd1351_128x128_spi_init(3,4,5);
//    il9163_128x128_spi_init(3,4,5);

    engine.setFrameRate( 10 );
    engine.begin();

    engine.getDisplay().setFixedFont(ssd1306xled_font6x8);
    engine.getCanvas().setFixedFont(ssd1306xled_font6x8);

    engine.getCanvas().setMode(CANVAS_MODE_TRANSPARENT);
    engine.refresh();
}

void addSnowFlake()
{
    for (uint8_t i=0; i<maxCount; i++)
    {
        if (!snowFlakes[i].isAlive())
        {
            snowFlakes[i].generate();
            engine.insert( snowFlakes[i] );
            break;
        }
    }
}

static uint8_t globalTimer=3;

void screen_update()
{
  if (!engine.nextFrame()) return;
  //engine.refresh();
  //engine.display();
  //engine.getCanvas().setCursor(0, 0);     // Start at top-left corner

  char output[32];

  sprintf(output, "%2i:%i:%i:%2i ", current_phrase, current_bar, current_beat, current_step);
  engine.getCanvas().printFixed(0,0, output);
  
  sprintf(output, "Mode: %s\n", 
    demo_mode==MODE_STANDBY ? "stby" : 
    demo_mode==MODE_EUCLIDIAN ? "Eucl" : 
    demo_mode==MODE_EUCLIDIAN_MUTATION ? "EuMu" : 
    demo_mode==MODE_RANDOM ? "RnDm" : 
    "????");
  //engine.getCanvas().write(output);
  engine.getCanvas().printFixed(0, 9, output);

  //engine.setTextSize(2);
  
  sprintf(output, "%cBPM: %3.1f\n", bpm_internal_mode?'i':'e', bpm_current);
  engine.getCanvas().printFixed(0, 17, output);

  //display.setTextSize(1);

  //engine.getCanvas().write(get_bass_info());
  //engine.getCanvas().printFixed(0, 23, get_bass_info());
  
  /*engine.getCanvas().write("\n");
  engine.setTextSize(1);
  engine.getCanvas().write(get_bass_info_2());
  engine.getCanvas().write("\n");*/
    
    engine.update();
    engine.display();
}
