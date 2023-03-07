#pragma once
//
//    FILE: PCA9634.h
//  AUTHOR: Rob Tillaart
//    DATE: 2022-01-03
// VERSION: 0.2.6
// PURPOSE: Arduino library for PCA9634 I2C LED driver, 8 channel
//     URL: https://github.com/RobTillaart/PCA9634


#include "Arduino.h"
#include "Wire.h"


#define PCA9634_LIB_VERSION         (F("0.2.6"))

#define PCA9634_MODE1               0x00
#define PCA9634_MODE2               0x01

//  0x80 bit ==> Auto-Increment for all registers.
//               used in writeN() - see issue #9
#define PCA9634_PWM(x)              (0x82+(x))

#define PCA9634_GRPPWM              0x0A
#define PCA9634_GRPFREQ             0x0B

//  check datasheet for details
#define PCA9634_LEDOUT_BASE         0x0C    //  0x0C..0x0D
#define PCA9634_LEDOFF              0x00    //  default @ startup
#define PCA9634_LEDON               0x01
#define PCA9634_LEDPWM              0x02
#define PCA9634_LEDGRPPWM           0x03

//  Error codes
#define PCA9634_OK                  0x00
#define PCA9634_ERROR               0xFF
#define PCA9634_ERR_WRITE           0xFE
#define PCA9634_ERR_CHAN            0xFD
#define PCA9634_ERR_MODE            0xFC
#define PCA9634_ERR_REG             0xFB
#define PCA9634_ERR_I2C             0xFA


//  Configuration bits MODE1 register
#define PCA9634_MODE1_AUTOINCR2     0x80  //  ReadOnly,  0 = disable  1 = enable
#define PCA9634_MODE1_AUTOINCR1     0x40  //  ReadOnly,  bit1
#define PCA9634_MODE1_AUTOINCR0     0x20  //  ReadOnly,  bit0
#define PCA9634_MODE1_SLEEP         0x10  //  0 = normal       1 = sleep
#define PCA9634_MODE1_SUB1          0x08  //  0 = disable      1 = enable
#define PCA9634_MODE1_SUB2          0x04  //  0 = disable      1 = enable
#define PCA9634_MODE1_SUB3          0x02  //  0 = disable      1 = enable
#define PCA9634_MODE1_ALLCALL       0x01  //  0 = disable      1 = enable
#define PCA9634_MODE1_NONE          0x00

//  Configuration bits MODE2 register
#define PCA9634_MODE2_BLINK         0x20  //  0 = dim          1 = blink
#define PCA9634_MODE2_INVERT        0x10  //  0 = normal       1 = inverted
#define PCA9634_MODE2_ACK           0x08  //  0 = on STOP      1 = on ACK
#define PCA9634_MODE2_TOTEMPOLE     0x04  //  0 = open drain   1 = totem-pole
#define PCA9634_MODE2_NONE          0x00

//  Registers in which the ALLCALL and sub-addresses are stored
#define PCA9634_SUBADR(x)           (0x0D +(x))  // x = 1..3
#define PCA9634_ALLCALLADR          0x11

//  Standard ALLCALL and sub-addresses --> only work for write commands and NOT for read commands
#define PCA9634_ALLCALL             0x70            //  TDS of chip says 0xE0, however,
                                                    //  in this library the LSB is added during the write command
                                                    //                 (0xE0 --> 0b11100000, 0x70 --> 0b1110000)
#define PCA9634_SUB1                0x71            //  see line above (0xE2 --> 0x71)
#define PCA9634_SUB2                0x72            //  see line above (0xE4 --> 0x72)
#define PCA9634_SUB3                0x74            //  see line above (0xE8 --> 0x74)


class PCA9634
{
public:
  explicit PCA9634(const uint8_t deviceAddress, TwoWire *wire = &Wire);

#if defined (ESP8266) || defined(ESP32)
  bool     begin(int sda, int scl,
                     uint8_t mode1_mask = PCA9634_MODE1_ALLCALL,
                     uint8_t mode2_mask = PCA9634_MODE2_NONE);
#endif
  bool     begin(uint8_t mode1_mask = PCA9634_MODE1_ALLCALL,
                 uint8_t mode2_mask = PCA9634_MODE2_NONE);
  bool     isConnected();


  /////////////////////////////////////////////////////
  //
  //  CONFIGURATION
  //
  uint8_t  configure(uint8_t mode1_mask, uint8_t mode2_mask);
  uint8_t  channelCount();

  uint8_t  setLedDriverMode(uint8_t channel, uint8_t mode);
  uint8_t  getLedDriverMode(uint8_t channel);

  //  reg = 1, 2  check datasheet for values
  uint8_t  writeMode(uint8_t reg, uint8_t value);
  uint8_t  readMode(uint8_t reg);
  //  convenience wrappers
  uint8_t  setMode1(uint8_t value) { return writeMode(PCA9634_MODE1, value); };
  uint8_t  setMode2(uint8_t value) { return writeMode(PCA9634_MODE2, value); };
  uint8_t  getMode1()              { return readMode(PCA9634_MODE1); };
  uint8_t  getMode2()              { return readMode(PCA9634_MODE2); };

  //  TODO PWM also in %% ?
  void     setGroupPWM(uint8_t value) { writeReg(PCA9634_GRPPWM, value); };
  uint8_t  getGroupPWM() { return readReg(PCA9634_GRPPWM); };

  //  TODO set time in milliseconds and round to nearest value?
  void     setGroupFREQ(uint8_t value) { writeReg(PCA9634_GRPFREQ, value); };
  uint8_t  getGroupFREQ() { return readReg(PCA9634_GRPFREQ); };


  /////////////////////////////////////////////////////
  //
  //  WRITE
  //
  //  single PWM setting
  uint8_t  write1(uint8_t channel, uint8_t value);

  //  RGB setting, write three consecutive PWM registers
  uint8_t  write3(uint8_t channel, uint8_t R, uint8_t G, uint8_t B);

  //  generic worker, write N consecutive PWM registers
  uint8_t  writeN(uint8_t channel, uint8_t* arr, uint8_t count);

  //  generic worker, write N consecutive PWM registers without Stop command
  uint8_t  writeN_noStop(uint8_t channel, uint8_t* arr, uint8_t count);

  //  write stop command to end transmission
  uint8_t  writeStop();


  /////////////////////////////////////////////////////
  //
  //  ERROR
  //
  //  note error flag is reset after read!
  int      lastError();


  /////////////////////////////////////////////////////
  //
  //  SUB CALL  -  ALL CALL  (since 0.2.0)
  //
  //  nr = { 1, 2, 3 }
  bool     enableSubCall(uint8_t nr);
  bool     disableSubCall(uint8_t nr);
  bool     isEnabledSubCall(uint8_t nr);
  bool     setSubCallAddress(uint8_t nr, uint8_t address);
  uint8_t  getSubCallAddress(uint8_t nr);

  bool     enableAllCall();
  bool     disableAllCall();
  bool     isEnabledAllCall();
  bool     setAllCallAddress(uint8_t address);
  uint8_t  getAllCallAddress();


  /////////////////////////////////////////////////////
  //
  //  OE - Output Enable control
  //
  bool     setOutputEnablePin(uint8_t pin);
  bool     setOutputEnable(bool on);
  uint8_t  getOutputEnable();


  //  EXPERIMENTAL 0.2.2
  int I2C_SoftwareReset(uint8_t method);


private:
  //  DIRECT CONTROL
  uint8_t  writeReg(uint8_t reg, uint8_t value);  //  returns error status.
  uint8_t  readReg(uint8_t reg);

  uint8_t  _address;
  uint8_t  _register;
  uint8_t  _data;
  int      _error;
  uint8_t  _channelCount = 8;
  uint8_t  _OutputEnablePin;

  TwoWire*  _wire;
};


//  -- END OF FILE --

