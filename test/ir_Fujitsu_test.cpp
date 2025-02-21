// Copyright 2017 Jonny Graham, David Conran

#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Fujitsu.h"
#include "gtest/gtest.h"

// Tests for Fujitsu A/C methods.

// Test sending typical data only.
TEST(TestIRFujitsuACClass, GetRawDefault) {
  IRFujitsuAC ac(0);  // AR-RAH2E
  ac.setSwing(kFujitsuAcSwingBoth);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);
  ac.setCmd(kFujitsuAcCmdTurnOn);
  uint8_t expected_arrah2e[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x01, 0x31, 0x00, 0x00, 0x00, 0x20, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Swing: Vert + Horiz, Command: N/A",
            ac.toString());

  uint8_t expected_ardb1[15] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x81, 0x01, 0x01, 0x00, 0x00, 0x00, 0x4D};
  ac.setModel(ARDB1);
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 15 * 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Command: N/A",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawTurnOff) {
  IRFujitsuAC ac(0);
  ac.setModel(ARRAH2E);
  ac.off();
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: Off, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Swing: Vert + Horiz, Command: N/A",
            ac.toString());

  ac.setModel(ARDB1);
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 6 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: Off, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Command: N/A",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawStepHoriz) {
  IRFujitsuAC ac(0);
  ac.stepHoriz();
  uint8_t expected[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x79, 0x86};
  EXPECT_STATE_EQ(expected, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 24C, "
      "Fan: 1 (HIGH), Swing: Vert + Horiz, Command: Step vane horizontally",
      ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawStepVert) {
  IRFujitsuAC ac(0);
  ac.setModel(ARRAH2E);
  ac.stepVert();
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x6C, 0x93};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Swing: Vert + Horiz, Command: Step vane vertically",
            ac.toString());

  ac.setModel(ARDB1);
  ac.stepVert();
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x6C};
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 6 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1,
            ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Command: Step vane vertically",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawWithSwingHoriz) {
  IRFujitsuAC ac(0);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingHoriz);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setTemp(25);
  uint8_t expected[16] = {0x14, 0x63, 0x0, 0x10, 0x10, 0xFE, 0x9, 0x30,
                          0x90, 0x1, 0x24, 0x0, 0x0, 0x0, 0x20, 0xFB};
  EXPECT_STATE_EQ(expected, ac.getRaw(), 16 * 8);
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 25C, "
            "Fan: 4 (QUIET), Swing: Horiz, Command: N/A",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawWithFan) {
  IRFujitsuAC ac(0);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingHoriz);
  ac.setMode(kFujitsuAcModeFan);
  ac.setFanSpeed(kFujitsuAcFanMed);
  ac.setTemp(20);  // temp doesn't matter for fan
                        // but it is sent by the RC anyway
  ac.setModel(ARRAH2E);
  uint8_t expected_arrah2e[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x40, 0x03, 0x22, 0x00, 0x00, 0x00, 0x20, 0x4B};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 3 (FAN), Temp: 20C, "
            "Fan: 2 (MED), Swing: Horiz, Command: N/A", ac.toString());

  ac.setModel(ARDB1);
  uint8_t expected_ardb1[15] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x40, 0x03, 0x02, 0x00, 0x00, 0x00, 0x8B};
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), ac.getStateLength() * 8);
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 3 (FAN), Temp: 20C, "
            "Fan: 2 (MED), Command: N/A", ac.toString());
}

TEST(TestIRFujitsuACClass, SetRaw) {
  IRFujitsuAC ac(0);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  uint8_t expected_default_arrah2e[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x01, 0x31, 0x00, 0x00, 0x00, 0x20, 0xFD};
  EXPECT_STATE_EQ(expected_default_arrah2e, ac.getRaw(),
                  ac.getStateLength() * 8);
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 24C, "
            "Fan: 1 (HIGH), Swing: Vert + Horiz, Command: N/A",
            ac.toString());
  // Now set a new state via setRaw();
  // This state is a real state from an AR-DB1 remote.
  uint8_t new_state1[kFujitsuAcStateLength - 1] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
    0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x9F};
  ac.setRaw(new_state1, kFujitsuAcStateLength - 1);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_STATE_EQ(new_state1, ac.getRaw(), ac.getStateLength() * 8);
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 19C, "
            "Fan: 0 (AUTO), Command: N/A", ac.toString());
}

TEST(TestSendFujitsuAC, GenerateMessage) {
  IRFujitsuAC ac(0);
  IRsendTest irsend(0);
  ac.begin();
  irsend.begin();

  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingBoth);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);

  EXPECT_EQ(kFujitsuAcFanHigh, ac.getFanSpeed());
  EXPECT_EQ(kFujitsuAcModeCool, ac.getMode());
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdStayOn, ac.getCmd());

  irsend.reset();
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLength);
  EXPECT_EQ(
  "f38000d50"
  "m3324s1574"
  "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
  "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
  "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
  "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s1182"
  "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s390"
  "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
  "m448s8100",
  irsend.outputStr());
}

TEST(TestSendFujitsuAC, GenerateShortMessage) {
  IRFujitsuAC ac(0);
  IRsendTest irsend(0);
  ac.begin();
  irsend.begin();

  ac.off();

  EXPECT_EQ(kFujitsuAcCmdTurnOff, ac.getCmd());

  irsend.reset();
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLengthShort);
  EXPECT_EQ(
  "f38000d50"
  "m3324s1574m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448"
  "s390m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s1182m448"
  "s1182m448s1182m448s1182m448s1182m448s1182m448s8100",
  irsend.outputStr());
}

// Issue #275
TEST(TestSendFujitsuAC, Issue275) {
  IRFujitsuAC ac(0);
  IRsendTest irsend(0);
  ac.begin();
  irsend.begin();
  irsend.reset();

  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLengthShort);
  EXPECT_EQ(
      "f38000d50"
      // Header
      "m3324s1574"
      //  0       0       1        0       1        0       0       0     (0x28)
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      //  1        1        0       0       0       1        1        0   (0xC6)
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      //  0       0       0       0       0       0       0       0       (0x00)
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      //  0       1        0       0       0       0       0       0      (0x40)
      "m448s390m448s1182m448s390m448s390m448s390m448s390m448s390m448s390"
      //  1        0       1        1        1        1        1        1 (0xBF)
      "m448s1182m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      // Footer
      "m448s8100", irsend.outputStr());

  irsend.reset();
  // Per report in Issue #275
  uint16_t off[115] = {
      3350, 1650,
      450, 400, 450, 450, 450, 1250, 450, 400, 450, 1250, 450, 400, 450, 400,
      450, 400, 450, 1250, 450, 1250, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 400, 450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 400, 450, 1250, 450, 1250, 450, 1250, 450, 1250, 450, 1250,
      450, 1250, 450};
  irsend.sendRaw(off, 115, 38);
  EXPECT_EQ(
      "f38000d50"
      // Header
      "m3350s1650"
      //  0       0       1        0       1        0       0       0     (0x28)
      "m450s400m450s450m450s1250m450s400m450s1250m450s400m450s400m450s400"
      //  1        1        0       0       0       1        1        0   (0xC6)
      "m450s1250m450s1250m450s400m450s400m450s400m450s1250m450s1250m450s400"
      //  0       0       0       0       0       0       0       0       (0x00)
      "m450s400m450s400m450s400m450s400m450s400m450s400m450s400m450s400"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m450s400m450s400m450s400m450s400m450s1250m450s400m450s400m450s400"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m450s400m450s400m450s400m450s400m450s1250m450s400m450s400m450s400"
      //  0       1        0       0       0       0       0       0      (0x40)
      "m450s400m450s1250m450s400m450s400m450s400m450s400m450s400m450s400"
      //  1        0       1        1        1        1        1        1 (0xBF)
      "m450s1250m450s400m450s1250m450s1250m450s1250m450s1250m450s1250m450s1250"
      // Footer
      "m450",
      irsend.outputStr());
}

TEST(TestDecodeFujitsuAC, SyntheticShortMessages) {
  IRsendTest irsend(0);
  IRFujitsuAC ac(0);
  IRrecv irrecv(0);

  irsend.begin();
  irsend.reset();

  ac.setModel(ARRAH2E);
  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits + 8, irsend.capture.bits);
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, irsend.capture.state, irsend.capture.bits);

  irsend.reset();

  ac.setModel(ARDB1);
  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits, irsend.capture.bits);
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected_ardb1, irsend.capture.state, irsend.capture.bits);
}

TEST(TestDecodeFujitsuAC, SyntheticLongMessages) {
  IRsendTest irsend(0);
  IRFujitsuAC ac(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();

  ac.setModel(ARRAH2E);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingVert);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setTemp(18);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  ASSERT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeFujitsuAC(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  uint8_t expected_arrah2e[kFujitsuAcStateLength] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
    0x20, 0x01, 0x14, 0x00, 0x00, 0x00, 0x20, 0x7B};
  EXPECT_STATE_EQ(expected_arrah2e, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 18C, "
            "Fan: 4 (QUIET), Swing: Vert, Command: N/A", ac.toString());

  irsend.reset();

  ac.setModel(ARDB1);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected_ardb1[kFujitsuAcStateLength - 1] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
    0x20, 0x01, 0x04, 0x00, 0x00, 0x00, 0xAB};
  EXPECT_STATE_EQ(expected_ardb1, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 18C, "
            "Fan: 4 (QUIET), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, RealShortARDB1OffExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  irsend.begin();

  irsend.reset();
  // "Off" Message recorded from an AR-DB1 remote.
  uint16_t rawData[99] = {
      3310, 1636,  440, 386,  440, 394,  442, 1210,  442, 390,  414, 1220,
      444, 390,  446, 380,  446, 380,  436, 1216,  438, 1214,  438, 388,
      438, 386,  438, 396,  410, 1222,  440, 1220,  442, 384,  442, 384,
      442, 384,  442, 382,  444, 382,  442, 382,  444, 380,  446, 380,
      446, 380,  444, 380,  436, 390,  436, 388,  436, 388,  438, 1214,
      438, 386,  438, 388,  438, 386,  440, 386,  440, 384,  442, 384,
      442, 384,  442, 1210,  444, 382,  444, 382,  444, 382,  444, 380,
      446, 1206,  436, 390,  436, 388,  436, 388,  438, 388,  438, 396,
      420, 388,  436};
  irsend.sendRaw(rawData, 99, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits, irsend.capture.bits);
  uint8_t expected[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: Off, Mode: 0 (AUTO), Temp: 16C, "
            "Fan: 0 (AUTO), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, RealLongARDB1Example) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  irsend.begin();
  irsend.reset();
  uint16_t rawData1[243] = {
      3316, 1632,  444, 390,  438, 388,  436, 1216,  438, 388,  438, 1214,
      438, 388,  438, 386,  440, 386,  440, 1212,  440, 1210,  442, 392,
      412, 396,  442, 392,  444, 1208,  444, 1208,  444, 380,  444, 380,
      446, 380,  436, 390,  436, 390,  436, 390,  436, 388,  438, 388,
      438, 388,  438, 388,  438, 386,  438, 386,  440, 384,  440, 1210,
      442, 384,  442, 382,  442, 384,  442, 384,  442, 382,  442, 382,
      444, 382,  444, 1208,  444, 382,  444, 380,  446, 380,  436, 390,
      436, 390,  436, 1214,  438, 1214,  438, 1212,  440, 1212,  440, 1220,
      412, 1222,  440, 394,  442, 382,  442, 382,  444, 1208,  444, 382,
      444, 380,  446, 380,  446, 380,  434, 390,  436, 388,  438, 388,
      438, 388,  438, 1214,  438, 1212,  440, 386,  440, 394,  412, 1222,
      440, 394,  442, 384,  442, 384,  442, 382,  442, 1208,  444, 390,
      414, 394,  442, 1216,  446, 380,  436, 390,  436, 390,  436, 388,
      436, 390,  436, 388,  438, 386,  440, 386,  440, 386,  438, 1212,
      440, 386,  440, 384,  440, 384,  442, 392,  412, 396,  440, 394,
      442, 382,  444, 382,  444, 382,  444, 380,  444, 380,  444, 382,
      444, 380,  446, 380,  436, 388,  436, 390,  436, 388,  438, 388,
      438, 388,  438, 388,  438, 386,  440, 386,  440, 386,  442, 384,
      440, 386,  442, 384,  440, 384,  442, 384,  442, 382,  442, 382,
      444, 1208,  444, 382,  444, 1208,  444, 380,  446, 1206,  436, 390,
      436, 1216,  436};
  irsend.sendRaw(rawData1, 243, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected1[kFujitsuAcStateLength - 1] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x21, 0x01, 0x04, 0x00, 0x00, 0x00, 0xAA};
  EXPECT_STATE_EQ(expected1, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 18C, "
            "Fan: 4 (QUIET), Command: N/A", ac.toString());

  irsend.reset();
  uint16_t rawData2[243] = {
      3316, 1630,  436, 398,  438, 386,  438, 1212,  440, 384,  440, 1212,
      442, 384,  442, 392,  414, 394,  442, 1218,  446, 1206,  436, 390,
      436, 388,  438, 388,  438, 1214,  440, 1212,  440, 384,  442, 384,
      442, 384,  442, 382,  444, 382,  444, 382,  444, 380,  446, 380,
      444, 380,  436, 390,  436, 388,  438, 396,  418, 388,  438, 1232,
      410, 396,  440, 394,  442, 384,  442, 384,  442, 382,  442, 392,
      414, 392,  444, 1216,  446, 380,  436, 390,  436, 396,  418, 390,
      436, 398,  438, 1214,  440, 1212,  440, 1210,  442, 1208,  444, 1216,
      416, 1218,  444, 388,  436, 390,  436, 388,  438, 1214,  440, 386,
      438, 386,  440, 386,  440, 384,  442, 384,  442, 384,  442, 382,
      444, 382,  444, 1206,  446, 1206,  436, 390,  436, 388,  438, 388,
      438, 386,  440, 394,  410, 396,  440, 1220,  442, 1210,  442, 392,
      414, 394,  442, 1218,  446, 406,  410, 388,  436, 390,  436, 390,
      436, 388,  438, 386,  440, 386,  440, 386,  440, 386,  440, 384,
      442, 384,  442, 384,  442, 382,  444, 382,  444, 380,  446, 380,
      446, 380,  436, 390,  436, 390,  436, 388,  438, 386,  438, 388,
      438, 386,  440, 386,  440, 384,  442, 384,  442, 384,  442, 384,
      442, 382,  444, 382,  444, 380,  446, 380,  446, 380,  436, 390,
      436, 388,  436, 388,  438, 386,  438, 386,  440, 386,  440, 1212,
      440, 1210,  442, 1210,  442, 1208,  444, 1208,  436, 390,  436, 388,
      436, 1214,  440};
  irsend.sendRaw(rawData2, 243, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected2[kFujitsuAcStateLength - 1] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x9F};
  EXPECT_STATE_EQ(expected2, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Power: On, Mode: 1 (COOL), Temp: 19C, "
            "Fan: 0 (AUTO), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, Issue414) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  // Capture as supplied by arpmota
  uint16_t rawData[259] = {3352, 1574, 480, 350, 480, 346, 480, 1190, 458, 346,
      508, 1140, 480, 346, 506, 346, 458, 346, 480, 1168, 480, 1192, 452, 374,
      458, 346, 480, 346, 508, 1168, 480, 1140, 480, 346, 506, 346, 458, 346,
      480, 346, 480, 346, 480, 346, 484, 372, 454, 374, 456, 346, 508, 318,
      480, 374, 458, 374, 480, 318, 480, 1196, 452, 346, 480, 346, 484, 342,
      484, 346, 480, 374, 458, 346, 506, 318, 508, 1170, 452, 346, 480, 374,
      458, 346, 506, 318, 480, 1196, 452, 1190, 458, 1162, 480, 1196, 452,
      1170, 480, 1190, 458, 1164, 480, 1196, 480, 318, 508, 346, 456, 1192,
      480, 346, 456, 374, 452, 346, 480, 374, 458, 342, 484, 346, 508, 346,
      456, 342, 512, 1164, 458, 1164, 508, 346, 456, 346, 480, 1190, 456, 342,
      484, 346, 506, 346, 456, 374, 452, 346, 508, 346, 458, 1164, 508, 346,
      458, 374, 452, 1168, 480, 374, 480, 318, 480, 374, 456, 346, 508, 318,
      480, 346, 484, 374, 480, 318, 484, 342, 484, 374, 480, 318, 484, 342,
      484, 346, 508, 318, 508, 346, 458, 346, 506, 318, 480, 374, 458, 346,
      506, 318, 480, 346, 484, 374, 480, 318, 482, 372, 456, 346, 508, 318,
      506, 348, 456, 342, 484, 346, 508, 318, 484, 374, 480, 318, 508, 318,
      484, 346, 508, 318, 480, 374, 456, 346, 508, 346, 480, 318, 480, 346,
      484, 374, 480, 320, 484, 1164, 508, 346, 458, 342, 512, 1164, 458, 1190,
      454, 346, 484, 1164, 508, 346, 458, 1164, 480, 350, 480, 374, 480};
  uint8_t state[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30, 0x81, 0x04, 0x00, 0x00,
      0x00, 0x00, 0x20, 0x2B};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 259, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(state, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 4 (HEAT), Temp: 24C, "
            "Fan: 0 (AUTO), Swing: Off, Command: N/A", ac.toString());

  // Resend it using the state this time.
  irsend.reset();
  irsend.sendFujitsuAC(state, 16);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(state, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "f38000d50"
      "m3324s1574"
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s1182"
      "m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s390"
      "m448s1182m448s1182m448s390m448s1182m448s390m448s1182m448s390m448s390"
      "m448s8100", irsend.outputStr());
}

TEST(TestIRFujitsuACClass, toCommon) {
  IRFujitsuAC ac(0);
  ac.setMode(kFujitsuAcModeCool);
  ac.setTemp(20);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setSwing(kFujitsuAcSwingBoth);

  // Now test it.
  ASSERT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  ASSERT_TRUE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(20, ac.toCommon().degrees);
  ASSERT_TRUE(ac.toCommon().quiet);

  ASSERT_EQ(stdAc::opmode_t::kCool, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kMin, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kAuto, ac.toCommon().swingv);
  ASSERT_EQ(stdAc::swingh_t::kAuto, ac.toCommon().swingh);
  // Unsupported.
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().beep);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);

  // Check off mode which is special.
  ac.off();
  ASSERT_FALSE(ac.toCommon().power);
  ac.send();
  ac.stateReset();
  IRrecv irrecv(0);
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ac.setRaw(ac._irsend.capture.state, ac._irsend.capture.bits / 8);

  // Now test it.
  EXPECT_EQ(    // Off mode technically has no temp, mode, fan, etc.
      "Model: 1 (ARRAH2E), Power: Off, Mode: 0 (AUTO), Temp: 16C, "
      "Fan: 0 (AUTO), Swing: Off, Command: N/A", ac.toString());
  ASSERT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  ASSERT_FALSE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(16, ac.toCommon().degrees);
  ASSERT_FALSE(ac.toCommon().quiet);

  ASSERT_EQ(stdAc::opmode_t::kAuto, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kAuto, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kOff, ac.toCommon().swingv);
  ASSERT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  // Unsupported.
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().beep);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);
}

TEST(TestDecodeFujitsuAC, Issue716) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  // Powerful command from a raw data capture.
  // Capture as supplied by u4mzu4
  uint16_t rawData[115] = {
      3320, 1610, 432, 406, 432, 406, 432, 1220, 432, 406, 432, 1192, 458, 406,
      432, 406, 432, 406, 432, 1218, 432, 1220, 432, 406, 432, 406, 432, 406,
      432, 1192, 458, 1192, 460, 406, 432, 406, 432, 406, 432, 406, 432, 406,
      432, 406, 432, 406, 432, 408, 432, 406, 432, 406, 430, 406, 432, 406, 432,
      406, 432, 1190, 460, 406, 432, 408, 430, 406, 432, 406, 432, 406, 432,
      406, 432, 406, 434, 1192, 458, 406, 432, 406, 432, 406, 432, 1194, 458,
      406, 432, 406, 432, 1194, 456, 1196, 454, 1220, 432, 406, 432, 406, 432,
      408, 430, 1194, 458, 1194, 456, 406, 432, 406, 430, 406, 432, 1194, 458,
      1194, 458};  // FUJITSU_AC
  uint8_t powerful[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0x39, 0xC6};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 115, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLengthShort * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(powerful, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 3 (ARREB1E), Power: On, Mode: 0 (AUTO), Temp: 16C, "
            "Fan: 0 (AUTO), Swing: Off, Command: Powerful, Outside Quiet: Off",
            ac.toString());

  // Economy (just from the state)
  uint8_t econo[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0x09, 0xF6};
  // Make sure we can't accidentally inherit the correct model.
  ASSERT_NE(fujitsu_ac_remote_model_t::ARDB1,
            fujitsu_ac_remote_model_t::ARREB1E);
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(econo, kFujitsuAcStateLengthShort);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 3 (ARREB1E), Power: On, Mode: 0 (AUTO), Temp: 16C, "
            "Fan: 0 (AUTO), Swing: Off, Command: Economy, Outside Quiet: Off",
            ac.toString());
}

TEST(TestIRFujitsuACClass, OutsideQuiet) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  ASSERT_NE(fujitsu_ac_remote_model_t::ARDB1,
    fujitsu_ac_remote_model_t::ARREB1E);
  ASSERT_NE(fujitsu_ac_remote_model_t::ARRAH2E,
    fujitsu_ac_remote_model_t::ARREB1E);
  // States as supplied by u4mzu4
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/716#issuecomment-495852309
  uint8_t off[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x2F};
  uint8_t on[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xAF};
  // Make sure we can't accidentally inherit the correct model.
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(off, kFujitsuAcStateLength);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_FALSE(ac.getOutsideQuiet());
  // We can really only tell the difference between ARRAH2E & ARREB1E if
  // the option is set. Otheriwse they appear the same.
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Power: On, Mode: 1 (COOL), Temp: 24C, "
      "Fan: 0 (AUTO), Swing: Off, Command: N/A", ac.toString());
  ac.setModel(fujitsu_ac_remote_model_t::ARREB1E);
  EXPECT_EQ(
      "Model: 3 (ARREB1E), Power: On, Mode: 1 (COOL), Temp: 24C, "
      "Fan: 0 (AUTO), Swing: Off, Command: N/A, Outside Quiet: Off",
      ac.toString());

  // Make sure we can't accidentally inherit the correct model.
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(on, kFujitsuAcStateLength);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_TRUE(ac.getOutsideQuiet());
  EXPECT_EQ(
      "Model: 3 (ARREB1E), Power: On, Mode: 1 (COOL), Temp: 24C, "
      "Fan: 0 (AUTO), Swing: Off, Command: N/A, Outside Quiet: On",
      ac.toString());

  ac.setOutsideQuiet(false);
  EXPECT_FALSE(ac.getOutsideQuiet());
  ac.setOutsideQuiet(true);
  EXPECT_TRUE(ac.getOutsideQuiet());
  ac.setOutsideQuiet(false);
  EXPECT_FALSE(ac.getOutsideQuiet());
}

TEST(TestIRFujitsuACClass, toggleSwing) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  ac.begin();
  ac.setModel(ARJW2);
  ac.setSwing(kFujitsuAcSwingOff);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ASSERT_EQ(kFujitsuAcSwingOff, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingHoriz, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingOff, ac.getSwing());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingVert, ac.getSwing());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingOff, ac.getSwing());

  // Both
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingVert, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());

  EXPECT_EQ(
      "Model: 4 (ARJW2), Power: On, Mode: 1 (COOL), Temp: 24C, Fan: 1 (HIGH), "
      "Command: Toggle horizontal swing",
      ac.toString());

  // Test without the update set.
  ac.toggleSwingHoriz(false);
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  ac.toggleSwingVert(false);
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
}

TEST(TestDecodeFujitsuAC, Issue726) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRFujitsuAC ac(0);

  // fan:auto mode:auto temp:24 power：on
  // Capture as supplied by huexpub
  // Rawdata was very messy. Had to use `./auto_analyse_raw_data.py -r 250` to
  // get it to parse due to timings being above tolerances.
  uint8_t auto_auto_on_24[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x2F};
  irsend.begin();
  irsend.reset();
  irsend.sendFujitsuAC(auto_auto_on_24, 16);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(auto_auto_on_24, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Power: On, Mode: 0 (AUTO), Temp: 24C, "
            "Fan: 0 (AUTO), Swing: Off, Command: N/A",
            ac.toString());
}
