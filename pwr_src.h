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
	void set_pdos(const uint32_t pdos[], size_t len) {
		
	}
};