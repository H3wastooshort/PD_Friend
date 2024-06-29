using pdo_validate_callback_t = bool(*)(uint8_t*,size_t);

class PDStack_SRC : public PDStack{
protected:
	void do_add_msg_resp(uint8_t* msg, size_t len) {
		switch(0) {
			default:
				send_ctrl_msg(PDM_Not_Supported);
				break;
		}
	}
	
	enum src_state_t {
		SRC_ADVERTIZE, //advertizing capabilites to device
		SRC_WAIT, //got a GoodCRC packet, stop advertising
		SRC_STARTING_PSU, //waiting to send PS_RDY
		SRC_PDO_ACTIVE //pdo has been selected
	} src_state;
	
	void run_src_sm() {
		
	}
	
public:
	PDStack_SRC(FUSB302 fusb) : PDStack(fusb) {};
	
	pdo_validate_callback_t pdo_validate_callback = NULL; //checks if PDO is valid, sends accept/reject and starts the powwer supply
	bool_callback_t ps_rdy_callback = NULL; //returns true if PS_RDY can be sent
	
	void init_src() {
		init_universal();
		fusb->set_controls_source();
		fusb->set_roles(true,true);
		fusb->disable_pulldowns();
	}
	
	bool attach() {
		fusb->disable_pulldowns();
		fusb->set_wake(true);
		fusb->enable_pullups();
		fusb->set_mdac(0b111111);
		
		uint8_t cc = fusb->find_cc_source();
		
		if (cc==0) return false;
		//else
		fusb->set_cc(cc);
		return true;
	}
	
	void detach() {
		//fusb->disable_pullups(); //TODO
	}
	
	void set_analog_host_current(uint8_t hc) {
		
	}
	
	
	void set_pdos(const uint32_t pdos[], size_t len) {
		
	}
	
	void handle() {
		
		handle_base();
	}
};