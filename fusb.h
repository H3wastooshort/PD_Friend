////////////////////////////////////////////////
//
// FUSB-specific code
//
////////////////////////////////////////////////

const uint8_t FUSB302_I2C_SLAVE_ADDR = 0x22;
const uint8_t TCPC_REG_DEVICE_ID = 0x01;
const uint8_t TCPC_REG_SWITCHES0 = 0x02;
const uint8_t TCPC_REG_SWITCHES1 = 0x03;
const uint8_t TCPC_REG_MEASURE = 0x04;
const uint8_t TCPC_REG_CONTROL0 = 0x06;
const uint8_t TCPC_REG_CONTROL1 = 0x07;
const uint8_t TCPC_REG_CONTROL2 = 0x08;
const uint8_t TCPC_REG_CONTROL3 = 0x09;
const uint8_t TCPC_REG_MASK = 0x0A;
const uint8_t TCPC_REG_POWER = 0x0B;
const uint8_t TCPC_REG_RESET = 0x0C;
const uint8_t TCPC_REG_MASKA = 0x0E;
const uint8_t TCPC_REG_MASKB = 0x0F;
const uint8_t TCPC_REG_STATUS0A = 0x3C;
const uint8_t TCPC_REG_STATUS1A = 0x3D;
const uint8_t TCPC_REG_INTERRUPTA = 0x3E;
const uint8_t TCPC_REG_INTERRUPTB = 0x3F;
const uint8_t TCPC_REG_STATUS0 = 0x40;
const uint8_t TCPC_REG_STATUS1 = 0x41;
const uint8_t TCPC_REG_INTERRUPT = 0x42;
const uint8_t TCPC_REG_FIFOS = 0x43;

class FUSB302 {
protected:
PDFriendI2C* i2c_dev;

public:
FUSB302(PDFriendI2C& new_i2c) {
	i2c_dev = &new_i2c;
}

void reset() {
	// reset the entire FUSB;
	i2c_dev->writeToRegister(TCPC_REG_RESET, 0b1);
}

void reset_pd() {
	// resets the FUSB PD logic;
	i2c_dev->writeToRegister(TCPC_REG_RESET, 0b10);
}

void unmask_all() {
	// unmasks all interrupts;
	i2c_dev->writeToRegister(TCPC_REG_MASK, 0b0);
	i2c_dev->writeToRegister(TCPC_REG_MASKA, 0b0);
	i2c_dev->writeToRegister(TCPC_REG_MASKB, 0b0);
}

uint8_t cc_current() {
	// show measured CC level interpreted as USB-C current levels;
	return i2c_dev->readFromRegister(TCPC_REG_STATUS0) & 0b11;
}

void read_cc(uint8_t cc) {
	// enable a CC pin for reading;
	if (cc > 2) return;
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0);
	//uint8_t x1 = x;
	uint8_t clear_mask = ~0b1100 & 0xFF;
	x &= clear_mask;
	const uint8_t masks[3] = {0b0, 0b100, 0b1000};
	x |= masks[cc];
	////print('TCPC_REG_SWITCHES0: ', bin(x1), bin(x), cc);
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, x);
}

void enable_pullups() {
	// enable host pullups on CC pins, disable pulldowns;
	uint8_t x = i2c_dev->readFromRegister(0x02);
	x |= 0b11000000;
	i2c_dev->writeToRegister( 0x02, x);
}

void set_mdac(uint8_t value) {
	uint8_t x = i2c_dev->readFromRegister(0x04);
	x &= 0b11000000;
	x |= value;
	i2c_dev->writeToRegister( TCPC_REG_MEASURE, x);
}

void enable_sop() {
	// enable reception of SOP'/SOP" messages;
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_CONTROL1);
	uint8_t mask = 0b1100011;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL1, x );
}

void disable_pulldowns() {
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0);
	uint8_t clear_mask = ~0b11 & 0xFF;
	x &= clear_mask;
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, x );
}

void enable_pulldowns() {
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0);
	x |= 0b11;
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, x );
}

uint8_t find_cc_sink() {
	// read CC pins and see which one senses the pullup;
	read_cc(1);
	delay(1);
	uint8_t cc1_c = cc_current();
	read_cc(2);
	delay(1);
	uint8_t cc2_c = cc_current();
	// picking the CC pin depending on which pin can detect a pullup;
	uint8_t cc = cc1_c < cc2_c ? 2 : 1;
	#ifdef FUSB_DEBUG_SERIAL
		FUSB_DEBUG_SERIAL.println();
		FUSB_DEBUG_SERIAL.print("m ");
		FUSB_DEBUG_SERIAL.print(cc1_c);
		FUSB_DEBUG_SERIAL.print(' ');
		FUSB_DEBUG_SERIAL.print(cc2_c);
		FUSB_DEBUG_SERIAL.print(' ');
		FUSB_DEBUG_SERIAL.println(cc);
	#endif
	if (cc1_c == cc2_c)
		return 0;
	return cc;
}

uint8_t host_current=0b10;

uint8_t find_cc_source() {
	// read CC pins and see which one senses the correct host current;
	read_cc(1);
	delay(1);
	uint8_t cc1_c = cc_current();
	read_cc(2);
	delay(1);
	uint8_t cc2_c = cc_current();
	uint8_t cc;
	if (cc1_c == host_current)
		cc = 1;
	else if (cc2_c == host_current)
		cc = 2;
	else
		cc = 0;
	#ifdef FUSB_DEBUG_SERIAL
		FUSB_DEBUG_SERIAL.println();
		FUSB_DEBUG_SERIAL.print("m ");
		FUSB_DEBUG_SERIAL.print(cc1_c);
		FUSB_DEBUG_SERIAL.print(' ');
		FUSB_DEBUG_SERIAL.print(cc2_c);
		FUSB_DEBUG_SERIAL.print(' ');
		FUSB_DEBUG_SERIAL.println(cc);
	#endif
	return cc;
}

void set_controls_sink() {
	// boot: 0b00100100;
	uint8_t ctrl0 = 0b00000000; // unmask all interrupts; don't autostart TX.. disable pullup current;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, ctrl0 );
	// boot: 0b00000110;
	uint8_t ctrl3 = 0b00000111; // enable automatic packet retries;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, ctrl3 );
}

void set_controls_source() {
	// boot: 0b00100100;
	uint8_t ctrl0 = 0b00000000; // unmask all interrupts; don't autostart TX;
	ctrl0 |= host_current << 2; // set host current advertisement pullups;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, ctrl0);
	i2c_dev->writeToRegister( 0x06, ctrl0);
	// boot: 0b00000110;
	uint8_t ctrl3 = 0b00000110; // no automatic packet retries;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, ctrl3);
	// boot: 0b00000010;
	//ctrl2 = 0b00000000 // disable DRP toggle. setting it to Do Not Use o_o ???;
	//i2c_dev->writeToRegister(TCPC_REG_CONTROL2, bytes((ctrl2,)) );
}

void set_wake(uint8_t state) {
	// boot: 0b00000010;
	uint8_t ctrl2 = i2c_dev->readFromRegister(0x08);
	uint8_t clear_mask = ~(1 << 3) & 0xFF;
	ctrl2 &= clear_mask;
	if (state)
		ctrl2 |= (1 << 3);
	i2c_dev->writeToRegister( 0x08, ctrl2);
}

void flush_receive() {
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_CONTROL1);
	uint8_t mask = 0b100; // flush receive;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL1, x );
}

void flush_transmit() {
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_CONTROL0);
	uint8_t mask = 0b01000000; // flush transmit;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, x );
}

void enable_tx(uint8_t cc) {
	// enables switch on either CC1 or CC2;
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES1);
	//uint8_t x1 = x;
	uint8_t mask = cc == 2 ? 0b10 :  0b1;
	x &= 0b10011100; // clearing both TX bits && revision bits;
	x |= mask;
	x |= 0b100;
	x |= 0b10 << 5; // revision 3.0;
	////print('et', bin(x1), bin(x), cc);
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES1, x );
}

void set_roles(bool power_role = 0, bool data_role = 0) {
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES1);
	x &= 0b01101111; // clearing both role bits
	x |= power_role << 7;
	x |= data_role << 7; //TODO: this seems off...
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES1, x);
}

void power() {
	// enables all power circuits
	uint8_t x = i2c_dev->readFromRegister(TCPC_REG_POWER);
	uint8_t mask = 0b1111;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_POWER, x );
}

uint8_t polarity() {
	// reads polarity and role bits from STATUS1A
	return (i2c_dev->readFromRegister(TCPC_REG_STATUS1A) >> 3) & 0b111;
	//'0b110001';
}

uint32_t get_interrupts() { //TODO: check if this is correct
	// return all interrupt registers;
	return
	((uint32_t)i2c_dev->readFromRegister(TCPC_REG_INTERRUPTB) << 16) +
	((uint32_t)i2c_dev->readFromRegister(TCPC_REG_INTERRUPTA) << 8) +
	 (uint32_t)i2c_dev->readFromRegister(TCPC_REG_INTERRUPT);
}

// interrupts are cleared just by reading them, it seems
//void clear_interrupts() {
//	// clear interrupt
//	i2c_dev->writeToRegister(TCPC_REG_INTERRUPTA);
//	i2c_dev->writeToRegister(TCPC_REG_INTERRUPT);
//}


uint8_t rxb_state() { //this function has been altered! now returns 2 bits
	// get read buffer interrupt states - (rx buffer empty, rx buffer full);
	uint8_t st = i2c_dev->readFromRegister(TCPC_REG_STATUS1);
	return (st & 0b100000) >> 4 | (st & 0b10000) >> 4;
}


uint8_t get_rxb(uint8_t l=80) {
	// read from FIFO;
	return i2c_dev->readFromRegister(TCPC_REG_FIFOS);
}

void tx_byte(uint8_t data) {
	i2c_dev->writeToRegister(TCPC_REG_FIFOS, data);
}

uint8_t hard_reset() {
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, 0b1000000);
	return i2c_dev->readFromRegister(TCPC_REG_CONTROL3);
}

uint8_t enable_cc(uint8_t cc) { //use with find_cc_sink() / find_cc_source()
	flush_receive();
	enable_tx(cc);
	read_cc(cc);
	flush_transmit();
	flush_receive();
	reset_pd();
	return cc;
}

// FUSB toggle logic shorthands
// currently unused

protected:
const int8_t polarity_values [8][2] = {
	{0, 0},   // 000: logic still running
	{1, 0},   // 001: cc1, src
	{2, 0},   // 010: cc2, src
	{-1, -1}, // 011: unknown
	{-1, -1}, // 100: unknown
	{1, 1},   // 101: cc1, snk
	{2, 1},   // 110: cc2, snk
	{0, 2}    // 111: audio accessory
};

public:
const int8_t* p_pol() {
	return polarity_values[polarity()];
}

uint16_t p_int(uint8_t a=0xFF) {
	if (a == 0xFF)
		a = get_interrupts();
	return a;
}

/*uint8_t p_cur() {
	return current_values[cc_current()];
}*/
};
