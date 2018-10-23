#include "mgos.h"

/*
 * Copyright 2018 John Forde <fordee@me.com>
 * All rights reserved
 *
 * Mitsubishi HVAC driver API
 */

#ifndef CS_FW_SRC_MGOS_HVAC_H_
#define CS_FW_SRC_MGOS_HVAC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



//------------------------------------------------------------------------------
// Mitsubishi HVAC sender
//------------------------------------------------------------------------------

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

/*
 * Send IR code for Mitsubishi HVAC protocol.
 * Params:
 * pin:  GPIO number.
 * code: 32-bit code.
 */
void mgos_irsend_mitsuibishi(int pin,
                             int OFF,
                             HvacMode HVAC_Mode,
                             int      HVAC_Temp,           // Example 21  (°c)
                             HvacFanMode   HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
                             HvacVanneMode HVAC_VanneMode);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_FW_SRC_MGOS_HVAC_H_ */