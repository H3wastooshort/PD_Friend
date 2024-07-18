//[SWI2C](//https://github.com/H3wastooshort/SWI2C_H3) compatibility
class PDFriendI2C {
private:
	SWI2C* i2c;
public:
	PDFriendI2C(SWI2C& new_i2c) {i2c = &new_i2c;}
	uint8_t readFromRegister(uint8_t reg) {
		uint8_t dat=0xFF;
		bool ok = i2c->readFromRegister(reg,dat);
#ifdef FUSB_DEBUG_SERIAL
		Serial.println();
		Serial.write('r');
		Serial.printHex(reg);
		Serial.write('R');
		Serial.printHex(dat);
		if(!ok) Serial.write('-');
		if(!i2c->checkStretchTimeout()) Serial.write('S');
		Serial.println();
#endif
		return dat;
	}
	bool writeToRegister(uint8_t reg, uint8_t dat) {
#ifdef FUSB_DEBUG_SERIAL
		Serial.println();
		Serial.write('r');
		Serial.printHex(reg);
		Serial.write('W');
		Serial.printHex(dat);
#endif
		bool ok = i2c->writeToRegister(reg,dat);
#ifdef FUSB_DEBUG_SERIAL
		if(!ok) Serial.write('-');
		if(!i2c->checkStretchTimeout()) Serial.write('S');
		Serial.println();
#endif
		return ok;
	}
};