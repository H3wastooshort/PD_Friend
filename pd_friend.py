

########################
#
# USB-C stacc code
#
########################

pdo_requested = False
pdos = []
timing_start = 0
timing_end = 0

# set to -1 because it's incremented before each command is sent out
msg_id = -1

def increment_msg_id():
    global msg_id
    msg_id += 1
    if msg_id == 8: msg_id = 0
    return msg_id

def reset_msg_id():
    global msg_id
    msg_id = -1

sent_messages = []

########################
#
# Packet reception
# and parsing code
#
########################

header_starts = [0xe0, 0xc0]

def get_message(get_rxb=get_rxb):
    header = 0
    d = {}
    # we might have to get through some message data!
    while header not in header_starts:
        header = get_rxb(1)[0]
        if header == 0:
            return
        if header not in header_starts:
            # this will be printed, eventually.
            # the aim is that it doesn't delay code in the way that print() seems to
            sys.stdout.write("disc {}\n".format(hex(header)))
    d["o"] = False # incoming message
    d["h"] = header
    b1, b0 = get_rxb(2)
    d["b0"] = b0
    d["b1"] = b1
    sop = 1 if header == 0xe0 else 0
    d["st"] = sop
    # parsing the packet header
    prole = b0 & 1
    d["pr"] = prole
    drole = b1 >> 5 & 1
    d["dr"] = drole
    msg_type = b1 & 0b11111
    pdo_count = (b0 >> 4) & 0b111
    d["dc"] = pdo_count
    d["t"] = msg_type
    d["c"] = pdo_count == 0 # control if True else data
    msg_index = int((b0 >> 1) & 0b111)
    d["i"] = msg_index
    if pdo_count:
        read_len = pdo_count*4
        pdos = get_rxb(read_len)
        d["d"] = pdos
    _ = get_rxb(4) # crc
    rev = b1 >> 6
    d["r"] = rev
    is_ext = b0 >> 7 # extended
    d["e"] = is_ext
    msg_types = control_message_types if pdo_count == 0 else data_message_types
    msg_name = msg_types[d["t"]]
    d["tn"] = msg_name
    if msg_name == "Vendor_Defined":
        parse_vdm(d)
    return d

def show_msg(d):
    ## d["h"] = header
    ## sop = 1 if header == 0xe0 else 0
    ## d["st"] = sop
    sop_str = "" if d["st"] else "'"
    # parsing the packet header
    ## d["pr"] = prole
    prole_str = "NC"[d["pr"]] if d["st"] else "R"
    drole_str = "UD"[d["dr"]] if d["st"] else "R"
    ## d["dc"] = pdo_count
    ## d["t"] = msg_type
    ## d["c"] = pdo_count == 0 # control if True else data
    message_types = control_message_types if d["c"]  else data_message_types
    ## d["i"] = msg_index
    msg_type_str = message_types[d["t"]] if d["t"] < len(message_types) else "Reserved"
    ## if pdo_count:
    ##    d["d"] = pdos
    ## d["r"] = rev
    rev_str = "123"[d["r"]]
    ## d["e"] = is_ext
    ext_str = ["std", "ext"][d["e"]]
    # msg direction
    dir_str = ">" if d["o"] else "<"
    if d["dc"]:
        # converting "41 80 00 FF A4 25 00 2C" to "FF008041 2C0025A4"
        pdo_strs = []
        pdo_data = myhex(d["d"]).split(' ')
        for i in range(len(pdo_data)//4):
           pdo_strs.append(''.join(reversed(pdo_data[(i*4):][:4])))
        pdo_str = " ".join(pdo_strs)
    else:
        pdo_str = ""
    sys.stdout.write("{} {}{}: {}; p{} d{} r{}, {}, p{}, {} {}\n".format(dir_str, d["i"], sop_str, msg_type_str, prole_str, drole_str, rev_str, ext_str, d["dc"], myhex((d["b0"], d["b1"])).replace(' ', ''), pdo_str))
    # extra parsing where possible
    if msg_type_str == "Vendor_Defined":
        print_vdm(d)
        #sys.stdout.write(str(d["d"]))
        #sys.stdout.write('\n')
    elif msg_type_str == "Source_Capabilities":
        sys.stdout.write(str(get_pdos(d)))
        sys.stdout.write('\n')
    return d

########################
#
# PDO parsing code
#
########################

pdo_types = ['fixed', 'batt', 'var', 'pps']
pps_types = ['spr', 'epr', 'res', 'res']

def parse_pdo(pdo):
    pdo_t = pdo_types[pdo[3] >> 6]
    if pdo_t == 'fixed':
        current_h = pdo[1] & 0b11
        current_b = ( current_h << 8 ) | pdo[0]
        current = current_b * 10
        voltage_h = pdo[2] & 0b1111
        voltage_b = ( voltage_h << 6 ) | (pdo[1] >> 2)
        voltage = voltage_b * 50
        peak_current = (pdo[2] >> 4) & 0b11
        return (pdo_t, voltage, current, peak_current, pdo[3])
    elif pdo_t == 'batt':
        # TODO
        return ('batt', pdo)
    elif pdo_t == 'var':
        current_h = pdo[1] & 0b11
        current = ( current_h << 8 ) | pdo[0]*10
        # TODO
        return ('var', current, pdo)
    elif pdo_t == 'pps':
        t = (pdo[3] >> 4) & 0b11
        limited = (pdo[3] >> 5) & 0b1
        max_voltage_h = pdo[3] & 0b1
        max_voltage_b = (max_voltage_h << 7) | pdo[2] >> 1
        max_voltage = max_voltage_b * 100
        min_voltage = pdo[1] * 100
        max_current_b = pdo[0] & 0b1111111
        max_current = max_current_b * 50
        return ('pps', pps_types[t], max_voltage, min_voltage, max_current, limited)

def create_pdo(pdo_t, *args):
    print(pdo_t, *args)
    assert(pdo_t in pdo_types)
    pdo = [0 for i in range(4)]
    if pdo_t == 'fixed':
        voltage, current, peak_current, pdo3 = args
        current_v = current // 10
        current_h = (current_v >> 8) & 0b11
        current_l = current_v & 0xFF
        pdo[1] = current_h
        pdo[0] = current_l
        """
        current_h = pdo[1] & 0b11
        current_b = ( current_h << 8 ) | pdo[0]
        current = current_b * 10
        """
        voltage_v = voltage // 50
        pdo[2] = (voltage_v >> 6) & 0b1111
        pdo[1] |= (voltage_v & 0b111111) << 2
        """
        voltage_h = pdo[2] & 0b1111
        voltage_b = ( voltage_h << 6 ) | (pdo[1] >> 2)
        voltage = voltage_b * 50
        """
        pdo[2] |= (peak_current & 0b11) << 4
        peak_current = (pdo[2] >> 4) & 0b11
        pdo[3] = pdo3
        pdo[3] |= pdo_types.index(pdo_t) << 6
    elif pdo_t == 'batt':
        raise Exception("Batt PDO formation not implemented yet!")
    elif pdo_t == 'var':
        raise Exception("Variable PDO formation not implemented yet!")
    elif pdo_t == 'pps':
        """t = (pdo[3] >> 4) & 0b11
        limited = (pdo[3] >> 5) & 0b1
        max_voltage_h = pdo[3] & 0b1
        max_voltage_b = (max_voltage_h << 7) | pdo[2] >> 1
        max_voltage = max_voltage_b * 100
        min_voltage = pdo[1] * 100
        max_current_b = pdo[0] & 0b1111111
        max_current = max_current_b * 50
        return ('pps', pps_types[t], max_voltage, min_voltage, max_current, limited)"""
        raise Exception("PPS PDO formation not implemented yet!")
    print(parse_pdo(bytes(pdo)))
    return pdo

def get_pdos(d):
    pdo_list = []
    pdos = d["d"]
    for pdo_i in range(d["dc"]):
        pdo_bytes = pdos[(pdo_i*4):][:4]
        #print(myhex(pdo_bytes))
        parsed_pdo = parse_pdo(pdo_bytes)
        pdo_list.append(parsed_pdo)
    return pdo_list

########################
#
# Command sending code
# and simple commands
#
########################

def send_command(command, data, msg_id=None, rev=0b10, power_role=0, data_role=0):
    msg_id = increment_msg_id() if msg_id is None else msg_id
    sop_seq = [0x12, 0x12, 0x12, 0x13, 0x80]
    eop_seq = [0xff, 0x14, 0xfe, 0xa1]
    obj_count = len(data) // 4

    header = [0, 0] # hoot hoot !

    header[0] |= rev << 6 # PD revision
    header[0] |= (data_role & 0b1) << 5 # PD revision
    header[0] |= (command & 0b11111)

    header[1] = power_role & 0b1
    header[1] |= (msg_id & 0b111) << 1 # message ID
    header[1] |= obj_count << 4

    message = header+data

    sop_seq[4] |= len(message)

    fusb.tx_byte(FUSB302_I2C_SLAVE_ADDR, TCPC_REG_FIFOS, bytes(sop_seq) )
    fusb.tx_byte(FUSB302_I2C_SLAVE_ADDR, TCPC_REG_FIFOS, bytes(message) )
    fusb.tx_byte(FUSB302_I2C_SLAVE_ADDR, TCPC_REG_FIFOS, bytes(eop_seq) )

    sent_messages.append(message)

def soft_reset():
    send_command(0b01101, [])
    reset_msg_id()

########################
#
# Helper functions
#
########################

def myhex(b, j=" "):
    l = []
    for e in b:
        e = hex(e)[2:].upper()
        if len(e) < 2:
            e = ("0"*(2-len(e)))+e
        l.append(e)
    return j.join(l)

def mybin(b, j=" "):
    l = []
    for e in b:
        e = bin(e)[2:].upper()
        if len(e) < 8:
            e = ("0"*(8-len(e)))+e
        l.append(e)
    return j.join(l)
