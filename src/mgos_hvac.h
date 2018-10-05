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

/*
 * Send IR code for Mitsubishi HVAC protocol.
 * Params:
 * pin:  GPIO number.
 * code: 32-bit code.
 */
void mgos_irsend_mitsuibishi(int pin, int code);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CS_FW_SRC_MGOS_HVAC_H_ */