
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include "mgos.h"
//#include "mgos_pwm.h"
#include "mgos_hvac.hpp"

int halfPeriodicTime;
int IRpin;
int khz;

typedef enum HvacMode {
  HVAC_HOT,
  HVAC_COLD,
  HVAC_DRY,
  HVAC_FAN, // used for Panasonic only
  HVAC_AUTO
} HvacMode_t; // HVAC  MODE

typedef enum HvacFanMode {
  FAN_SPEED_1,
  FAN_SPEED_2,
  FAN_SPEED_3,
  FAN_SPEED_4,
  FAN_SPEED_5,
  FAN_SPEED_AUTO,
  FAN_SPEED_SILENT
} HvacFanMode_;  // HVAC  FAN MODE

typedef enum HvacVanneMode {
  VANNE_AUTO,
  VANNE_H1,
  VANNE_H2,
  VANNE_H3,
  VANNE_H4,
  VANNE_H5,
  VANNE_AUTO_MOVE
} HvacVanneMode_;  // HVAC  VANNE MODE

typedef enum HvacWideVanneMode {
  WIDE_LEFT_END,
  WIDE_LEFT,
  WIDE_MIDDLE,
  WIDE_RIGHT,
  WIDE_RIGHT_END
} HvacWideVanneMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacAreaMode {
  AREA_SWING,
  AREA_LEFT,
  AREA_AUTO,
  AREA_RIGHT
} HvacAreaMode_t;  // HVAC  WIDE VANNE MODE

typedef enum HvacProfileMode {
  NORMAL,
  QUIET,
  BOOST
} HvacProfileMode_t;  // HVAC PANASONIC OPTION MODE


// HVAC MITSUBISHI_
#define HVAC_MITSUBISHI_HDR_MARK    3400
#define HVAC_MITSUBISHI_HDR_SPACE   1750
#define HVAC_MITSUBISHI_BIT_MARK    450
#define HVAC_MITSUBISHI_ONE_SPACE   1300
#define HVAC_MISTUBISHI_ZERO_SPACE  420
#define HVAC_MITSUBISHI_RPT_MARK    440
#define HVAC_MITSUBISHI_RPT_SPACE   17100 // Above original iremote limit


/****************************************************************************
/* enableIROut : Set global Variable for Frequency IR Emission
/***************************************************************************/ 
void enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  halfPeriodicTime = 500/khz; // T = 1/f but we need T/2 in microsecond and f is in kHz
}

//------------------------------------------------------------------------------
// mark
//------------------------------------------------------------------------------
static void mark(int pin, int n)
{
  // make signal of circa 38 kHz of circa 1/3 duty
  for (; n >= 0; --n) {
    mgos_gpio_write(pin, 1);
    mgos_usleep(13);
    mgos_gpio_write(pin, 0);
    mgos_usleep(13);
  }
}

/****************************************************************************
/* space ( int time) 
/***************************************************************************/ 
/* Leave pin off for time (given in microseconds) */
void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  mgos_gpio_write(IRpin, LOW);
  if (time > 0) mgos_usleep(time);
}

/****************************************************************************
/* sendRaw (unsigned int buf[], int len, int hz)
/***************************************************************************/ 
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

void irsend_mitsubishi_pwm(int pin, int code) {
  int32_t mask = 1; //our bitmask

  int32_t data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F };

  int i, j;
  
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT); // enableIROut(38);  // 38khz

  

  data[17] = 0;
  for (i = 0; i < 17; i++) {
    data[17] = (int32_t) data[i] + data[17];  // CRC is a simple bits addition
  }
  
  space(0);

  for (j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(pin, HVAC_MITSUBISHI_HDR_MARK); //mark(HVAC_MITSUBISHI_HDR_MARK);
    space(HVAC_MITSUBISHI_HDR_SPACE); //space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(pin, HVAC_MITSUBISHI_BIT_MARK); //mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MITSUBISHI_ONE_SPACE); //space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(pin, HVAC_MITSUBISHI_BIT_MARK);//mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MITSUBISHI_ZERO_SPACE); //space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(pin, HVAC_MITSUBISHI_RPT_MARK); //mark(HVAC_MITSUBISHI_RPT_MARK);
    space(HVAC_MITSUBISHI_RPT_SPACE); //space(HVAC_MITSUBISHI_RPT_SPACE);
    space(0); // Just to be sure
  }

  // cleanup
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_INPUT);
}

void mgos_irsend_mitsubishi(int pin, int code)
{
  LOG(LL_DEBUG, ("IRSEND @ %d: %08X", pin, code));

  irsend_mitsubishi_pwm(pin, code);
}

//------------------------------------------------------------------------------


// NOTE: library init function must be called mgos_LIBNAME_init()
bool mgos_hvac_init(void) {
  return true;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */