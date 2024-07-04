enum ctrl_msg_type_t {
	PDCM_Reserved=0,
	PDCM_GoodCRC=1,
	PDCM_GotoMin=2,
	PDCM_Accept=3,
	PDCM_Reject=4,
	PDCM_Ping=5,
	PDCM_PS_RDY=6,
	PDCM_Get_Source_Cap=7,
	PDCM_Get_Sink_Cap=8,
	PDCM_DR_Swap=9,
	PDCM_PR_Swap=10,
	PDCM_VCONN_Swap=11,
	PDCM_Wait=12,
	PDCM_Soft_Reset=13,
	PDCM_Data_Reset=14,
	PDCM_Data_Reset_Complete=15,
	PDCM_Not_Supported=16,
	PDCM_Get_Source_Cap_Extended=17,
	PDCM_Get_Status=18,
	PDCM_FR_Swap=19,
	PDCM_Get_PPS_Status=20,
	PDCM_Get_Country_Codes=21,
	PDCM_Get_Sink_Cap_Extended=22,
	PDCM_Get_Source_Info=23,
	PDCM_Get_Revision=24,
	
	PDCM_INVALID = 255
};

enum data_msg_type_t {
	PDDM_Reserved=0,
	PDDM_Source_Capabilities=1,
	PDDM_Request=2,
	PDDM_BIST=3,
	PDDM_Sink_Capabilities=4,
	PDDM_Battery_Status=5,
	PDDM_Alert=6,
	PDDM_Get_Country_Info=7,
	PDDM_Enter_USB=8,
	PDDM_EPR_Request=9,
	PDDM_EPR_Mode=10,
	PDDM_Source_Info=11,
	PDDM_Revision=12,
	//PDDM_Reserved=13,
	//PDDM_Reserved=14,
	PDDM_Vendor_Defined=15,
	
	PDDM_INVALID = 255
};

using pdo_t = struct pdo_struct{
	uint16_t voltage;
	uint16_t current;
};

using bool_callback_t = bool(*)();

class PDStack { //use this as the base for your own implementation
protected:
FUSB302* fusb;
uint8_t current_message_id = 0;

public:

void do_other_msg_resp(uint8_t* msg, size_t len) { //call this, when you dont want to deal with a packet.
	if (is_data_msg(msg,len)) 
		switch (get_data_msg_type(msg,len)) {
			default: send_ctrl_msg(PDCM_Not_Supported); break;
		}
	
	else
		switch (get_ctrl_msg_type(msg,len)) {
			case PDCM_GoodCRC: break;
			default: send_ctrl_msg(PDCM_Not_Supported); break;
		}
}

static bool is_data_msg(uint8_t* msg, size_t len) {
	//return len > 2;
	
	if (len < 2) return false;
	uint8_t num_dat_obj = (msg[1] & 0b01110000) >> 4;
	return num_dat_obj > 0;
}

static ctrl_msg_type_t get_ctrl_msg_type(uint8_t* msg, size_t len) {
	if (len < 2) return PDCM_INVALID;
	return msg[0] & 0b11111;
}

static data_msg_type_t get_data_msg_type(uint8_t* msg, size_t len) {
	if (len < 2) return PDDM_INVALID;
	return msg[0] & 0b11111;
}

PDStack (FUSB302& new_fusb) {
	fusb = &new_fusb;
}

	void read_msg(uint8_t* buf, size_t len) {
		fusb->read_msg(buf,len);
	}

	void send_ctrl_msg(ctrl_msg_type_t msg_type) {
		fusb->send_ctrl_msg(msg_type, current_message_id);
		current_message_id++;
		current_message_id %= 32;
	}

	bool send_data_msg(data_msg_type_t msg_type, uint32_t* data_objects, uint8_t num_data_objects) {
		if (num_data_objects > 7) return false;
		fusb->send_data_msg(msg_type, data_objects, num_data_objects, current_message_id);
		current_message_id++;
		current_message_id %= 32;
		return true;
	}

	void init_universal() {
		fusb->reset();
		fusb->power();
		fusb->unmask_all();
	}

	void reset() {
		send_ctrl_msg(PDCM_Soft_Reset);
		fusb->reset_pd();
		current_message_id=0;
	}

	void detach() { //
		fusb->disable_pullups();
		fusb->disable_pulldowns();
		fusb->set_wake(false);
		fusb->set_cc(0);
	}


};