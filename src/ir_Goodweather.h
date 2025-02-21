// Goodweather A/C
//
// Copyright 2019 ribeirodanielf
// Copyright 2019 David Conran

// Supports:
//   Brand: Goodweather,  Model: ZH/JT-03 remote

#ifndef IR_GOODWEATHER_H_
#define IR_GOODWEATHER_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/697

// Constants

// Timing
const uint16_t kGoodweatherBitMark = 640;
const uint16_t kGoodweatherOneSpace = 580;
const uint16_t kGoodweatherZeroSpace = 1600;
const uint16_t kGoodweatherHdrMark = 6800;
const uint16_t kGoodweatherHdrSpace = 6800;

// Masks
const uint8_t kGoodweatherBitLight = 8;
const uint64_t kGoodweatherLightMask = 0x1ULL << kGoodweatherBitLight;
const uint8_t kGoodweatherBitTurbo = kGoodweatherBitLight + 3;  // 11
const uint64_t kGoodweatherTurboMask = 0x1ULL << kGoodweatherBitTurbo;
const uint8_t kGoodweatherBitCommand = kGoodweatherBitTurbo + 5;  // 16
const uint64_t kGoodweatherCommandMask = 0xFULL << kGoodweatherBitCommand;
const uint8_t kGoodweatherBitSleep = kGoodweatherBitCommand + 8;  // 24
const uint64_t kGoodweatherSleepMask = 0x1ULL << kGoodweatherBitSleep;
const uint8_t kGoodweatherBitPower = kGoodweatherBitSleep + 1;  // 25
const uint64_t kGoodweatherPowerMask = 0x1ULL << kGoodweatherBitPower;
const uint8_t kGoodweatherBitSwing = kGoodweatherBitPower + 1;  // 26
const uint64_t kGoodweatherSwingMask = 0x3ULL << kGoodweatherBitSwing;
const uint8_t kGoodweatherBitFan = kGoodweatherBitSwing + 3;  // 29
const uint64_t kGoodweatherFanMask = 0x3ULL << kGoodweatherBitFan;
const uint8_t kGoodweatherBitTemp = kGoodweatherBitFan + 3;  // 32
const uint64_t kGoodweatherTempMask = 0xFULL << kGoodweatherBitTemp;
const uint8_t kGoodweatherBitMode = kGoodweatherBitTemp + 5;  // 37
const uint64_t kGoodweatherModeMask = 0x7ULL << kGoodweatherBitMode;

// Modes
const uint8_t kGoodweatherAuto = 0b000;
const uint8_t kGoodweatherCool = 0b001;
const uint8_t kGoodweatherDry =  0b010;
const uint8_t kGoodweatherFan =  0b011;
const uint8_t kGoodweatherHeat = 0b100;
const uint8_t kGoodweatherSwingFast = 0b00;
const uint8_t kGoodweatherSwingSlow = 0b01;
const uint8_t kGoodweatherSwingOff =  0b10;
// Fan Control
const uint8_t kGoodweatherFanAuto = 0b00;
const uint8_t kGoodweatherFanHigh = 0b01;
const uint8_t kGoodweatherFanMed =  0b10;
const uint8_t kGoodweatherFanLow =  0b11;
// Temperature
const uint8_t kGoodweatherTempMin = 16;  // Celsius
const uint8_t kGoodweatherTempMax = 31;  // Celsius
// Commands
const uint8_t kGoodweatherCmdPower    = 0x00;
const uint8_t kGoodweatherCmdMode     = 0x01;
const uint8_t kGoodweatherCmdUpTemp   = 0x02;
const uint8_t kGoodweatherCmdDownTemp = 0x03;
const uint8_t kGoodweatherCmdSwing    = 0x04;
const uint8_t kGoodweatherCmdFan      = 0x05;
const uint8_t kGoodweatherCmdTimer    = 0x06;
const uint8_t kGoodweatherCmdAirFlow  = 0x07;
const uint8_t kGoodweatherCmdHold     = 0x08;
const uint8_t kGoodweatherCmdSleep    = 0x09;
const uint8_t kGoodweatherCmdTurbo    = 0x0A;
const uint8_t kGoodweatherCmdLight    = 0x0B;


// Classes
class IRGoodweatherAc {
 public:
  explicit IRGoodweatherAc(uint16_t pin);

  void stateReset(void);
#if SEND_GOODWEATHER
  void send(const uint16_t repeat = kGoodweatherMinRepeat);
  uint8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_GOODWEATHER
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void);
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);
  void setFan(const uint8_t speed);
  uint8_t getFan(void);
  void setMode(const uint8_t mode);
  uint8_t getMode();
  void setSwing(const uint8_t speed);
  uint8_t getSwing(void);
  void setSleep(const bool toggle);
  bool getSleep(void);
  void setTurbo(const bool toggle);
  bool getTurbo(void);
  void setLight(const bool toggle);
  bool getLight(void);
  void setCommand(const uint8_t cmd);
  uint8_t getCommand(void);
  uint64_t getRaw(void);
  void setRaw(const uint64_t state);
  uint8_t convertMode(const stdAc::opmode_t mode);
  uint8_t convertFan(const stdAc::fanspeed_t speed);
  uint8_t convertSwingV(const stdAc::swingv_t swingv);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void);
  String toString();
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else
  IRsendTest _irsend;
#endif
  uint64_t remote;  // The state of the IR remote in IR code form.
};
#endif  // IR_GOODWEATHER_H_
