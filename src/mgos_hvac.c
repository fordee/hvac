
//#include "mgos.h"
//#include "mgos_pwm.h"
#include "mgos_hvac.h"

//------------------------------------------------------------------------------
// PWM
//------------------------------------------------------------------------------

static void irsend_carrier_38kHz(int pin, int n)
{
  // make signal of circa 38 kHz of circa 1/3 duty
  for (; n >= 0; --n) {
    mgos_gpio_write(pin, 1);
    mgos_usleep(9);
    mgos_gpio_write(pin, 0);
    mgos_usleep(16);
  }
}

//------------------------------------------------------------------------------
// MITSUBISHI sender
//------------------------------------------------------------------------------

#define IRSEND_MITSU_HDR_MARK   3400
#define IRSEND_MITSU_HDR_SPACE  1750
#define IRSEND_MITSU_BIT_MARK    450
#define IRSEND_MITSU_ONE_SPACE  1300
#define IRSEND_MITSU_ZERO_SPACE  420
#define IRSEND_MITSU_RPT_MARK    440
#define IRSEND_MITSU_RPT_SPACE 17100

#define IRSEND_NEC_PWM_CYCLE  (1000000 / 38000)

int32_t mask = 1; //our bitmask

int32_t data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x05, 0x30, 0x45, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B };


//int32_t data[17] = { 0x23, 0xCB, 0x26, 0x21, 0x00, 0x40, 0x52, 0x35, 0x04, 0x00, 0x00, 0xBF, 0xAD, 0xCA, 0xFB, 0xFF, 0xFF };
// HEADER[8]  { 0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x00, 0x00 }
// CONTROL 5 + 1 {"POWER","MODE","TEMP","FAN","VANE"}; {0x01, 0x01,  0x05,  0x00, 0x01, 0x01};
  //                 { 0xfc, 0x41, 0x01, 0x30, 0x10, 0x01, 0x00, 0x00,         0x01, 0x01,  0x05,  0x00, 0x01, 0x01};
  // data array is a valid trame, only byte to be chnaged will be updated.
//  int32_t data[18] = {0xfc, 0x02, 0x00, 0x00, 0x01, 0x01, 0x0f, 0x00, 0x07, 0x00, 0x00, 0x03, 0x94, 0x00, 0x00, 0x00, 0x00, 0xac};

int i, j;

void irsend_mitsubishi_pwm(int pin, int code) {
  
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT); // enableIROut(38);  // 38khz

  //space(0);

    // Byte 11 - XOR of Byte 5
  data[11] = (int32_t) 0xff ^ data[5];
  
  // Byte 12 - XOR of Byte 6
  data[12] = (int32_t) 0xff ^ data[6];
  
  // Byte 13 - XOR of Byte 7
  data[13] = (int32_t) 0xff ^ data[7];
  
  for (j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    irsend_carrier_38kHz(pin, IRSEND_MITSU_HDR_MARK / IRSEND_NEC_PWM_CYCLE); //mark(HVAC_MITSUBISHI_HDR_MARK);
    mgos_usleep(IRSEND_MITSU_HDR_SPACE); //space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          irsend_carrier_38kHz(pin, IRSEND_MITSU_BIT_MARK / IRSEND_NEC_PWM_CYCLE); //mark(HVAC_MITSUBISHI_BIT_MARK);
          mgos_usleep(IRSEND_MITSU_ONE_SPACE); //space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          irsend_carrier_38kHz(pin, IRSEND_MITSU_BIT_MARK / IRSEND_NEC_PWM_CYCLE);//mark(HVAC_MITSUBISHI_BIT_MARK);
          mgos_usleep(IRSEND_MITSU_ZERO_SPACE); //space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    irsend_carrier_38kHz(pin, IRSEND_MITSU_RPT_MARK / IRSEND_NEC_PWM_CYCLE); //mark(HVAC_MITSUBISHI_RPT_MARK);
    mgos_usleep(IRSEND_MITSU_RPT_SPACE); //space(HVAC_MITSUBISHI_RPT_SPACE);
    //space(0); // Just to be sure
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