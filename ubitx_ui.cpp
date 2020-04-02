#include <Arduino.h>
#include "nano_gui.h"
#include "settings.h"
#include "setup.h"
#include "ubitx.h"
#include "version.h"

static const unsigned int COLOR_TEXT = DISPLAY_WHITE;
static const unsigned int COLOR_BACKGROUND = DISPLAY_NAVY;

static const unsigned int COLOR_ACTIVE_VFO_TEXT = DISPLAY_WHITE;
static const unsigned int COLOR_ACTIVE_VFO_BACKGROUND = DISPLAY_BLACK;

static const unsigned int COLOR_INACTIVE_VFO_TEXT = DISPLAY_GREEN;
static const unsigned int COLOR_INACTIVE_VFO_BACKGROUND = DISPLAY_BLACK;

static const unsigned int COLOR_INACTIVE_TEXT = DISPLAY_GREEN;
static const unsigned int COLOR_INACTIVE_BACKGROUND = DISPLAY_BLACK;
static const unsigned int COLOR_INACTIVE_BORDER = DISPLAY_DARKGREY;

static const unsigned int COLOR_ACTIVE_TEXT = DISPLAY_BLACK;
static const unsigned int COLOR_ACTIVE_BACKGROUND = DISPLAY_ORANGE;
static const unsigned int COLOR_ACTIVE_BORDER = DISPLAY_WHITE;

static const unsigned int COLOR_VERSION_TEXT = DISPLAY_LIGHTGREY;

static const unsigned int LAYOUT_VFO_LABEL_X = 0;
static const unsigned int LAYOUT_VFO_LABEL_Y = 10;
static const unsigned int LAYOUT_VFO_LABEL_WIDTH = 159;
static const unsigned int LAYOUT_VFO_LABEL_HEIGHT = 36;
static const unsigned int LAYOUT_VFO_LABEL_PITCH_X = 160;

static const unsigned int LAYOUT_MODE_TEXT_X = 0;
static const unsigned int LAYOUT_MODE_TEXT_Y = LAYOUT_VFO_LABEL_Y + LAYOUT_VFO_LABEL_HEIGHT + 1;
static const unsigned int LAYOUT_MODE_TEXT_WIDTH = 320;
static const unsigned int LAYOUT_MODE_TEXT_HEIGHT = 36;

static const unsigned int LAYOUT_BUTTON_X = 2;
static const unsigned int LAYOUT_BUTTON_Y = LAYOUT_MODE_TEXT_Y + LAYOUT_MODE_TEXT_HEIGHT + 1;
static const unsigned int LAYOUT_BUTTON_WIDTH = 60;
static const unsigned int LAYOUT_BUTTON_HEIGHT = 36;
static const unsigned int LAYOUT_BUTTON_PITCH_X = 64;
static const unsigned int LAYOUT_BUTTON_PITCH_Y = 40;

static const unsigned int LAYOUT_CW_TEXT_X = 0;
static const unsigned int LAYOUT_CW_TEXT_Y = LAYOUT_BUTTON_Y + 3*LAYOUT_BUTTON_PITCH_Y + 1;
static const unsigned int LAYOUT_CW_TEXT_WIDTH = 220;
static const unsigned int LAYOUT_CW_TEXT_HEIGHT = 36;

static const unsigned int LAYOUT_VERSION_TEXT_X = LAYOUT_CW_TEXT_X + LAYOUT_CW_TEXT_WIDTH + 1;
static const unsigned int LAYOUT_VERSION_TEXT_Y = LAYOUT_CW_TEXT_Y;
static const unsigned int LAYOUT_VERSION_TEXT_WIDTH = 320 - LAYOUT_CW_TEXT_WIDTH - 1;
static const unsigned int LAYOUT_VERSION_TEXT_HEIGHT = LAYOUT_CW_TEXT_HEIGHT;

static const unsigned int LAYOUT_TX_X = 280;
static const unsigned int LAYOUT_TX_Y = LAYOUT_MODE_TEXT_Y;
static const unsigned int LAYOUT_TX_WIDTH = 40;
static const unsigned int LAYOUT_TX_HEIGHT = 36;

enum btn_set_e {
  BUTTON_VFOA,
  BUTTON_VFOB,
  BUTTON_RIT,
  BUTTON_USB,
  BUTTON_LSB,
  BUTTON_CW,
  BUTTON_SPL,
  BUTTON_80,
  BUTTON_40,
  BUTTON_30,
  BUTTON_20,
  BUTTON_17,
  BUTTON_15,
  BUTTON_10,
  BUTTON_BLANK_1,
  BUTTON_MNU,
  BUTTON_FRQ,
  BUTTON_TOTAL
};

struct Button {
  int x, y, w, h;
  unsigned int id;
  char text[5];
  char morse;
  void (*morse_status)(int8_t* val_out);//-1 if a low tone should play, +1 if a high tone should play
};

void msVfoA(int8_t* val_out){
  *val_out = (Vfo_e::VFO_A == globalSettings.activeVfo) ? 1 : -1;
}
void msVfoB(int8_t* val_out){
  *val_out = (Vfo_e::VFO_B == globalSettings.activeVfo) ? 1 : -1;
}
void msRit(int8_t* val_out){
  *val_out = globalSettings.ritOn ? 1 : -1;
}
void msUsb(int8_t* val_out){
  *val_out = (VfoMode_e::VFO_MODE_USB == GetActiveVfoMode()) ? 1 : -1;
}
void msLsb(int8_t* val_out){
  *val_out = (VfoMode_e::VFO_MODE_LSB == GetActiveVfoMode()) ? 1 : -1;
}
void msCw(int8_t* val_out){
  *val_out = (TuningMode_e::TUNE_CW == globalSettings.tuningMode) ? 1 : -1;
}
void msSpl(int8_t* val_out){
  *val_out = globalSettings.splitOn ? 1 : -1;
}
void msIgnore(int8_t* val_out){
  *val_out = 0;
}

constexpr Button btn_set[BUTTON_TOTAL] PROGMEM = {
  {LAYOUT_VFO_LABEL_X + 0*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_VFO_LABEL_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_VFO_LABEL_HEIGHT, BUTTON_VFOA, "VFOA", 'A', msVfoA},
  {LAYOUT_VFO_LABEL_X + 1*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_VFO_LABEL_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_VFO_LABEL_HEIGHT, BUTTON_VFOB, "VFOB", 'B', msVfoB},
  
  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_RIT, "RIT", 'R', msRit},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_USB, "USB", 'U', msUsb},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_LSB, "LSB", 'L', msLsb},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_CW ,  "CW", 'C', msCw},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_SPL, "SPL", 'S', msSpl},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_80, "80", '8', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_40, "40", '4', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_30, "30", '3', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_20, "20", '2', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_17, "17", '7', msIgnore},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_15 ,  "15", '5', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_10 ,  "10", '1', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_BLANK_1, "", '\0', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_MNU, "\x7F", 'M', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, BUTTON_FRQ, "FRQ", 'F', msIgnore},
};

static const unsigned int KEYS_OFFSET = 256;//Unique from buttons
enum keypad_e {
  KEYS_1 = KEYS_OFFSET,
  KEYS_2,
  KEYS_3,
  KEYS_BLANK_1,
  KEYS_OK,
  KEYS_4,
  KEYS_5,
  KEYS_6,
  KEYS_0,
  KEYS_BACKSPACE,
  KEYS_7,
  KEYS_8,
  KEYS_9,
  KEYS_BLANK_2,
  KEYS_CANCEL,
  KEYS_TOTAL = KEYS_CANCEL - KEYS_OFFSET + 1
};
constexpr Button keypad[KEYS_TOTAL] PROGMEM = {
  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_1,        "1", '1', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_2,        "2", '2', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_3,        "3", '3', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_BLANK_1,   "", '\0', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 0*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_OK,      "OK", 'K', msIgnore},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_4,          "4", '4', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_5,          "5", '5', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_6,          "6", '6', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_0,          "0", '0', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 1*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_BACKSPACE, "<-", 'B', msIgnore},

  {LAYOUT_BUTTON_X + 0*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_7,        "7", '7', msIgnore},
  {LAYOUT_BUTTON_X + 1*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_8,        "8", '8', msIgnore},
  {LAYOUT_BUTTON_X + 2*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_9,        "9", '9', msIgnore},
  {LAYOUT_BUTTON_X + 3*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_BLANK_2,   "", '\0', msIgnore},
  {LAYOUT_BUTTON_X + 4*LAYOUT_BUTTON_PITCH_X, LAYOUT_BUTTON_Y + 2*LAYOUT_BUTTON_PITCH_Y, LAYOUT_BUTTON_WIDTH, LAYOUT_BUTTON_HEIGHT, KEYS_CANCEL, "Can", 'C', msIgnore},
};

boolean getButton(btn_set_e index, Button* button){
  if(BUTTON_TOTAL == index){
    return false;
  }
  memcpy_P(button, &(btn_set[index]), sizeof(Button));
  return true;
}

inline void drawCommandbar(char* text){
  displayText(text, LAYOUT_MODE_TEXT_X, LAYOUT_MODE_TEXT_Y, LAYOUT_MODE_TEXT_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
}

/** A generic control to read variable values
*/
int getValueByKnob(int minimum, int maximum, int step_size,  int initial, const __FlashStringHelper* prefix, const __FlashStringHelper* postfix)
{
  int knob = 0;
  int knob_value;

  while (btnDown())
    active_delay(100);

  active_delay(200);
  knob_value = initial;

  strncpy_P(b,(const char*)prefix,sizeof(b));
  itoa(knob_value, c, 10);
  strncat(b, c, sizeof(b) - strlen(b));
  strncat_P(b, (const char*)postfix, sizeof(b) - strlen(b));
  drawCommandbar(b);

  while(!btnDown() && digitalRead(PTT) == HIGH){
    knob = enc_read();
    if (knob != 0){
      if (knob_value > minimum && knob < 0)
        knob_value -= step_size;
      if (knob_value < maximum && knob > 0)
        knob_value += step_size;

      strncpy_P(b,(const char*)prefix,sizeof(b));
      itoa(knob_value, c, 10);
      strncat(b, c, sizeof(b) - strlen(b));
      strncat_P(b,(const char*)postfix, sizeof(b) - strlen(b));
      drawCommandbar(b);
    }
  checkCAT();
  }
  b[0] = 0;
  drawCommandbar(b);
  return knob_value;
}

void displayVFO(Vfo_e vfo){
  uint16_t displayColor = COLOR_INACTIVE_TEXT;
  uint16_t displayBackground = COLOR_INACTIVE_BACKGROUND;
  uint16_t displayBorder = COLOR_INACTIVE_BORDER;
  Button button;

  if (globalSettings.splitOn){
    if (vfo == globalSettings.activeVfo){
      c[0] = 'R';
    }
    else{
      c[0] = 'T';
    }
  }
  else{
    if(VFO_A == vfo){
      c[0] = 'A';
    }
    else if(VFO_B == vfo){
      c[0] = 'B';
    }
    else{
      c[0] = '?';
    }
  }
  c[1] = ':';
  c[2] = ' ';


  if (VFO_A == vfo){
    getButton(BUTTON_VFOA, &button);
    formatFreq(globalSettings.vfoA.frequency, c+3, sizeof(c)-3, 10);

    if (VFO_A == globalSettings.activeVfo){
      displayColor = COLOR_ACTIVE_VFO_TEXT;
      displayBackground = COLOR_ACTIVE_VFO_BACKGROUND;
      displayBorder = COLOR_ACTIVE_BORDER;
    }else{
      displayColor = COLOR_INACTIVE_VFO_TEXT;
      displayBackground = COLOR_INACTIVE_VFO_BACKGROUND;
      displayBorder = COLOR_INACTIVE_BORDER;
    }
  }

  if (VFO_B == vfo){
    getButton(BUTTON_VFOB, &button);
    formatFreq(globalSettings.vfoB.frequency, c+3, sizeof(c)-3, 10);

    if (VFO_B == globalSettings.activeVfo){
      displayColor = COLOR_ACTIVE_VFO_TEXT;
      displayBackground = COLOR_ACTIVE_VFO_BACKGROUND;
      displayBorder = COLOR_ACTIVE_BORDER;
    } else {
      displayColor = COLOR_INACTIVE_VFO_TEXT;
      displayBackground = COLOR_INACTIVE_VFO_BACKGROUND;
      displayBorder = COLOR_INACTIVE_BORDER;
    }
  }

  displayText(c, button.x, button.y, button.w, button.h, displayColor, displayBackground, displayBorder);
}

void btnDrawActive(Button* button){
  displayText(button->text, button->x, button->y, button->w, button->h, COLOR_ACTIVE_TEXT, COLOR_ACTIVE_BACKGROUND, COLOR_INACTIVE_BORDER);
}
void btnDrawInactive(Button* button){
  displayText(button->text, button->x, button->y, button->w, button->h, COLOR_INACTIVE_TEXT, COLOR_INACTIVE_BACKGROUND, COLOR_INACTIVE_BORDER);
}

void btnDraw(struct Button *button){
  switch(button->id){
    case BUTTON_VFOA:
    {
      displayVFO(VFO_A);
      break;
    }
    case BUTTON_VFOB:
    {
      displayVFO(VFO_B);
      break;
    }
    case BUTTON_RIT:
    {
      if(globalSettings.ritOn){
        btnDrawActive(button);
      }
      else{
        btnDrawInactive(button);
      }
      break;
    }
    case BUTTON_USB:
    {
      if(VFO_MODE_USB == GetActiveVfoMode()){
        btnDrawActive(button);
      }
      else{
        btnDrawInactive(button);
      }
      break;
    }
    case BUTTON_LSB:
    {
      if(VFO_MODE_LSB == GetActiveVfoMode()){
        btnDrawActive(button);
      }
      else{
        btnDrawInactive(button);
      }
      break;
    }
    case BUTTON_SPL:
    {
      if(globalSettings.splitOn){
        btnDrawActive(button);
      }
      else{
        btnDrawInactive(button);
      }
      break;
    }
    case BUTTON_CW:
    {
      if(TuningMode_e::TUNE_CW == globalSettings.tuningMode){
        btnDrawActive(button);
      }
      else{
        btnDrawInactive(button);
      }
      break;
    }
    default:
    {
      btnDrawInactive(button);
      break;
    }
  }//End switch
}


void displayRIT(){
  c[0] = 0;
  displayFillrect(LAYOUT_MODE_TEXT_X,LAYOUT_MODE_TEXT_Y,LAYOUT_MODE_TEXT_WIDTH,LAYOUT_MODE_TEXT_HEIGHT, COLOR_BACKGROUND);
  if(globalSettings.ritOn){
    strncpy_P(c,(const char*)F("TX:"),sizeof(c));
    formatFreq(globalSettings.ritFrequency, c+3, sizeof(c)-strlen(c));
    if (VFO_A == globalSettings.activeVfo)
      displayText(c, LAYOUT_VFO_LABEL_X + 0*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_MODE_TEXT_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
    else
      displayText(c, LAYOUT_VFO_LABEL_X + 1*LAYOUT_VFO_LABEL_PITCH_X, LAYOUT_MODE_TEXT_Y, LAYOUT_VFO_LABEL_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  }
}

void fastTune(){
  //if the btn is down, wait until it is up
  while(btnDown())
    active_delay(50);
  active_delay(50);
  
  strncpy_P(c,(const char*)F("Fast tune"),sizeof(c));
  displayText(c, LAYOUT_MODE_TEXT_X, LAYOUT_MODE_TEXT_Y, LAYOUT_MODE_TEXT_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
  while(true){
    checkCAT();

    //exit after debouncing the btnDown
    if (btnDown()){
      displayFillrect(LAYOUT_MODE_TEXT_X, LAYOUT_MODE_TEXT_Y, LAYOUT_MODE_TEXT_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_BACKGROUND);

      //wait until the button is realsed and then return
      while(btnDown())
        active_delay(50);
      active_delay(50);
      return;
    }
    
    int encoder = enc_read();
    if (encoder != 0){
      uint32_t freq = GetActiveVfoFreq();
      if (encoder > 0 && freq < 30000000l){
        freq += 50000l;
      }
      else if (encoder < 0 && freq > 600000l){
        freq -= 50000l;
      }
      setFrequency(freq);
      displayVFO(globalSettings.activeVfo);
    }
  }// end of the event loop
}

void enterFreq(){
  //force the display to refresh everything
  //display all the buttons
  
  for (int i = 0; i < KEYS_TOTAL; i++){
    Button button;
    memcpy_P(&button, &(keypad[i]), sizeof(Button));
    btnDraw(&button);
  }

  while(btnDown())
    active_delay(50);
  active_delay(50);

  int cursor_pos = 0;
  memset(c, 0, sizeof(c));

  bool exit = false;
  while(!exit){

    checkCAT();
    if(btnDown()){
      while(btnDown())
        active_delay(50);
      active_delay(50);
      break;
    }
    if(!readTouch())
      continue;
      
    scaleTouch(&ts_point);

    for (int i = 0; i < KEYS_TOTAL; i++){
      Button button;
      memcpy_P(&button, &(keypad[i]), sizeof(Button));

      int x2 = button.x + button.w;
      int y2 = button.y + button.h;
  
      if(button.x < ts_point.x && ts_point.x < x2 && 
         button.y < ts_point.y && ts_point.y < y2){
        //Serial.print(F("Entered key: "));
        //Serial.println(button.text);
        switch(button.id){
          case KEYS_OK:
          {
            uint32_t new_freq = atol(c);
            if((LOWEST_FREQ/1000 <= new_freq) && (new_freq <= HIGHEST_FREQ/1000)){
              new_freq *= 1000L;
              
              uint32_t prev_freq = GetActiveVfoFreq();
              //Transition from below to above the traditional threshold for USB
              if(prev_freq < THRESHOLD_USB_LSB && new_freq >= THRESHOLD_USB_LSB){
                SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
              }
              
              //Transition from aboveo to below the traditional threshold for USB
              if(prev_freq >= THRESHOLD_USB_LSB && new_freq < THRESHOLD_USB_LSB){
                SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
              }

              if (VFO_A == globalSettings.activeVfo){
                globalSettings.vfoA.frequency = new_freq;
              }
              else{
                globalSettings.vfoB.frequency = new_freq;
              }

              setFrequency(new_freq);
              saveVFOs();
            }
            exit = true;
            break;
          }
          
          case KEYS_BACKSPACE:
          {
            c[cursor_pos] = 0;
            if (cursor_pos > 0){
              cursor_pos--;
            }
            c[cursor_pos] = 0;
            break;
          }
          case KEYS_CANCEL:
          {
            exit = true;
            break;
          }
          case KEYS_0:
          case KEYS_1:
          case KEYS_2:
          case KEYS_3:
          case KEYS_4:
          case KEYS_5:
          case KEYS_6:
          case KEYS_7:
          case KEYS_8:
          case KEYS_9:
          {
            c[cursor_pos++] = button.text[0];
            c[cursor_pos] = 0;
            break;
          }
          default:
          {
            //Serial.print(F("Unknown key id: "));
            //Serial.println(button.id);
            break;
          }
        }//switch
      }//if button hit test
    }// end of the button scanning loop
    strncpy(b, c, sizeof(b));
    strncat_P(b,(const char*)F(" KHz"),sizeof(b) - strlen(b));
    displayText(b, LAYOUT_MODE_TEXT_X, LAYOUT_MODE_TEXT_Y, LAYOUT_MODE_TEXT_WIDTH, LAYOUT_MODE_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND);
    active_delay(300);
    while(readTouch())
      checkCAT();
  } // end of event loop : while(!exit)

  guiUpdate();
  enc_read();//clear out any tuner turning that happened during entry
}

void drawCWStatus(){
  strncpy_P(b,(const char*)F(" cw: "),sizeof(b));
  int wpm = 1200/globalSettings.cwDitDurationMs;
  itoa(wpm,c, 10);
  strncat(b, c, sizeof(b) - strlen(b));
  strncat_P(b,(const char*)F("wpm, "), sizeof(b) - strlen(b));
  itoa(globalSettings.cwSideToneFreq, c, 10);
  strncat(b, c, sizeof(b) - strlen(b));
  strncat_P(b,(const char*)F("hz"), sizeof(b) - strlen(b));
  displayText(b, LAYOUT_CW_TEXT_X, LAYOUT_CW_TEXT_Y, LAYOUT_CW_TEXT_WIDTH, LAYOUT_CW_TEXT_HEIGHT, COLOR_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND, TextJustification_e::Left);
}

void drawVersion()
{
  strncpy_P(b,VERSION_STRING,sizeof(b));
  displayText(b, LAYOUT_VERSION_TEXT_X, LAYOUT_VERSION_TEXT_Y, LAYOUT_VERSION_TEXT_WIDTH, LAYOUT_VERSION_TEXT_HEIGHT, COLOR_VERSION_TEXT, COLOR_BACKGROUND, COLOR_BACKGROUND, TextJustification_e::Right);
}

void drawTx(){
  if (globalSettings.txActive){
    strncpy_P(b,(const char*)F("TX"), sizeof(b));
    displayText(b, LAYOUT_TX_X, LAYOUT_TX_Y, LAYOUT_TX_WIDTH, LAYOUT_TX_HEIGHT, COLOR_ACTIVE_TEXT, COLOR_ACTIVE_BACKGROUND, COLOR_BACKGROUND);
  }
  else{
    displayFillrect(LAYOUT_TX_X, LAYOUT_TX_Y, LAYOUT_TX_WIDTH, LAYOUT_TX_HEIGHT, COLOR_BACKGROUND);
  }
}
void drawStatusbar(){
  drawCWStatus();
  drawVersion();
}

void guiUpdate(){

/*
  if (doingCAT)
    return;
*/
  // use the current frequency as the VFO frequency for the active VFO
  displayClear(COLOR_BACKGROUND);

  displayVFO(VFO_A);
  displayVFO(VFO_B);

  checkCAT();
  displayRIT();
  checkCAT();

  //force the display to refresh everything
  //display all the buttons
  for (int i = 0; i < BUTTON_TOTAL; i++){
    Button button;
    memcpy_P(&button, &(btn_set[i]), sizeof(Button));
    btnDraw(&button);
    checkCAT();
  }
  drawStatusbar();
  checkCAT();
}



// this builds up the top line of the display with frequency and mode
void updateDisplay() {
   displayVFO(globalSettings.activeVfo);
}

void ritToggle(struct Button *button){
  if(!globalSettings.ritOn){
    ritEnable(GetActiveVfoFreq());
  }
  else{
    ritDisable();
  }
  btnDraw(button);
  displayRIT();
}

void splitToggle(Button *button){

  globalSettings.splitOn = !globalSettings.splitOn;

  btnDraw(button);

  //disable rit as well
  ritDisable();
  
  Button button2;
  getButton(BUTTON_RIT, &button2);
  btnDraw(&button2);
  
  displayRIT();
  displayVFO(VFO_A);
  displayVFO(VFO_B);
}

void vfoReset(){
  Button button;
  if (VFO_A == globalSettings.activeVfo)
    globalSettings.vfoB.frequency = globalSettings.vfoA.frequency;
  else
    globalSettings.vfoA.frequency = globalSettings.vfoB.frequency;

  if(globalSettings.splitOn){
    getButton(BUTTON_SPL, &button);
    splitToggle(&button);
  }

  if(globalSettings.ritOn){
    getButton(BUTTON_RIT, &button);
    ritToggle(&button);
  }
  
  displayVFO(VFO_A);
  displayVFO(VFO_B);

  saveVFOs();
}

void cwToggle(struct Button *b){
  if (TuningMode_e::TUNE_SSB == globalSettings.tuningMode){
    globalSettings.tuningMode = TuningMode_e::TUNE_CW;
  }
  else{
    globalSettings.tuningMode = TuningMode_e::TUNE_SSB;
  }

  setFrequency(GetActiveVfoFreq());
  btnDraw(b);
}

void sidebandToggle(Button* button){
  if(BUTTON_LSB == button->id){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
  }
  else{
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
  }
  setFrequency(GetActiveVfoFreq());

  struct Button button2;
  getButton(BUTTON_USB, &button2);
  btnDraw(&button2);
  getButton(BUTTON_LSB, &button2);
  btnDraw(&button2);

  saveVFOs();
}



void redrawVFOs(){
    struct Button button;
    ritDisable();
    getButton(BUTTON_RIT, &button);
    btnDraw(&button);
    displayRIT();
    displayVFO(VFO_A);
    displayVFO(VFO_B);

    //draw the lsb/usb buttons, the sidebands might have changed
    getButton(BUTTON_LSB, &button);
    btnDraw(&button);
    getButton(BUTTON_USB, &button);
    btnDraw(&button);
}


void switchBand(uint32_t bandfreq){

  //Serial.println(frequency);
  //Serial.println(bandfreq);
  uint32_t offset;
  uint32_t freq = GetActiveVfoFreq();
  if (3500000L <= freq && freq <= 4000000L)
    offset = freq - 3500000l;
  else if (24800000L <= freq && freq <= 25000000L)
    offset = freq - 24800000L;
  else 
    offset = freq % 1000000L;

  //Serial.println(offset);

  setFrequency(bandfreq + offset);

  if(bandfreq >= THRESHOLD_USB_LSB){
    SetActiveVfoMode(VfoMode_e::VFO_MODE_USB);
  }
  else{
    SetActiveVfoMode(VfoMode_e::VFO_MODE_LSB);
  }

  updateDisplay();
  saveVFOs();
}

void doCommand(Button* button){
  //Serial.print(F("Doing command: "));
  //Serial.print(button->text);
  //Serial.print(F(", Hitbox: "));
  //Serial.print(button->x);Serial.print(F(","));
  //Serial.print(button->y);Serial.print(F(";"));
  //Serial.print(button->x + button->w);Serial.print(F(","));
  //Serial.println(button->y + button->h);
  switch(button->id){
    case BUTTON_RIT:
    {
      ritToggle(button);
      break;
    }
    case BUTTON_LSB:
    case BUTTON_USB:
    {
      sidebandToggle(button);
      break;
    }
    case BUTTON_CW:
    {
      cwToggle(button);
      break;
    }
    case BUTTON_SPL:
    {
      splitToggle(button);
      break;
    }
    case BUTTON_VFOA:
    {
      switchVFO(VFO_A);
      uint32_t freq = globalSettings.vfoA.frequency;
      ltoa(freq,b,10);
      break;
    }
    case BUTTON_VFOB:
    {
      switchVFO(VFO_B);
      uint32_t freq = globalSettings.vfoB.frequency;
      ltoa(freq,b,10);
      break;
    }
    case BUTTON_80:
    {
      switchBand(3500000L);
      break;
    }
    case BUTTON_40:
    {
      switchBand(7000000L);
      break;
    }
    case BUTTON_30:
    {
      switchBand(10000000L);
      break;
    }
    case BUTTON_20:
    {
      switchBand(14000000L);
      break;
    }
    case BUTTON_17:
    {
      switchBand(18000000L);
      break;
    }
    case BUTTON_15:
    {
      switchBand(21000000L);
      break;
    }
    case BUTTON_10:
    {
      switchBand(28000000L);
      break;
    }
    case BUTTON_FRQ:
    {
      enterFreq();
      break;
    }
    case BUTTON_MNU:
    {
      doSetup2();
      break;
    }
    default:
    {
      //Serial.print(F("Unknown command: "));
      //Serial.println(button.id);
      break;
    }
  }
}

void  checkTouch(){
  if (!readTouch())
    return;

  while(readTouch())
    checkCAT();
  scaleTouch(&ts_point);
 
  /* //debug code
  Serial.print(ts_point.x); Serial.print(' ');Serial.println(ts_point.y);
  */
  for (int i = 0; i < BUTTON_TOTAL; i++){
    Button button;
    memcpy_P(&button, &(btn_set[i]), sizeof(Button));

    int x2 = button.x + button.w;
    int y2 = button.y + button.h;

    if(button.x < ts_point.x && ts_point.x < x2 && 
       button.y < ts_point.y && ts_point.y < y2)
      doCommand(&button);
  }
}

//returns true if the button is pressed
int btnDown(){
  if (digitalRead(FBUTTON) == HIGH)
    return 0;
  else
    return 1;
}


void drawFocus(int ibtn, int color){
  Button button;

  memcpy_P(&button, &(btn_set[ibtn]), sizeof(Button));
  displayRect(button.x, button.y, button.w, button.h, color);
}

void doCommands(){
  int select = 0;
  int prev_button = 0;

  //wait for the button to be raised up
  while(btnDown())
    active_delay(50);
  active_delay(50);  //debounce

  Button button;
  memcpy_P(&button, &(btn_set[select/10]), sizeof(Button));
 
  int8_t morse_status = 0;
  button.morse_status(&morse_status);
  if(morse_status){
    if(morse_status > 0){
      
    }
    else{
      
    }
  }

  while (true){
    //check if the knob's button was pressed
    if (btnDown()){
      doCommand(&button);

      //unfocus the buttons
      drawFocus(select, COLOR_INACTIVE_BORDER);
      if (VFO_A == globalSettings.activeVfo)
        drawFocus(BUTTON_VFOA, COLOR_ACTIVE_BORDER);
      else
        drawFocus(BUTTON_VFOB, COLOR_ACTIVE_BORDER);
        
      //wait for the button to be up and debounce
      while(btnDown())
        active_delay(100);
      active_delay(500);
      return;
    }

    int knob = enc_read();
    
    if (knob == 0){
      active_delay(50);
      continue;
    }
    
    if (knob > 0){
      if (select + knob < BUTTON_TOTAL * 10)
        select += knob;
    }
    if (knob < 0 && select + knob >= 0)
      select += knob;      //caught ya, i is already -ve here, so you add it
    
    if (prev_button == select / 10)
      continue;
    
    //we are on a new button
    drawFocus(prev_button, COLOR_INACTIVE_BORDER);
    drawFocus(select/10, COLOR_ACTIVE_BORDER);
    memcpy_P(&button, &(btn_set[select/10]), sizeof(Button));

    morse_status = 0;
    button.morse_status(&morse_status);
    Serial.println((uint16_t)button.morse_status);
    Serial.println(morse_status);
    if(morse_status){
      if(morse_status > 0){
       
      }
      else{

      }
    }
    enc_read();//Clear any rotation that was done while the letter was playing
    prev_button = select/10;
  }
//  guiUpdate();

  //debounce the button
  while(btnDown())
    active_delay(50);
  active_delay(50);

  checkCAT();
}
