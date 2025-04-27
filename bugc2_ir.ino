/**
 * @file ir_nec_rx.ino
 * @author chou shisei
 * @brief
 * @version 0.1
 * @date 2024-10-6
 *
 *
 * @Hardwares: M5StickC + Hat BugC2
 * @Platform Version: Arduino M5Stack Board Manager v2.1.1
 * @Dependent Library:
 * M5HatBugC: https://github.com/m5stack/M5Hat-BugC
 * IRremote: https://github.com/Arduino-IRremote/Arduino-IRremote
 */

#include "M5HatBugC.h"
#include "M5StickCPlus2.h"
#include <IRremote.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

typedef struct
{
  uint32_t order;
  int8_t speed;
  unsigned long duration;
} action;

M5HatBugC bugc;

unsigned long gobal_duration = 200;
int8_t gobal_speed = 10;
int8_t gobal_max_speed = 30;
int8_t gobal_turbo_speed = 80;

int vol = 0;
bool puro_mode = false;
bool turbo_mode = false;
vector<action> user_actions;

void actions_recorder(uint32_t order_for_record, int8_t speed_for_record, unsigned long duration_for_record)
{
  action now;
  now.duration = order_for_record;
  now.speed = speed_for_record;
  now.order = duration_for_record;
  user_actions.push_back(now);

  Serial.println("Recorded Action.");
}

void run(uint32_t order_for_run, uint8_t speed_for_run, unsigned long duration_for_run)
{
  switch (order_for_run)
  {

  case 0xE619FF00:
    bugc.move(MOVE_FORWARD, speed_for_run);
    break;
  case 0xE31CFF00:
    bugc.move(MOVE_BACKWARD, speed_for_run);
    break;
  case 0xF30CFF00:
    bugc.move(MOVE_LEFT, speed_for_run);
    break;
  case 0xA15EFF00:
    bugc.move(MOVE_RIGHT, speed_for_run);
    break;
  case 0xE718FF00:
    bugc.move(MOVE_ROTATE, speed_for_run);
    break;
  case 0xB847FF00:
    bugc.move(MOVE_REVERSE_ROTATE, speed_for_run);
    break;

  case 0xB649FF00: // F1
    bugc.setMotorSpeed(0, speed_for_run);
    break;
  case 0xB24DFF00: // F2
    bugc.setMotorSpeed(1, speed_for_run);
    break;
  case 0xAA55FF00: // F3
    bugc.setMotorSpeed(2, speed_for_run);
    break;
  case 0xA659FF00: // F4
    bugc.setMotorSpeed(3, speed_for_run);
    break;
  }
  delay(duration_for_run);
  bugc.setAllMotorSpeed(0, 0, 0, 0);
}

void actions_play()
{
  Serial.println("func -> actions play");
  // for (int i = 0; i < user_actions.size(); i++)
  // {
  //   Serial.println("One action played(1)");
  //   auto temp = user_actions.at(i);
  //   Serial.println("One action played(2)");
  //   run(temp.order, temp.speed, temp.duration);
  //   Serial.println("One action played(3)");
  //   delay(100);
  // }
}

void empty_actions()
{
  user_actions.clear();
  Serial.println("I erased all the action.");
}

void order_manager(uint32_t order)
{
  vector<uint32_t> movement = {0xB649FF00, 0xB24DFF00, 0xAA55FF00, 0xA659FF00, 0xE619FF00, 0xE31CFF00, 0xF30CFF00, 0xA15EFF00, 0xE718FF00, 0xB847FF00};

  /*
    Manage something other than movement, e.g., speed
    動き以外のものを管理する、例えば速度
  */
  switch (order)
  {
  case 0xB946FF00: // SETUP
    if (puro_mode)
    {
      puro_mode = false;
    }
    else
    {
      puro_mode = true;
    }
    break;
  case 0xAE51FF00:
    if (turbo_mode)
    {
      turbo_mode = false;
      StickCP2.Display.setTextColor(YELLOW);
    }
    else
    {
      turbo_mode = true;
      StickCP2.Display.setTextColor(RED);
    }

  case 0xFB04FF00: // 1
    if (!puro_mode)
    {
      Serial.println("Actions play");
      actions_play();
    }
    break;

  case 0xFA05FF00: // 2
    if (!puro_mode)
    {
      empty_actions();
    }
    break;
  case 0xBB44FF00: //<<
    if (!(gobal_speed <= 10))
    {
      gobal_speed -= 5;
    }
    break;
  case 0xBC43FF00: //>>
    if (turbo_mode)
    {
      if (!(gobal_speed >= gobal_turbo_speed))
      {
        gobal_speed += 5;
      }
    }else {
      if (!(gobal_speed >= gobal_max_speed))
      {
        gobal_speed += 5;
      }
    }
    break;
  case 0xBA45FF00: // POWER
    if (gobal_duration > 100)
    {
      gobal_duration -= 100;
    }
    break;
  case 0xAD52FF00: // SOURCE
    if (gobal_duration < 3000)
    {
      gobal_duration += 100;
    }
    break;
  }

  if (puro_mode)
  {
    bool found = find(movement.begin(), movement.end(), order) != movement.end();
    if (found)
    {
      actions_recorder(order, gobal_speed, gobal_duration);
    }
  }
  else
  {
    run(order, gobal_speed, gobal_duration);
  }

  StickCP2.Speaker.tone(10000, 100);
  delay(100);
  StickCP2.Speaker.tone(4000, 20);
  delay(100);
}

void setup()
{
  Serial.begin(115200);
  while (!bugc.begin(&Wire, BUGC_DEFAULT_I2C_ADDR, 0, 26, 400000U))
  {
    Serial.println("Couldn't find BugC");
    delay(1000);
  }
  bugc.setAllMotorSpeed(0, 0, 0, 0);
  bugc.setAllLedColor(0xff0000, 0x0000ff);

  // only BugC2 support
  IrReceiver.begin(BUGC2_IR_RX_PIN, ENABLE_LED_FEEDBACK);

  auto cfg = M5.config();
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setTextDatum(middle_center);
  StickCP2.Display.setTextFont(&fonts::Orbitron_Light_24);
  StickCP2.Display.setTextSize(1);
  // int textsize = StickCP2.Display.height() / 60;
  // if (textsize == 0)
  // {
  //   textsize = 1;
  // }
  // StickCP2.Display.setTextSize(textsize);
  vol = StickCP2.Power.getBatteryVoltage();
}

void loop()
{
  /*
   * Check if received data is available and if yes, try to decode it.
   * Decoded result is in the IrReceiver.decodedIRData structure.
   *
   * E.g. command is in IrReceiver.decodedIRData.command
   * address is in command is in IrReceiver.decodedIRData.address
   * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
   *
   * At 115200 baud, printing takes 40 ms for NEC protocol and 10 ms for NEC
   * repeat
   */
  if (IrReceiver.decode())
  {
    uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;

    // if (receivedCode != 0x00)
    // {
    //   Serial.printf("Decoded IR signal: 0x%X\n", IrReceiver.decodedIRData.decodedRawData);
    // }
    if (receivedCode != 0x00)
    {
      order_manager(receivedCode);
    }
    IrReceiver.resume();
    vol = StickCP2.Power.getBatteryVoltage();
  }
  else if (IrReceiver.isIdle())
  {
    StickCP2.Display.clear();
    StickCP2.Display.setCursor(10, 20);
    StickCP2.Display.printf("speed: %d", gobal_speed);
    StickCP2.Display.setCursor(10, 50);
    StickCP2.Display.printf("time: %lu", gobal_duration);
    StickCP2.Display.setCursor(10, 80);
    StickCP2.Display.printf("bat: %d", vol - 3000);
    StickCP2.Display.setCursor(10, 110);
    if (puro_mode)
    {
      StickCP2.Display.printf("puro: ON");
    }
    else
    {
      StickCP2.Display.printf("puro: OFF");
    }
  }
}
