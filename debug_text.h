#ifdef FUSB_DEBUG_SERIAL

const String current_values[4] = {
	"Ra/low",
	"Rd-Default",
	"Rd-1.5",
	"Rd-3.0"
};

//packets

const String control_message_types[25] = [
    "Reserved",
    "GoodCRC",
    "GotoMin",
    "Accept",
    "Reject",
    "Ping",
    "PS_RDY",
    "Get_Source_Cap",
    "Get_Sink_Cap",
    "DR_Swap",
    "PR_Swap",
    "VCONN_Swap",
    "Wait",
    "Soft_Reset",
    "Data_Reset",
    "Data_Reset_Complete",
    "Not_Supported",
    "Get_Source_Cap_Extended",
    "Get_Status",
    "FR_Swap",
    "Get_PPS_Status",
    "Get_Country_Codes",
    "Get_Sink_Cap_Extended",
    "Get_Source_Info",
    "Get_Revision"
]

const String data_message_types[16] = [
    "Reserved",
    "Source_Capabilities",
    "Request",
    "BIST",
    "Sink_Capabilities",
    "Battery_Status",
    "Alert",
    "Get_Country_Info",
    "Enter_USB",
    "EPR_Request",
    "EPR_Mode",
    "Source_Info",
    "Revision",
    "Reserved",
    "Reserved",
    "Vendor_Defined"
]


//PDOs
const std::map<pdo_type_t,String> {
	
};


//VDMs
const String vdm_commands[7] = {
    "Reserved",
    "Discover Identity",
    "Discover SVIDs",
    "Discover Modes",
    "Enter Mode",
    "Exit Mode",
    "Attention"
};

const std::map<uint16_t,String> svids = {
    {0xff00, 'SID'},
    {0xff01, 'DisplayPort'}
};

const std::map<uint8_t,String> dp_commands = {
    {0x10, "DP Status Update"},
    {0x11, "DP Configure"}
};

const String vdm_cmd_types[4] = {"REQ", "ACK", "NAK", "BUSY"};

const std::map<uint8_t,String> vdm_dp_pin_assg = {
	 {0b1,"A"},
	 {0b10,"B"},
	 {0b100,"C"},
	 {0b1000,"D"},
	 {0b10000,"E"},
	 {0b100000,"F"}
};

const String vdm_dp_port_cap = {
 "RES",
 "UFP",
 "DFP",
 "UFP&DFP"
};

const String vdm_dp_port_conn = {
 "NC",
 "UFP",
 "DFP",
 "UFP&DFP"
};

const String vdm_dp_port_conf = {
 "USB",
 "DFP",
 "DFP",
 "RES"
};

const std::map<uint8_t,String> vdm_dp_sgn = {
 0b1:"DP",
 0b10:"USBg2",
 0b100:"RES1",
 0b1000:"RES2"
}

#endif