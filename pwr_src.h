class PDStack_SRC : public PDStack{
protected:
	void do_add_msg_resp(uint8_t* msg, size_t len) {
		switch(0) {
			default:
				send_ctrl_msg(PDM_Not_Supported);
				break;
		}
	}
	

public:
	PDStack_SRC(FUSB302 fusb) : PDStack(fusb) {};
	
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
};