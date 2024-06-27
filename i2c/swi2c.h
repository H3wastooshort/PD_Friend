//[SWI2C](//https://github.com/H3wastooshort/SWI2C_H3) compatibility
class PDFriendI2C {
private:
	SWI2C* i2c;
public:
	PDFriendI2C(SWI2C& new_i2c) {i2c = &new_i2c;}
	uint8_t readFromRegister(uint8_t reg) {
		uint8_t dat;
		i2c->readFromRegister(reg,dat);
		return dat;
	}
	bool writeToRegister(uint8_t reg, uint8_t dat) {
		return i2c->writeToRegister(reg,dat);
	}
};