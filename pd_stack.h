//PD stack stuff
#include "pwr_snk.h"
#include "pwr_src.h"
#include "other.h"
template <typename I2C_TYPE>
class PDStack : public FUSB302<I2C_TYPE>{
public:
	void interruptCallback() {
		
	}
	void handle() {
		
	}
};