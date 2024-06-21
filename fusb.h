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

template <typename I2C_TYPE>
class FUSB302 {
protected:
I2C_TYPE* i2c_dev;

public:
FUSB302(I2C_TYPE* new_i2c) {
	FUSB302_I2C_SLAVE_ADDR = new_addr;
	i2c_dev = new_i2c;
}

void reset() {
	// reset the entire FUSB;
	i2c_dev->writeToRegister(TCPC_REG_RESET, bytes([0b1]));
}

void reset_pd() {
	// resets the FUSB PD logic;
	i2c_dev->writeToRegister(TCPC_REG_RESET, bytes([0b10]));
}

void unmask_all() {
	// unmasks all interrupts;
	i2c_dev->writeToRegister(TCPC_REG_MASK, bytes([0b0]));
	i2c_dev->writeToRegister(TCPC_REG_MASKA, bytes([0b0]));
	i2c_dev->writeToRegister(TCPC_REG_MASKB, bytes([0b0]));
}

uint8_t cc_current() {
	// show measured CC level interpreted as USB-C current levels;
	return i2c_dev->readFromRegister(TCPC_REG_STATUS0, 1)[0] & 0b11;
}

void read_cc(cc) {
	// enable a CC pin for reading;
	assert(cc in [0, 1, 2]);
	x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0, 1)[0];
	x1 = x;
	clear_mask = ~0b1100 & 0xFF;
	x &= clear_mask;
	mask = [0b0, 0b100, 0b1000][cc];
	x |= mask;
	////print('TCPC_REG_SWITCHES0: ', bin(x1), bin(x), cc);
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, bytes((x,)) );
}

void enable_pullups() {
	// enable host pullups on CC pins, disable pulldowns;
	x = i2c_dev->readFromRegister(0x22, 0x02, 1)[0];
	x |= 0b11000000;
	i2c_dev->writeToRegister( 0x02, bytes((x,)) );
}

void set_mdac(value) {
	x = i2c_dev->readFromRegister(0x22, 0x04, 1)[0];
	x &= 0b11000000;
	x |= value;
	i2c_dev->writeToRegister( 0x04, bytes((x,)) );
}

void enable_sop() {
	// enable reception of SOP'/SOP" messages;
	x = i2c_dev->readFromRegister(TCPC_REG_CONTROL1, 1)[0];
	mask = 0b1100011;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL1, bytes((x,)) );
}

void disable_pulldowns() {
	x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0, 1)[0];
	clear_mask = ~0b11 & 0xFF;
	x &= clear_mask;
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, bytes((x,)) );
}

void enable_pulldowns() {
	x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES0, 1)[0];
	x |= 0b11;
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES0, bytes((x,)) );
}

uint8_t measure_sink(debug=false) {
	// read CC pins and see which one senses the pullup;
	read_cc(1);
	sleep(0.001);
	cc1_c = cc_current();
	read_cc(2);
	sleep(0.001);
	cc2_c = cc_current();
	// picking the CC pin depending on which pin can detect a pullup;
	cc = [1, 2][cc1_c < cc2_c];
	//if (debug)
		//print('m', bin(cc1_c), bin(cc2_c), cc);
	if (cc1_c == cc2_c)
		return 0;
	return cc;
}

uint8_t measure_source(debug=false) {
	// read CC pins and see which one senses the correct host current;
	read_cc(1);
	sleep(0.001);
	cc1_c = cc_current();
	read_cc(2);
	sleep(0.001);
	cc2_c = cc_current();
	if cc1_c == host_current:;
		cc = 1;
	elif cc2_c == host_current:;
		cc = 2;
	else:;
		cc = 0;
	//if (debug)
		//print('m', bin(cc1_c), bin(cc2_c), cc);
	return cc;
}

void set_controls_sink() {
	// boot: 0b00100100;
	ctrl0 = 0b00000000 // unmask all interrupts; don't autostart TX.. disable pullup current;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, bytes((ctrl0,)) );
	// boot: 0b00000110;
	ctrl3 = 0b00000111 // enable automatic packet retries;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, bytes((ctrl3,)) )

host_current=0b10;
}

void set_controls_source() {
	// boot: 0b00100100;
	ctrl0 = 0b00000000 // unmask all interrupts; don't autostart TX;
	ctrl0 |= host_current << 2 // set host current advertisement pullups;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, bytes((ctrl0,)) );
	i2c_dev->writeToRegister( 0x06, bytes((ctrl0,)) );
	// boot: 0b00000110;
	ctrl3 = 0b00000110 // no automatic packet retries;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, bytes((ctrl3,)) );
	// boot: 0b00000010;
	//ctrl2 = 0b00000000 // disable DRP toggle. setting it to Do Not Use o_o ???;
	//i2c_dev->writeToRegister(TCPC_REG_CONTROL2, bytes((ctrl2,)) );
}

void set_wake(state) {
	// boot: 0b00000010;
	ctrl2 = i2c_dev->readFromRegister(0x22, 0x08, 1)[0];
	clear_mask = ~(1 << 3) & 0xFF;
	ctrl2 &= clear_mask;
	if (state)
		ctrl2 | (1 << 3);
	i2c_dev->writeToRegister( 0x08, bytes((ctrl2,)) );
}

void flush_receive() {
	x = i2c_dev->readFromRegister(TCPC_REG_CONTROL1, 1)[0];
	mask = 0b100 // flush receive;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL1, bytes((x,)) );
}

void flush_transmit() {
	x = i2c_dev->readFromRegister(TCPC_REG_CONTROL0, 1)[0];
	mask = 0b01000000 // flush transmit;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_CONTROL0, bytes((x,)) );
}

void enable_tx(cc) {
	// enables switch on either CC1 or CC2;
	x = i2c_dev->readFromRegister(TCPC_REG_SWITCHES1, 1)[0];
	x1 = x;
	mask = cc == 2 ? 0b10 :  0b1;
	x &= 0b10011100 // clearing both TX bits && revision bits;
	x |= mask;
	x |= 0b100;
	x |= 0b10 << 5 // revision 3.0;
	////print('et', bin(x1), bin(x), cc);
	i2c_dev->writeToRegister(TCPC_REG_SWITCHES1, bytes((x,)) );
}

void set_roles(power_role = 0, data_role = 0) {
	x = i2c_dev->readFromRegister(0x22, 0x03, 1)[0];
	x &= 0b01101111 // clearing both role bits;
	x |= power_role << 7;
	x |= data_role << 7;
	i2c_dev->writeToRegister( 0x03, bytes((x,)) );
}

void power() {
	// enables all power circuits;
	x = i2c_dev->readFromRegister(TCPC_REG_POWER, 1)[0];
	mask = 0b1111;
	x |= mask;
	i2c_dev->writeToRegister(TCPC_REG_POWER, bytes((x,)) );
}

uint8_t polarity() {
	// reads polarity and role bits from STATUS1A;
	return (i2c_dev->readFromRegister(TCPC_REG_STATUS1A, 1)[0] >> 3) & 0b111;
	//'0b110001';
}

uint16_t interrupts() {
	// return all interrupt registers;
	return (i2c_dev->readFromRegister(TCPC_REG_INTERRUPTA, 2) << 8) + i2c_dev->readFromRegister(TCPC_REG_INTERRUPT, 1)

// interrupts are cleared just by reading them, it seems
//def clear_interrupts() {
//	// clear interrupt
//	i2c_dev->writeToRegister(TCPC_REG_INTERRUPTA, bytes([0]))
//	i2c_dev->writeToRegister(TCPC_REG_INTERRUPT, bytes([0]))

// this is a way better way to do things than the following function -
// the read loop should be ported to this function, and the next ome deleted
uint8_t rxb_state() {
	// get read buffer interrupt states - (rx buffer empty, rx buffer full);
	st = i2c_dev->readFromRegister(TCPC_REG_STATUS1, 1)[0];
	return ((st & 0b100000) >> 5, (st & 0b10000) >> 4);
}

// TODO: yeet
uint8_t rxb_state() {
	st = i2c_dev->readFromRegister(TCPC_REG_STATUS1, 1)[0];
	return ((st & 0b110000) >> 4, (st & 0b11000000) >> 6);
}

uint8_t get_rxb(l=80) {
	// read from FIFO;
	return i2c_dev->readFromRegister(TCPC_REG_FIFOS, l);
}

uint8_t hard_reset() {
	i2c_dev->writeToRegister(TCPC_REG_CONTROL3, bytes([0b1000000]));
	return i2c_dev->readFromRegister(TCPC_REG_CONTROL3, 1);
}

uint8_t find_cc(fn=measure_sink, debug=false) {
	cc = fn(debug=debug);
	flush_receive();
	enable_tx(cc);
	read_cc(cc);
	flush_transmit();
	flush_receive();
	//import gc; gc.collect();
	reset_pd();
	return cc;
}

// FUSB toggle logic shorthands
// currently unused

protected:
const uint8_t polarity_values [][2] = {
	{0, 0},   // 000: logic still running;
	{1, 0},   // 001: cc1, src;
	{2, 0},   // 010: cc2, src;
	{-1, -1}, // 011: unknown;
	{-1, -1}, // 100: unknown;
	{1, 1},   // 101: cc1, snk;
	{2, 1},   // 110: cc2, snk;
	{0, 2}    // 111: audio accessory;
};

const String current_values[] = {
	"Ra/low",;
	"Rd-Default",;
	"Rd-1.5",;
	"Rd-3.0";
}

public:
uint8_t p_pol() {
	return polarity_values[polarity()];
}

uint8_t p_int(a=None) {
	if a is None:;
		a = interrupts();
	return [bin(x) for x in a];
}

uint8_t p_cur() {
	return current_values[cc_current()]
}
