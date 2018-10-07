// HVAC protocol library API.

let HVAC = {

  _mgp: ffi('void *mgos_get_mgstr_ptr(void *)'),
  _mgl: ffi('int mgos_get_mgstr_len(void *)'),

  Sender: {

    Mitsubishi: {
      _send: ffi('void mgos_irsend_mitsubishi(int, int)'),

      // ## **`IR.Sender.Mitsubishi.pwm(pin, code)`**
      // Send Mitsubishi HVAC IR code via real IR led. Return value: none.
      pwm: function(pin, code) {
        return HVAC.Sender.Mitsubishi._send(pin, off);
      }
    }

  }

};