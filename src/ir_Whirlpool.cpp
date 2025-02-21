// Copyright 2018 David Conran
//
// Code to emulate Whirlpool protocol compatible devices.
// Should be compatible with:
// * SPIS409L, SPIS412L, SPIW409L, SPIW412L, SPIW418L
// Remotes:
// * DG11J1-3A / DG11J1-04
// * DG11J1-91
//
// Note: Smart, iFeel, AroundU, PowerSave, & Silent modes are unsupported.
//       Advanced 6thSense, Dehumidify, & Sleep modes are not supported.
//       FYI:
//         Dim == !Light
//         Jet == Super == Turbo
//

#include "ir_Whirlpool.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
// Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/509
const uint16_t kWhirlpoolAcHdrMark = 8950;
const uint16_t kWhirlpoolAcHdrSpace = 4484;
const uint16_t kWhirlpoolAcBitMark = 597;
const uint16_t kWhirlpoolAcOneSpace = 1649;
const uint16_t kWhirlpoolAcZeroSpace = 533;
const uint16_t kWhirlpoolAcGap = 7920;
const uint32_t kWhirlpoolAcMinGap = kDefaultMessageGap;  // Just a guess.
const uint8_t kWhirlpoolAcSections = 3;

#if SEND_WHIRLPOOL_AC
// Send a Whirlpool A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kWhirlpoolAcStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/509
void IRsend::sendWhirlpoolAC(const unsigned char data[], const uint16_t nbytes,
                             const uint16_t repeat) {
  if (nbytes < kWhirlpoolAcStateLength)
    return;  // Not enough bytes to send a proper message.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Section 1
    sendGeneric(kWhirlpoolAcHdrMark, kWhirlpoolAcHdrSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcOneSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark, kWhirlpoolAcGap,
                data, 6,  // 6 bytes == 48 bits
                38000,    // Complete guess of the modulation frequency.
                false, 0, 50);
    // Section 2
    sendGeneric(0, 0, kWhirlpoolAcBitMark, kWhirlpoolAcOneSpace,
                kWhirlpoolAcBitMark, kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcGap, data + 6, 8,  // 8 bytes == 64 bits
                38000,  // Complete guess of the modulation frequency.
                false, 0, 50);
    // Section 3
    sendGeneric(0, 0, kWhirlpoolAcBitMark, kWhirlpoolAcOneSpace,
                kWhirlpoolAcBitMark, kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcMinGap, data + 14, 7,  // 7 bytes == 56 bits
                38000,  // Complete guess of the modulation frequency.
                false, 0, 50);
  }
}
#endif  // SEND_WHIRLPOOL_AC

// Class for emulating a Whirlpool A/C remote.
// Decoding help from:
//   @redmusicxd, @josh929800, @raducostea

IRWhirlpoolAc::IRWhirlpoolAc(const uint16_t pin) : _irsend(pin) {
  this->stateReset();
}

void IRWhirlpoolAc::stateReset(void) {
  for (uint8_t i = 2; i < kWhirlpoolAcStateLength; i++) remote_state[i] = 0x0;
  remote_state[0] = 0x83;
  remote_state[1] = 0x06;
  remote_state[6] = 0x80;
  this->_setTemp(kWhirlpoolAcAutoTemp);  // Default to a sane value.
}

void IRWhirlpoolAc::begin(void) { _irsend.begin(); }

bool IRWhirlpoolAc::validChecksum(uint8_t state[], const uint16_t length) {
  if (length > kWhirlpoolAcChecksumByte1 &&
      state[kWhirlpoolAcChecksumByte1] !=
          xorBytes(state + 2, kWhirlpoolAcChecksumByte1 - 1 - 2)) {
    DPRINTLN("DEBUG: First Whirlpool AC checksum failed.");
    return false;
  }
  if (length > kWhirlpoolAcChecksumByte2 &&
      state[kWhirlpoolAcChecksumByte2] !=
          xorBytes(state + kWhirlpoolAcChecksumByte1 + 1,
                   kWhirlpoolAcChecksumByte2 - kWhirlpoolAcChecksumByte1 - 1)) {
    DPRINTLN("DEBUG: Second Whirlpool AC checksum failed.");
    return false;
  }
  // State is too short to have a checksum or everything checked out.
  return true;
}

// Update the checksum for the internal state.
void IRWhirlpoolAc::checksum(uint16_t length) {
  if (length >= kWhirlpoolAcChecksumByte1)
    remote_state[kWhirlpoolAcChecksumByte1] =
        xorBytes(remote_state + 2, kWhirlpoolAcChecksumByte1 - 1 - 2);
  if (length >= kWhirlpoolAcChecksumByte2)
    remote_state[kWhirlpoolAcChecksumByte2] =
        xorBytes(remote_state + kWhirlpoolAcChecksumByte1 + 1,
                 kWhirlpoolAcChecksumByte2 - kWhirlpoolAcChecksumByte1 - 1);
}

#if SEND_WHIRLPOOL_AC
void IRWhirlpoolAc::send(const uint16_t repeat, const bool calcchecksum) {
  if (calcchecksum) this->checksum();
  _irsend.sendWhirlpoolAC(remote_state, kWhirlpoolAcStateLength, repeat);
}
#endif  // SEND_WHIRLPOOL_AC

uint8_t *IRWhirlpoolAc::getRaw(const bool calcchecksum) {
  if (calcchecksum) this->checksum();
  return remote_state;
}

void IRWhirlpoolAc::setRaw(const uint8_t new_code[], const uint16_t length) {
  for (uint8_t i = 0; i < length && i < kWhirlpoolAcStateLength; i++)
    remote_state[i] = new_code[i];
}

whirlpool_ac_remote_model_t IRWhirlpoolAc::getModel(void) {
  if (remote_state[kWhirlpoolAcAltTempPos] & kWhirlpoolAcAltTempMask)
    return DG11J191;
  else
    return DG11J13A;
}

void IRWhirlpoolAc::setModel(const whirlpool_ac_remote_model_t model) {
  switch (model) {
    case DG11J191:
      remote_state[kWhirlpoolAcAltTempPos] |= kWhirlpoolAcAltTempMask;
      break;
    case DG11J13A:
      // FALL THRU
    default:
      remote_state[kWhirlpoolAcAltTempPos] &= ~kWhirlpoolAcAltTempMask;
  }
  this->_setTemp(_desiredtemp);  // Different models have different temp values.
}

// Return the temp. offset in deg C for the current model.
int8_t IRWhirlpoolAc::getTempOffset(void) {
  switch (this->getModel()) {
    case whirlpool_ac_remote_model_t::DG11J191:
      return -2;
      break;
    default:
      return 0;
  }
}

// Set the temp. in deg C
void IRWhirlpoolAc::_setTemp(const uint8_t temp, const bool remember) {
  if (remember) _desiredtemp = temp;
  int8_t offset = this->getTempOffset();  // Cache the min temp for the model.
  uint8_t newtemp = std::max((uint8_t)(kWhirlpoolAcMinTemp + offset), temp);
  newtemp = std::min((uint8_t)(kWhirlpoolAcMaxTemp + offset), newtemp);
  remote_state[kWhirlpoolAcTempPos] =
      (remote_state[kWhirlpoolAcTempPos] & ~kWhirlpoolAcTempMask) |
      ((newtemp - (kWhirlpoolAcMinTemp + offset)) << 4);
}

// Set the temp. in deg C
void IRWhirlpoolAc::setTemp(const uint8_t temp) {
  this->_setTemp(temp);
  this->setSuper(false);  // Changing temp cancels Super/Jet mode.
  this->setCommand(kWhirlpoolAcCommandTemp);
}

// Return the set temp. in deg C
uint8_t IRWhirlpoolAc::getTemp(void) {
  return ((remote_state[kWhirlpoolAcTempPos] & kWhirlpoolAcTempMask) >> 4) +
         + kWhirlpoolAcMinTemp + this->getTempOffset();
}

void IRWhirlpoolAc::_setMode(const uint8_t mode) {
  switch (mode) {
    case kWhirlpoolAcAuto:
      this->setFan(kWhirlpoolAcFanAuto);
      this->_setTemp(kWhirlpoolAcAutoTemp, false);
      this->setSleep(false);  // Cancel sleep mode when in auto/6thsense mode.
      // FALL THRU
    case kWhirlpoolAcHeat:
    case kWhirlpoolAcCool:
    case kWhirlpoolAcDry:
    case kWhirlpoolAcFan:
      remote_state[kWhirlpoolAcModePos] &= ~kWhirlpoolAcModeMask;
      remote_state[kWhirlpoolAcModePos] |= mode;
      this->setCommand(kWhirlpoolAcCommandMode);
      break;
    default:
      return;
  }
  if (mode == kWhirlpoolAcAuto) this->setCommand(kWhirlpoolAcCommand6thSense);
}

void IRWhirlpoolAc::setMode(const uint8_t mode) {
    this->setSuper(false);  // Changing mode cancels Super/Jet mode.
    this->_setMode(mode);
}

uint8_t IRWhirlpoolAc::getMode(void) {
  return remote_state[kWhirlpoolAcModePos] & kWhirlpoolAcModeMask;
}

void IRWhirlpoolAc::setFan(const uint8_t speed) {
  switch (speed) {
    case kWhirlpoolAcFanAuto:
    case kWhirlpoolAcFanLow:
    case kWhirlpoolAcFanMedium:
    case kWhirlpoolAcFanHigh:
      remote_state[kWhirlpoolAcFanPos] =
          (remote_state[kWhirlpoolAcFanPos] & ~kWhirlpoolAcFanMask) | speed;
      this->setSuper(false);  // Changing fan speed cancels Super/Jet mode.
      this->setCommand(kWhirlpoolAcCommandFanSpeed);
      break;
  }
}

uint8_t IRWhirlpoolAc::getFan(void) {
  return remote_state[kWhirlpoolAcFanPos] & kWhirlpoolAcFanMask;
}

void IRWhirlpoolAc::setSwing(const bool on) {
  if (on) {
    remote_state[kWhirlpoolAcFanPos] |= kWhirlpoolAcSwing1Mask;
    remote_state[kWhirlpoolAcOffTimerPos] |= kWhirlpoolAcSwing2Mask;
  } else {
    remote_state[kWhirlpoolAcFanPos] &= ~kWhirlpoolAcSwing1Mask;
    remote_state[kWhirlpoolAcOffTimerPos] &= ~kWhirlpoolAcSwing2Mask;
  }
  setCommand(kWhirlpoolAcCommandSwing);
}

bool IRWhirlpoolAc::getSwing(void) {
  return (remote_state[kWhirlpoolAcFanPos] & kWhirlpoolAcSwing1Mask) &&
         (remote_state[kWhirlpoolAcOffTimerPos] & kWhirlpoolAcSwing2Mask);
}

void IRWhirlpoolAc::setLight(const bool on) {
  if (on)
    remote_state[kWhirlpoolAcClockPos] &= ~kWhirlpoolAcLightMask;
  else
    remote_state[kWhirlpoolAcClockPos] |= kWhirlpoolAcLightMask;
}

bool IRWhirlpoolAc::getLight(void) {
  return !(remote_state[kWhirlpoolAcClockPos] & kWhirlpoolAcLightMask);
}

void IRWhirlpoolAc::setTime(const uint16_t pos,
                            const uint16_t minspastmidnight) {
  // Hours
  remote_state[pos] &= ~kWhirlpoolAcHourMask;
  remote_state[pos] |= (minspastmidnight / 60) % 24;
  // Minutes
  remote_state[pos + 1] &= ~kWhirlpoolAcMinuteMask;
  remote_state[pos + 1] |= minspastmidnight % 60;
}

uint16_t IRWhirlpoolAc::getTime(const uint16_t pos) {
  return (remote_state[pos] & kWhirlpoolAcHourMask) * 60 +
         (remote_state[pos + 1] & kWhirlpoolAcMinuteMask);
}

bool IRWhirlpoolAc::isTimerEnabled(const uint16_t pos) {
  return remote_state[pos - 1] & kWhirlpoolAcTimerEnableMask;
}

void IRWhirlpoolAc::enableTimer(const uint16_t pos, const bool on) {
  if (on)
    remote_state[pos - 1] |= kWhirlpoolAcTimerEnableMask;
  else
    remote_state[pos - 1] &= ~kWhirlpoolAcTimerEnableMask;
}

void IRWhirlpoolAc::setClock(const uint16_t minspastmidnight) {
  this->setTime(kWhirlpoolAcClockPos, minspastmidnight);
}

uint16_t IRWhirlpoolAc::getClock(void) {
  return this->getTime(kWhirlpoolAcClockPos);
}

void IRWhirlpoolAc::setOffTimer(const uint16_t minspastmidnight) {
  this->setTime(kWhirlpoolAcOffTimerPos, minspastmidnight);
}

uint16_t IRWhirlpoolAc::getOffTimer(void) {
  return this->getTime(kWhirlpoolAcOffTimerPos);
}

bool IRWhirlpoolAc::isOffTimerEnabled(void) {
  return this->isTimerEnabled(kWhirlpoolAcOffTimerPos);
}

void IRWhirlpoolAc::enableOffTimer(const bool on) {
  this->enableTimer(kWhirlpoolAcOffTimerPos, on);
  this->setCommand(kWhirlpoolAcCommandOffTimer);
}

void IRWhirlpoolAc::setOnTimer(const uint16_t minspastmidnight) {
  this->setTime(kWhirlpoolAcOnTimerPos, minspastmidnight);
}

uint16_t IRWhirlpoolAc::getOnTimer(void) {
  return this->getTime(kWhirlpoolAcOnTimerPos);
}

bool IRWhirlpoolAc::isOnTimerEnabled(void) {
  return this->isTimerEnabled(kWhirlpoolAcOnTimerPos);
}

void IRWhirlpoolAc::enableOnTimer(const bool on) {
  this->enableTimer(kWhirlpoolAcOnTimerPos, on);
  this->setCommand(kWhirlpoolAcCommandOnTimer);
}

void IRWhirlpoolAc::setPowerToggle(const bool on) {
  if (on)
    remote_state[kWhirlpoolAcPowerTogglePos] |= kWhirlpoolAcPowerToggleMask;
  else
    remote_state[kWhirlpoolAcPowerTogglePos] &= ~kWhirlpoolAcPowerToggleMask;
  this->setSuper(false);  // Changing power cancels Super/Jet mode.
  this->setCommand(kWhirlpoolAcCommandPower);
}

bool IRWhirlpoolAc::getPowerToggle(void) {
  return remote_state[kWhirlpoolAcPowerTogglePos] & kWhirlpoolAcPowerToggleMask;
}

uint8_t IRWhirlpoolAc::getCommand(void) {
  return remote_state[kWhirlpoolAcCommandPos];
}

void IRWhirlpoolAc::setSleep(const bool on) {
  if (on) {
    remote_state[kWhirlpoolAcSleepPos] |= kWhirlpoolAcSleepMask;
    this->setFan(kWhirlpoolAcFanLow);
  } else {
    remote_state[kWhirlpoolAcSleepPos] &= ~kWhirlpoolAcSleepMask;
  }
  this->setCommand(kWhirlpoolAcCommandSleep);
}

bool IRWhirlpoolAc::getSleep(void) {
  return remote_state[kWhirlpoolAcSleepPos] & kWhirlpoolAcSleepMask;
}

// AKA Jet/Turbo mode.
void IRWhirlpoolAc::setSuper(const bool on) {
  if (on) {
    this->setFan(kWhirlpoolAcFanHigh);
    switch (this->getMode()) {
      case kWhirlpoolAcHeat:
        this->setTemp(kWhirlpoolAcMaxTemp + this->getTempOffset());
        break;
      case kWhirlpoolAcCool:
      default:
        this->setTemp(kWhirlpoolAcMinTemp + this->getTempOffset());
        this->setMode(kWhirlpoolAcCool);
        break;
    }
    remote_state[kWhirlpoolAcSuperPos] |= kWhirlpoolAcSuperMask;
  } else {
    remote_state[kWhirlpoolAcSuperPos] &= ~kWhirlpoolAcSuperMask;
  }
  this->setCommand(kWhirlpoolAcCommandSuper);
}

bool IRWhirlpoolAc::getSuper(void) {
  return remote_state[kWhirlpoolAcSuperPos] & kWhirlpoolAcSuperMask;
}

void IRWhirlpoolAc::setCommand(const uint8_t code) {
  remote_state[kWhirlpoolAcCommandPos] = code;
}

// Convert a standard A/C mode into its native mode.
uint8_t IRWhirlpoolAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kWhirlpoolAcCool;
    case stdAc::opmode_t::kHeat:
      return kWhirlpoolAcHeat;
    case stdAc::opmode_t::kDry:
      return kWhirlpoolAcDry;
    case stdAc::opmode_t::kFan:
      return kWhirlpoolAcFan;
    default:
      return kWhirlpoolAcAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRWhirlpoolAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kWhirlpoolAcFanLow;
    case stdAc::fanspeed_t::kMedium:
      return kWhirlpoolAcFanMedium;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kWhirlpoolAcFanHigh;
    default:
      return kWhirlpoolAcFanAuto;
  }
}

// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRWhirlpoolAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kWhirlpoolAcCool: return stdAc::opmode_t::kCool;
    case kWhirlpoolAcHeat: return stdAc::opmode_t::kHeat;
    case kWhirlpoolAcDry: return stdAc::opmode_t::kDry;
    case kWhirlpoolAcFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRWhirlpoolAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kWhirlpoolAcFanHigh: return stdAc::fanspeed_t::kMax;
    case kWhirlpoolAcFanMedium: return stdAc::fanspeed_t::kMedium;
    case kWhirlpoolAcFanLow: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRWhirlpoolAc::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::WHIRLPOOL_AC;
  result.model = this->getModel();
  result.power = this->getPowerToggle();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  result.swingv = this->getSwing() ? stdAc::swingv_t::kAuto :
                                     stdAc::swingv_t::kOff;
  result.turbo = this->getSuper();
  result.light = this->getLight();
  result.sleep = this->getSleep() ? 0 : -1;
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.filter = false;
  result.econo = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

String IRWhirlpoolAc::timeToString(const uint16_t minspastmidnight) {
  String result = "";
  uint8_t hours = minspastmidnight / 60;
  if (hours < 10) result += '0';
  result += uint64ToString(hours);
  result += ':';
  uint8_t mins = minspastmidnight % 60;
  if (mins < 10) result += '0';
  result += uint64ToString(mins);
  return result;
}

// Convert the internal state into a human readable string.
String IRWhirlpoolAc::toString(void) {
  String result = "";
  result.reserve(200);  // Reserve some heap for the string to reduce fragging.
  result += F("Model: ");
  result += uint64ToString(this->getModel());
  switch (this->getModel()) {
    case DG11J191:
      result += F(" (DG11J191)");
      break;
    case DG11J13A:
      result += F(" (DG11J13A)");
      break;
    default:
      result += F(" (UNKNOWN)");
  }
  result += IRutils::acBoolToString(getPowerToggle(), F("Power toggle"));
  result += IRutils::acModeToString(getMode(), kWhirlpoolAcAuto,
                                    kWhirlpoolAcCool, kWhirlpoolAcHeat,
                                    kWhirlpoolAcDry, kWhirlpoolAcFan);
  result += F(", Temp: ");
  result += uint64ToString(this->getTemp());
  result += F("C, Fan: ");
  result += uint64ToString(this->getFan());
  switch (getFan()) {
    case kWhirlpoolAcFanAuto:
      result += F(" (AUTO)");
      break;
    case kWhirlpoolAcFanHigh:
      result += F(" (HIGH)");
      break;
    case kWhirlpoolAcFanMedium:
      result += F(" (MEDIUM)");
      break;
    case kWhirlpoolAcFanLow:
      result += F(" (LOW)");
      break;
    default:
      result += F(" (UNKNOWN)");
      break;
  }
  result += IRutils::acBoolToString(getSwing(), F("Swing"));
  result += IRutils::acBoolToString(getLight(), F("Light"));
  result += F(", Clock: ");
  result += this->timeToString(this->getClock());
  result += F(", On Timer: ");
  if (this->isOnTimerEnabled())
    result += this->timeToString(this->getOnTimer());
  else
    result += F("Off");
  result += F(", Off Timer: ");
  if (this->isOffTimerEnabled())
    result += this->timeToString(this->getOffTimer());
  else
    result += F("Off");
  result += IRutils::acBoolToString(getSleep(), F("Sleep"));
  result += IRutils::acBoolToString(getSuper(), F("Super"));
  result += F(", Command: ");
  result += uint64ToString(this->getCommand());
  switch (this->getCommand()) {
    case kWhirlpoolAcCommandLight:
      result += F(" (LIGHT)");
      break;
    case kWhirlpoolAcCommandPower:
      result += F(" (POWER)");
      break;
    case kWhirlpoolAcCommandTemp:
      result += F(" (TEMP)");
      break;
    case kWhirlpoolAcCommandSleep:
      result += F(" (SLEEP)");
      break;
    case kWhirlpoolAcCommandSuper:
      result += F(" (SUPER)");
      break;
    case kWhirlpoolAcCommandOnTimer:
      result += F(" (ONTIMER)");
      break;
    case kWhirlpoolAcCommandMode:
      result += F(" (MODE)");
      break;
    case kWhirlpoolAcCommandSwing:
      result += F(" (SWING)");
      break;
    case kWhirlpoolAcCommandIFeel:
      result += F(" (IFEEL)");
      break;
    case kWhirlpoolAcCommandFanSpeed:
      result += F(" (FANSPEED)");
      break;
    case kWhirlpoolAcCommand6thSense:
      result += F(" (6THSENSE)");
      break;
    case kWhirlpoolAcCommandOffTimer:
      result += F(" (OFFTIMER)");
      break;
    default:
      result += F(" (UNKNOWN)");
      break;
  }
  return result;
}

#if DECODE_WHIRLPOOL_AC
// Decode the supplied Whirlpool A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kWhirlpoolAcBits
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Working as intended.
//
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/509
bool IRrecv::decodeWhirlpoolAC(decode_results *results, const uint16_t nbits,
                               const bool strict) {
  if (results->rawlen < 2 * nbits + 4 + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid Whirlpool A/C message.
  if (strict) {
    if (nbits != kWhirlpoolAcBits) return false;
  }

  uint16_t offset = kStartOffset;
  uint8_t sectionSize[kWhirlpoolAcSections] = {6, 8, 7};

  // Header
  if (!matchMark(results->rawbuf[offset++], kWhirlpoolAcHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kWhirlpoolAcHdrSpace))
    return false;

  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kWhirlpoolAcSections;
       section++) {
    uint16_t used;
    // Section Data
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, sectionSize[section] * 8,
                        0, 0,
                        kWhirlpoolAcBitMark, kWhirlpoolAcOneSpace,
                        kWhirlpoolAcBitMark, kWhirlpoolAcZeroSpace,
                        kWhirlpoolAcBitMark, kWhirlpoolAcGap,
                        section >= kWhirlpoolAcSections - 1,
                        kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += sectionSize[section];
  }

  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != nbits) return false;
    if (!IRWhirlpoolAc::validChecksum(results->state, nbits / 8))
      return false;
  }

  // Success
  results->decode_type = WHIRLPOOL_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // WHIRLPOOL_AC
