// include the library code:
#include <LiquidCrystal.h>
#include "Tlc5940.h"
#include <Encoder.h>

const int rs = 8, en = 2, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Encoder myEnc(A1, A0);
int redraw = 1; //only redraw when changed


const int buttonPin = 17;    // Button
int buttonState = 0;         // variable for reading the pushbutton status

const int encPin = 16;    // Enc Pressed
int encState = 0;         // variable for reading the pushbutton status

long oldPosition  = 0;
long newPosition = 0;

//LED Flicker
long flicker[] = {10, 10, 20, 30, 30, 30, 40, 50, 60, 70, 80, 70, 70,
                  60, 60, 50, 50, 50, 60, 70, 80, 90, 100,
                  120, 140, 160, 240, 250, 100, 150, 250, 250, 140,
                  240, 230, 220, 100, 80, 70, 70, 70, 80, 80,
                  140, 130, 120, 110, 200, 210, 220, 220, 100, 90,
                  40, 30, 30, 30, 20, 10, 10, 10, 10, 20, 30, 30, 30, 40, 50, 60, 70, 80, 70, 70,
                  60, 60, 50, 50, 50, 60, 70, 80, 90, 100,
                  120, 140, 160, 240, 250, 100, 150, 250, 250, 140,
                  240, 230, 220, 100, 80, 70, 70, 70, 80, 80,
                  140, 130, 120, 110, 200, 210, 220, 220, 100, 90,
                  40, 30, 30, 30, 20, 10, 10
                 };
int count = 0;

//Menu related stuff
int select = 1; // which Channel is select
int mode = 0;
String mode_string[] = {"off", "full", "PWM", "Flicker"};
int menu = 0; // 0=Main, 1=ChannelMode, 2=Set PWM;
int channel_mode[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //0=off, 1=on-full, 2=on-dim, 3=flicker
long channel_pwm[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int btn_a = 0;
int tlc_chan = 0;

void setup() {
  Tlc.init();
  Tlc.clear();
  lcd.begin(16, 2);
  lcd.clear();
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(encPin, INPUT_PULLUP);
}

void loop()
{

  if ((redraw == 1) && (menu != 2)) //only write to Display after something changed
  {
    lcd.setCursor(0, 0);
    lcd.print("Channel = ");
    lcd.setCursor(11, 0); lcd.print(" "); //clear area display
    lcd.setCursor(10, 0);
    lcd.print(select);

    lcd.setCursor(0, 1);
    lcd.print("Modus = ");
    lcd.setCursor(8, 1); lcd.print("         "); //clear area display
    lcd.setCursor(8, 1);
    lcd.print(mode_string[channel_mode[select]]);
    redraw = 0;
  }

  if ((redraw == 1) && (menu == 2)) //only write to Display after something change and we are in PWM mode
  {
    lcd.setCursor(0, 0);
    lcd.print("Channel = ");
    lcd.setCursor(11, 0); lcd.print(" "); //clear area display
    lcd.setCursor(10, 0);
    lcd.print(select);

    lcd.setCursor(0, 1);
    lcd.print("PWM = ");
    lcd.setCursor(6, 1); lcd.print("        "); //clear area display
    lcd.setCursor(6, 1);
    lcd.print(channel_pwm[select]);
    redraw = 0;
  }

  //
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW)
  {
    btn_a = 1; //Helper

    if ((menu == 0) && (btn_a == 1)) //switch from Channel Select to Channel mode
    {
      menu = 1;
      btn_a = 0;
      redraw = 1;
    }

    if ((menu == 1) && (btn_a == 1)) //we need to check if we go to PWM Mode or back to Channel select
    {
      if (mode == 2) //PWM menu
      {
        menu = 2;
        btn_a = 0;
        redraw = 1;
      }
      if (mode != 2) //channel select menu
      {
        menu = 0;
        btn_a = 0;
        redraw = 1;
      }
    }

    if ((menu == 2) && (btn_a == 1)) //Back from PWM Mode to Channel select
    {
      menu = 0;
      btn_a = 0;
      redraw = 1;
    }

    delay(800);
   }

  if (menu == 0)
  {
    newPosition = myEnc.read() / 10;
    if (newPosition != oldPosition)
    {
      if (newPosition > oldPosition) {
        select++;
      }
      else select--;
      if (select > 16) {
        select = 16;
      }
      if (select < 1) {
        select = 1;
      }
      oldPosition = newPosition;
      redraw = 1;
    }
  }

  if (menu == 1)
  {
    newPosition = myEnc.read() / 10;
    if (newPosition != oldPosition)
    {
      if (newPosition > oldPosition) {
        mode++;
      }
      else mode--;
      if (mode > 3) {
        mode = 0;
      }
      if (mode < 0) {
        mode = 0;
      }
      oldPosition = newPosition;
      channel_mode[select] = mode;
      redraw = 1;
    }
  }

  if (menu == 2)
  {
    newPosition = myEnc.read() / 10;
    if (newPosition != oldPosition)
    {
      if ((newPosition > oldPosition) && (channel_pwm[select] < 100)) {
        channel_pwm[select]++;
      }
      else if (channel_pwm[select] > 0) {
        channel_pwm[select]--;
      }
      oldPosition = newPosition;
      redraw = 1;
    }
  }

  //output TLC all channels

  if (tlc_chan > 16) {
    tlc_chan = 1; //re start with channel 1 after channel 16 was written
  }
  if (count > 130) {
    count = 0; // set count for flicker to zero to restart loop
  }

  // Switch channel off
  if (channel_mode[tlc_chan] == 0)
  {
    Tlc.set(tlc_chan - 1, 0);
    Tlc.update();
  }

  // Switch channel full on
  if (channel_mode[tlc_chan] == 1)
  {
    Tlc.set(tlc_chan - 1, 1000);
    Tlc.update();
  }

  // Switch channel to PWM level
  if (channel_mode[tlc_chan] == 2)
  {
    Tlc.set(tlc_chan - 1, channel_pwm[tlc_chan] * 10);
    Tlc.update();
  }

  // Switch channel to PWM level from flicker table
  if (channel_mode[tlc_chan] == 3)
  {
    Tlc.set(tlc_chan - 1, flicker[count+tlc_chan] * 2);
    Tlc.update();
  }

  tlc_chan++;
  count++;
  delay(4);

} // main loop end
