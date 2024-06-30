//stuff only relevant to SRC
class PDStack_SRC : public PDStack {
public:
PDStack_SRC(FUSB302 fusb) : PDStack(fusb) {};


/*
B31…30 Fixed supply
B29 Dual-Role Power
B28 USB Suspend Supported
B27 Unconstrained Power
B26 USB Communications Capable
B25 Dual-Role Data
B24 Unchunked Extended Messages Supported
B23 EPR Mode Capable
B22 Reserved – Shall be set to zero.
B21…20 Peak Current
B19…10 Voltage in 50mV units
B9…0 Maximum Current in 10mA units

Source: USB C PD Specification, Page 147, Table 6.9 “Fixed Supply PDO – Source”
*/
static uint32_t make_fixed_pdo(uint16_t voltage_mV, uint16_t current_mA, uint8_t peak_current = 0b00, bool unconstrained_power = true, bool usb_data_capable = false, bool usb_dual_role_data = false) {
	uint16_t voltage = voltage_mV / 50;
	uint16_t current = current_mA / 10;
	
	uint32_t pdo = 0;
	//pdo |= 0b00 << 30; //fixed PDO
	//B29 no DRP support here
	//B28 no USB suspend here
	pdo |= uint32_t(unconstrained_power & 1) << 27;
	pdo |= uint32_t(usb_data_capable & 1) << 26;
	pdo |= uint32_t(usb_dual_role_data & 1) << 25;
	//B24 no
	//B23 rather not with this shitty pd stack
	//B22 0
	pdo |= uint32_t(peak_current & 0b11) << 20;
	pdo |= uint32_t(current & 0x01FF) << 10;
	pdo |= uint32_t(voltage & 0x01FF) << 0;
	
	return pdo;
}

void init_src() {
  init_universal();
  fusb->set_controls_source();
  fusb->set_roles(true, true);
  fusb->disable_pulldowns();
}

bool attach_src() {
  fusb->disable_pulldowns();
  fusb->set_wake(true);
  fusb->enable_pullups();
  fusb->set_mdac(0b111111);

  uint8_t cc = fusb->find_cc_source();

  if (cc == 0) return false;
  //else
  fusb->set_cc(cc);
  return true;
}
};