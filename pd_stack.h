enum ctrl_msg_type_t {
	//PDM_Reserved=0,
	PDM_GoodCRC=1,
	PDM_GotoMin=2,
	PDM_Accept=3,
	PDM_Reject=4,
	PDM_Ping=5,
	PDM_PS_RDY=6,
	PDM_Get_Source_Cap=7,
	PDM_Get_Sink_Cap=8,
	PDM_DR_Swap=9,
	PDM_PR_Swap=10,
	PDM_VCONN_Swap=11,
	PDM_Wait=12,
	PDM_Soft_Reset=13,
	PDM_Data_Reset=14,
	PDM_Data_Reset_Complete=15,
	PDM_Not_Supported=16,
	PDM_Get_Source_Cap_Extended=17,
	PDM_Get_Status=18,
	PDM_FR_Swap=19,
	PDM_Get_PPS_Status=20,
	PDM_Get_Country_Codes=21,
	PDM_Get_Sink_Cap_Extended=22,
	PDM_Get_Source_Info=23,
	PDM_Get_Revision=24
};

enum data_msg_type_t {
	//PDM_Reserved=0,
	PDM_Source_Capabilities=1,
	PDM_Request=2,
	PDM_BIST=3,
	PDM_Sink_Capabilities=4,
	PDM_Battery_Status=5,
	PDM_Alert=6,
	PDM_Get_Country_Info=7,
	PDM_Enter_USB=8,
	PDM_EPR_Request=9,
	PDM_EPR_Mode=10,
	PDM_Source_Info=11,
	PDM_Revision=12,
	//PDM_Reserved=13,
	//PDM_Reserved=14,
	PDM_Vendor_Defined=15
};

using pdo_t = struct pdo_struct{
	uint16_t voltage;
	uint16_t current;
};

using bool_callback_t = bool(*)();

class PDStack { //use this as the base for your own implementation
protected:
FUSB302* fusb;

virtual void do_add_msg_resp(uint8_t* msg, size_t len) { //this gets replaced by SRC/SNK code
	send_ctrl_msg(PDM_Not_Supported);
}

void do_msg_resp(uint8_t* msg, size_t len) {
	switch(0) { //reply to common messages
		default: do_add_msg_resp(msg,len); break;
	}
}

bool get_msg_type(uint8_t& buffer, size_t buffer_len) { //returns true for data messages
	return false;
}

void handle_base() {
	
}

public:
PDStack (FUSB302& new_fusb) {
	fusb = &new_fusb;
}

	pdo_t parse_pdo(uint8_t* buf,size_t len) {
		
	}

	void init_universal() {
		fusb->reset();
		fusb->power();
		fusb->unmask_all();
	}

	void disconnect() { //
		
	}

	void interruptCallback() {
		
	}
	virtual void handle() {
		handle_base();
	}


	void send_ctrl_msg(ctrl_msg_type_t msg_type) {
		
	}

	void send_data_msg(data_msg_type_t msg_type, uint8_t* data) {
		
	}
};