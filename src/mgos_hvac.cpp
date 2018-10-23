
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include "mgos.h"
//#include "mgos_pwm.h"
#include "mgos_hvac.hpp"

int halfPeriodicTime;
int IRpin;
int khz;

typedef unsigned char byte;

// HVAC MITSUBISHI_
#define HVAC_MITSUBISHI_HDR_MARK    3400
#define HVAC_MITSUBISHI_HDR_SPACE   1750
#define HVAC_MITSUBISHI_BIT_MARK    450
#define HVAC_MITSUBISHI_ONE_SPACE   1300
#define HVAC_MISTUBISHI_ZERO_SPACE  420
#define HVAC_MITSUBISHI_RPT_MARK    440
#define HVAC_MITSUBISHI_RPT_SPACE   17100 // Above original iremote limit

//------------------------------------------------------------------------------
// enableIROut : Set global Variable for Frequency IR Emission
//------------------------------------------------------------------------------
void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  halfPeriodicTime = 500/khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
}

//------------------------------------------------------------------------------
// mark
//------------------------------------------------------------------------------
static void mark(int n)
{
  // make signal of circa 38 kHz of circa 1/2 duty
  //mgos_ints_disable();
  for (; n >= 0; --n) {
    mgos_gpio_write(IRpin, 1);
    mgos_usleep(13);
    mgos_gpio_write(IRpin, 0);
    mgos_usleep(13);
    //mgos_wdt_feed();
  }
  //mgos_ints_enable();
}

//------------------------------------------------------------------------------
// space ( int time) 
//------------------------------------------------------------------------------ 
/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  //mgos_ints_disable();
  mgos_gpio_write(IRpin, 0);
  if (time > 0) mgos_usleep(time);
  //mgos_ints_enable();
}

//------------------------------------------------------------------------------
// sendRaw (unsigned int buf[], int len, int hz)
//------------------------------------------------------------------------------
void sendRaw (unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

//------------------------------------------------------------------------------
// MITSUBISHI sender
//------------------------------------------------------------------------------


#define IRSEND_NEC_PWM_CYCLE  (1000000 / 38000)

void irsend_mitsubishi_pwm(int pin,
                           int OFF,
                           HvacMode HVAC_Mode,
                           int      HVAC_Temp,           // Example 21  (°c)
                           HvacFanMode   HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
                           HvacVanneMode HVAC_VanneMode) {

  IRpin = pin;

  byte mask = 1; //our bitmask
  byte data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x05, 0x30, 0x45, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F };
  byte i, j;
  
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT); // enableIROut(38);  // 38khz
  //mgos_wdt_set_timeout(10);

  // Byte 6 - On / Off
  if (OFF) {
    data[5] = (byte) 0x0; // Turn OFF HVAC
  } else {
    data[5] = (byte) 0x20; // Tuen ON HVAC
  }

  // Byte 7 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) 0x08; break;
    case HVAC_COLD:  data[6] = (byte) 0x18; break;
    case HVAC_DRY:   data[6] = (byte) 0x10; break;
    case HVAC_AUTO:  data[6] = (byte) 0x20; break;
    default: break;
  }

  // Byte 8 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 31) { Temp = 31;}
  else if (HVAC_Temp < 16) { Temp = 16; } 
  else { Temp = HVAC_Temp; };
  data[7] = (byte) Temp - 16;

  // Byte 10 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[9] = (byte) 0x01; break;//B00000001; break;
    case FAN_SPEED_2:       data[9] = (byte) 0x02; break;//B00000010; break;
    case FAN_SPEED_3:       data[9] = (byte) 0x03; break;//B00000011; break;
    case FAN_SPEED_4:       data[9] = (byte) 0x04; break;//B00000100; break;
    case FAN_SPEED_5:       data[9] = (byte) 0x04; break;//B00000100; break; //No FAN speed 5 for MITSUBISHI so it is consider as Speed 4
    case FAN_SPEED_AUTO:    data[9] = (byte) 0x80; break;//B10000000; break;
    case FAN_SPEED_SILENT:  data[9] = (byte) 0x05; break;//B00000101; break;
    default: break;
  }

  switch (HVAC_VanneMode)
  {
    case VANNE_AUTO:        data[9] = (byte) data[9] | 0x40; break;//B01000000; break;
    case VANNE_H1:          data[9] = (byte) data[9] | 0x48; break;//B01001000; break;
    case VANNE_H2:          data[9] = (byte) data[9] | 0x50; break;//B01010000; break;
    case VANNE_H3:          data[9] = (byte) data[9] | 0x58; break;//B01011000; break;
    case VANNE_H4:          data[9] = (byte) data[9] | 0x60; break;//B01100000; break;
    case VANNE_H5:          data[9] = (byte) data[9] | 0x68; break;//B01101000; break;
    case VANNE_AUTO_MOVE:   data[9] = (byte) data[9] | 0x78; break;//B01111000; break;
    default: break;
  }

  data[17] = 0;
  for (i = 0; i < 17; i++) {
    data[17] = (byte) data[i] + data[17];  // CRC is a simple bits addition
  }

  LOG(LL_WARN, ("Sending...%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17]));

  for (j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mgos_ints_disable();
    mark(HVAC_MITSUBISHI_HDR_MARK/IRSEND_NEC_PWM_CYCLE );
    space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_MITSUBISHI_BIT_MARK/IRSEND_NEC_PWM_CYCLE );
          space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_MITSUBISHI_BIT_MARK/IRSEND_NEC_PWM_CYCLE);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_MITSUBISHI_RPT_MARK/IRSEND_NEC_PWM_CYCLE );
    space(HVAC_MITSUBISHI_RPT_SPACE);
    space(0); // Just to be sure
    mgos_ints_enable();
  }

  // cleanup
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_INPUT);
}

void mgos_irsend_mitsubishi(int pin,
                           int off,
                           HvacMode HVAC_Mode,
                           int      HVAC_Temp,                // Example 21  (°c)
                           HvacFanMode   HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
                           HvacVanneMode HVAC_VanneMode)
{
  LOG(LL_DEBUG, ("IRSEND @ %d: %08X", pin, off));

  irsend_mitsubishi_pwm(pin,
                        off,
                        HVAC_Mode,
                        HVAC_Temp,           // Example 21  (°c)
                        HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
                        HVAC_VanneMode);
}

//------------------------------------------------------------------------------


// NOTE: library init function must be called mgos_LIBNAME_init()
bool mgos_hvac_init(void) {
  return true;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */