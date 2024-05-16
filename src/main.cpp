#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Rotary.h>
#include <DFRobotDFPlayerMini.h>
#include <EEPROM.h>

#define SWITCH_OFF 1
#define SWITCH_ON 0

#define LOOP_ALL 0
#define LOOP_FOLDER 1
#define LOOP_ONE 2

void printDetail(uint8_t type, int value);

// LiquidCrystal_I2C型変数の宣言
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27のアドレス,16列2行のLCDを使用

DFRobotDFPlayerMini dfp;

Rotary r1 = Rotary(3, 4);
const int SW1_PIN = 5;
Rotary r2 = Rotary(6, 7);
const int SW2_PIN = 8;
Rotary r3 = Rotary(9, 10);
const int SW3_PIN = 11;

int sw1_state = SWITCH_OFF;
int sw2_state = SWITCH_OFF;
int sw3_state = SWITCH_OFF;

int slide1 = 0;
int slide2 = 0;

// int folder = 1;
// int file = 1;

int totalFile[99];
int totalFolder;

// int loopState;

// int eq = 0;

struct Status
{
  int folder;
  int file;
  int eq;
  int loop;
};
Status status = {1, 1, 0, LOOP_ALL};

void saveStatus()
{
  EEPROM.put(0, status);
}

void loadStatus()
{
  EEPROM.get(0, status);
}

void showPlayFile(int folder, int file)
{
  lcd.setCursor(0, 1);
  if (folder < 10)
  {
    lcd.print("0");
  }
  lcd.print(folder);

  lcd.print("-");

  if (file < 100)
  {
    lcd.print("0");
  }
  if (file < 10)
  {
    lcd.print("0");
  }
  lcd.print(file);
}

void playFile()
{
  Serial.print(F("playFolder:"));
  Serial.print(F(" Folder:"));
  Serial.print(status.folder);
  Serial.print(F(" File:"));
  Serial.println(status.file);
  dfp.playFolder(status.folder, status.file);
  Serial.println(F("playFolder after"));
  showPlayFile(status.folder, status.file);
  saveStatus();

  delay(200);
}

void previous()
{
  if (status.file > 1)
  {
    status.file--;
  }
  else
  {
    status.file = totalFile[status.folder - 1];
  }
  playFile();
}

void next()
{
  if (totalFile[status.folder - 1] > status.file)
  {
    status.file++;
  }
  else
  {
    status.file = 1;
  }
  playFile();
}

void previousFolder()
{
  if (status.folder > 1)
  {
    status.folder--;
  }
  else
  {
    status.folder = totalFolder;
  }
  status.file = 1;
  playFile();
}

void nextFolder()
{
  if (totalFolder > status.folder)
  {
    status.folder++;
  }
  else
  {
    status.folder = 1;
  }
  status.file = 1;

  playFile();
}

void nextAuto()
{
  if (status.loop != LOOP_ONE)
  {
    if (totalFile[status.folder - 1] > status.file)
    {
      status.file++;
    }
    else
    {
      if (status.loop == LOOP_ALL)
      {
        nextFolder();
        return;
      }
      else
      {
        status.file = 1;
      }
    }
  }
  playFile();
}

void setEQ()
{
  dfp.EQ(status.eq);
  lcd.setCursor(0, 0);
  switch (status.eq)
  {
  case DFPLAYER_EQ_NORMAL:
    lcd.print("NORMAL    ");
    break;
  case DFPLAYER_EQ_POP:
    lcd.print("POP       ");
    break;
  case DFPLAYER_EQ_ROCK:
    lcd.print("ROCK      ");
    break;
  case DFPLAYER_EQ_JAZZ:
    lcd.print("JAZZ      ");
    break;
  case DFPLAYER_EQ_CLASSIC:
    lcd.print("CLASSIC   ");
    break;
  case DFPLAYER_EQ_BASS:
    lcd.print("BASS      ");
    break;
  }
}

void previousEQ()
{
  if (status.eq > 0)
  {
    status.eq--;
  }
  else
  {
    status.eq = 5;
  }
  setEQ();
}

void nextEQ()
{
  if (5 > status.eq)
  {
    status.eq++;
  }
  else
  {
    status.eq = 0;
  }
  setEQ();
}

void showLoop()
{
  lcd.setCursor(10, 1);
  switch (status.loop)
  {
  case LOOP_ONE:
    Serial.println(F("Loop: ONE"));
    lcd.print("ONE   ");
    break;
  case LOOP_FOLDER:
    Serial.println(F("Loop: FOLDER"));
    lcd.print("FOLDER");
    break;
  case LOOP_ALL:
    Serial.println(F("Loop: ALL"));
    lcd.print("ALL   ");
    break;
  default:
    break;
  }
}

void changeLoop()
{
  Serial.print(F("Loop:"));
  Serial.print(status.loop);
  Serial.print(F(" to "));
  switch (status.loop)
  {
  case -1:
  case LOOP_ONE:
    status.loop = LOOP_FOLDER;
    break;
  case LOOP_FOLDER:
    status.loop = LOOP_ALL;
    break;
  case LOOP_ALL:
    status.loop = LOOP_ONE;
    break;
  default:
    break;
  }
  Serial.println(status.loop);
  showLoop();
}

void setVolume()
{
  int sl = analogRead(A7) * 31 / 1024;
  if (sl != slide1)
  {
    if (slide2 != sl)
    {
      Serial.print(F("Volume:"));
      Serial.println(sl);
      slide2 = slide1;
      slide1 = sl;

      lcd.setCursor(10, 0); // カーソルの位置を指定
      lcd.print("Vol:");    // 文字の表示
      if (sl < 10)
      {
        lcd.print("0");
      }
      dfp.volume(sl);
      lcd.print(sl); // 文字の表示
      Serial.println("Volume end");    // 文字の表示

      delay(100);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  lcd.init();      // LCDの初期化
  lcd.backlight(); // LCDバックライトの点灯
  lcd.clear();

  lcd.setCursor(0, 0);     // カーソルの位置を指定
  lcd.print("MP3 Player"); // 文字の表示

  lcd.setCursor(0, 1); // カーソルの位置を指定
  lcd.print("(^_^)/"); // 文字の表示

  delay(2000); // DFPlayerが使える状態になるまで待つ必要あり

  Serial1.begin(9600);
  if (!dfp.begin(Serial1))
  {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));

    lcd.setCursor(0, 1);         // カーソルの位置を指定
    lcd.print("Insert SD card"); // 文字の表示
    while (true)
      ;
  }

  dfp.setTimeOut(1000);
  totalFolder = dfp.readFolderCounts();
  if (totalFolder < 0) // フォルダの情報が取れるまで何回か繰り返しが必要
  {
    totalFolder = dfp.readFolderCounts();
    if (totalFolder < 0)
    {
      totalFolder = dfp.readFolderCounts();
      if (totalFolder < 0)
      {
        totalFolder = dfp.readFolderCounts();
      }
    }
  }
  Serial.print("Total Folder:");
  Serial.println(totalFolder);
  totalFolder--; // 暫定
  for (int i = 0; i < totalFolder; i++)
  {
    totalFile[i] = -1;
    // totalFile[i] = dfp.readFileCountsInFolder(i + 1);
    // Serial.print(F("Folder:"));
    // Serial.print(i + 1);
    // Serial.print(F(" File:"));
    // Serial.println(totalFile[i]);
    // lcd.setCursor(0, 1);
    // lcd.print(i + 1);
    // lcd.setCursor(0, 1);
  }
  dfp.setTimeOut(500);

  // EEPROM
  loadStatus();

  if (totalFolder < status.folder)
  {
    status.folder = 1;
    status.file = 1;
  }

  showLoop();

  setVolume();

  lcd.setCursor(0, 0);
  lcd.print("Play      ");
  playFile();
}

void loop()
{
  // 曲数判定
  if (totalFile[status.folder - 1] <= 1)
  {
    int count = dfp.readFileCountsInFolder(status.folder);
    if (count > 0)
    {
      totalFile[status.folder - 1] = count;
      Serial.print(F("Folder:"));
      Serial.print(status.folder);
      Serial.print(F(" File:"));
      Serial.println(totalFile[status.folder - 1]);
    }
  }

  // 次、前
  unsigned char rotary1 = r1.process();
  if (rotary1)
  {
    if (rotary1 == DIR_CW)
    {
      Serial.println("Previous File");
      previous();
    }
    else
    {
      Serial.println("Next File");
      next();
    }
  }

  // 次、前 フォルダ
  unsigned char rotary2 = r2.process();
  if (rotary2)
  {
    if (rotary2 == DIR_CW)
    {
      Serial.println("Previous DIR");
      previousFolder();
    }
    else
    {
      Serial.println("Next DIR");
      nextFolder();
    }
  }

  // EQ
  unsigned char rotary3 = r3.process();
  if (rotary3)
  {
    if (rotary3 == DIR_CW)
    {
      Serial.println("Previous EQ");
      previousEQ();
    }
    else
    {
      Serial.println("Next EQ");
      nextEQ();
    }
  }

  // 開始、一時停止
  int sw1 = digitalRead(SW1_PIN);
  if (sw1 != sw1_state)
  {
    sw1_state = sw1;
    if (sw1 == SWITCH_ON)
    {
      Serial.println("Pause / Start");
      Serial.println(dfp.readState());
      int state = dfp.readState();
      if (state == 2)
      {
        dfp.start();
        lcd.setCursor(0, 0);
        lcd.print("Play      ");
      }
      else
      {
        dfp.pause();
        lcd.setCursor(0, 0);
        lcd.print("Pause     ");
      }
    }
    delay(10);
  }

  // Loop
  int sw2 = digitalRead(SW2_PIN);
  if (sw2 != sw2_state)
  {
    sw2_state = sw2;
    if (sw2 == SWITCH_ON)
    {
      Serial.println("Loop");
      changeLoop();
    }
    delay(10);
  }

  // None
  int sw3 = digitalRead(SW3_PIN);
  if (sw3 != sw3_state)
  {
    sw3_state = sw3;
    if (sw3 == SWITCH_ON)
    {
      Serial.println("Reset");
      dfp.begin(Serial1);
      playFile();
    }
    delay(10);
  }

  // Volume
  setVolume();

  if (dfp.available())
  {
    uint8_t type = dfp.readType();
    int value = dfp.read();
    printDetail(type, value); // Print the detail message from DFPlayer to handle different errors and states.

    switch (type)
    {
    case DFPlayerPlayFinished:
      // 曲が終わったら次に進む
      nextAuto();
      break;
    default:
      break;
    }
  }
}

void printDetail(uint8_t type, int value)
{
  switch (type)
  {
  case TimeOut:
    Serial.println(F("Time Out!"));
    break;
  case WrongStack:
    Serial.println(F("Stack Wrong!"));
    break;
  case DFPlayerCardInserted:
    Serial.println(F("Card Inserted!"));
    break;
  case DFPlayerCardRemoved:
    Serial.println(F("Card Removed!"));
    break;
  case DFPlayerCardOnline:
    Serial.println(F("Card Online!"));
    setup();
    break;
  case DFPlayerPlayFinished:
    Serial.print(F("Number:"));
    Serial.print(value);
    Serial.println(F(" Play Finished!"));
    break;
  case DFPlayerError:
    Serial.print(F("DFPlayerError:"));
    switch (value)
    {
    case Busy:
      Serial.println(F("Card not found"));
      break;
    case Sleeping:
      Serial.println(F("Sleeping"));
      break;
    case SerialWrongStack:
      Serial.println(F("Get Wrong Stack"));
      break;
    case CheckSumNotMatch:
      Serial.println(F("Check Sum Not Match"));
      break;
    case FileIndexOut:
      Serial.println(F("File Index Out of Bound"));
      break;
    case FileMismatch:
      Serial.println(F("Cannot Find File"));
      break;
    case Advertise:
      Serial.println(F("In Advertise"));
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}
