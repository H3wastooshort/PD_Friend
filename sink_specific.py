########################
#
# PDO request code
#
########################

def request_fixed_pdo(num, current, max_current):
    pdo = [0 for i in range(4)]

    max_current_b = max_current // 10
    max_current_l = max_current_b & 0xff
    max_current_h = max_current_b >> 8
    pdo[0] = max_current_l
    pdo[1] |= max_current_h

    current_b = current // 10
    current_l = current_b & 0x3f
    current_h = current_b >> 6
    pdo[1] |= current_l << 2
    pdo[2] |= current_h

    pdo[3] |= (num+1) << 4 # object position
    pdo[3] |= 0b1 # no suspend

    send_command(0b00010, pdo)

def request_pps_pdo(num, voltage, current):
    pdo = [0 for i in range(4)]

    current = current // 50
    pdo[0] = current & 0x7f

    voltage = voltage // 20
    voltage_l = (voltage & 0x7f)
    voltage_h = (voltage >> 7) & 0x1f
    pdo[1] |= voltage_l << 1
    pdo[2] = voltage_h

    pdo[3] |= (num+1) << 4 # object position
    pdo[3] |= 0b1 # no suspend

    send_command(0b00010, pdo)

########################
#
# VDM parsing and response code
#
########################

vdm_commands = [
    "Reserved",
    "Discover Identity",
    "Discover SVIDs",
    "Discover Modes",
    "Enter Mode",
    "Exit Mode",
    "Attention"]

svids = {
    0xff00: 'SID',
    0xff01: 'DisplayPort',
}

dp_commands = {
    0x10: "DP Status Update",
    0x11: "DP Configure"}

vdm_cmd_types = ["REQ", "ACK", "NAK", "BUSY"]

# reply-with-hardcoded code

def react_vdm(d):
    if d["vdm_s"]:
        cmd_type = d["vdm_ct"]
        command_name = d["vdm_cn"]
        # response vdm params
        rd = {}
        # all same params as the incoming message, save for the command type
        for key in ["vdm_s", "vdm_sv", "vdm_c", "vdm_v", "vdm_o"]:
            rd[key] = d[key]
        # command type is ACK and not REQ for all command replies
        rd["vdm_ct"] = 1 # ACK
        if command_name == "Discover Identity":
            # discover identity response with "we are an altmode adapter yesyes"
            data = list(b'A\xA0\x00\xff\xa4%\x00,\x00\x00\x00\x00\x01\x00\x00\x00\x0b\x00\x00\x11')
            r = create_vdm_data(rd, data[4:])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("a") # debug stuff
        elif command_name == "Discover SVIDs":
            data = list(b'B\xA0\x00\xff\x00\x00\x01\xff')
            r = create_vdm_data(rd, data[4:])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("b")
        elif command_name == "Discover Modes":
            #data = list(b'C\xA0\x01\xff\x45\x04\x00\x00')
            data = list(b'C\xA0\x01\xff\x05\x0c\x00\x00')
            r = create_vdm_data(rd, data[4:])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("c")
        elif command_name == "Enter Mode":
            data = list(b'D\xA1\x01\xff')
            r = create_vdm_data(rd, [])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("d")
        elif command_name == "DP Status Update":
            #data = list(b'P\xA1\x01\xff\x1a\x00\x00\x00')
            data = list(b'P\xA1\x01\xff\x9a\x00\x00\x00')
            r = create_vdm_data(rd, data[4:])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("e")
        elif command_name == "DP Configure":
            data = list(b'Q\xA1\x01\xff')
            r = create_vdm_data(rd, [])
            print(r)
            print(data)
            send_command(d["t"], r)
            #sys.stdout.write("f")
    # no unstructured vdm processing at this time

def create_vdm_data(d, data):
    """
    Creates the VDM header (PDO) from a dict with pre-supplied data and an additional data list.
    """
    l = 4 + len(data)
    vdm = bytearray(l)
    for i in data:
        vdm[i+4] = i
    # most basic vdm flags
    vdm_s = d["vdm_s"]
    vdm[1] |= vdm_s << 7
    vdm_sv = d["vdm_sv"]
    vdm[2] = vdm_sv & 0xff
    vdm[3] = vdm_sv >> 8
    # can't build unstructured vdms yet
    if vdm_s:
        # building structured vdm
        # vdm command
        vdm_c = d["vdm_c"]
        vdm[0] |= (vdm_c & 0b11111)
        # vdm command type
        vdm_ct = d["vdm_ct"]
        vdm[0] |= (vdm_ct & 0b11) << 6
        # default version codes set to 0b01; 0b00
        vdm_v = d.get("vdm_v", 0b0100)
        vdm[1] |= (vdm_v & 0b1111) << 3
        # object position
        vdm_o = d.get("vdm_o", 0)
        vdm[1] |= vdm_o & 0b111
    else:
        raise NotImplementedError
    return bytes(vdm)

def parse_vdm(d):
    data = d['d']
    is_structured = data[1] >> 7
    d["vdm_s"] = is_structured
    svid = (data[3] << 8) + data[2]
    d["vdm_sv"] = svid
    svid_name = svids.get(svid, "Unknown ({})".format(hex(svid)))
    d["vdm_svn"] = svid_name
    if is_structured:
        # version: major and minor
        version_bin = (data[1] >> 3) & 0xf
        d["vdm_v"] = version_bin
        obj_pos = data[1] & 0b111
        d["vdm_o"] = obj_pos
        cmd_type = data[0]>>6
        d["vdm_ct"] = cmd_type
        command = data[0] & 0b11111
        d["vdm_c"] = command
        if command > 15:
            command_name = "SVID specific {}".format(bin(command))
            if svid_name == "DisplayPort":
                command_name = dp_commands.get(command, command_name)
        else:
            command_name = vdm_commands[command] if command < 7 else "Reserved"
        d["vdm_cn"] = command_name
        #if svid_name == "DisplayPort":
        #    parse_dp_command(version_str())
    else:
        vdmd = [data[1] & 0x7f, data[0]]
        d["vdm_d"] = vdmd

vdm_dp_pin_assg = {
 0b1:"A",
 0b10:"B",
 0b100:"C",
 0b1000:"D",
 0b10000:"E",
 0b100000:"F",
}

vdm_dp_port_cap = [
 "RES",
 "UFP",
 "DFP",
 "UFP&DFP"
]

vdm_dp_port_conn = [
 "NC",
 "UFP",
 "DFP",
 "UFP&DFP"
]

vdm_dp_port_conf = [
 "USB",
 "DFP",
 "DFP",
 "RES"
]

vdm_dp_sgn = {
 0b1:"DP",
 0b10:"USBg2",
 0b100:"RES1",
 0b1000:"RES2"
}

def print_vdm(d):
    if d["vdm_s"]:
        svid_name = d["vdm_svn"]
        version_str = mybin([d["vdm_v"]])[4:]
        objpos_str = mybin([d["vdm_o"]])[5:]
        cmd_type_name = vdm_cmd_types[d["vdm_ct"]]
        cmd_name = d["vdm_cn"]
        sys.stdout.write("VDM: str, m{} v{} o{}, ct{}: {}\n".format(svid_name, version_str, objpos_str, cmd_type_name, cmd_name))
        if svid_name == "DisplayPort":
            if cmd_name == "Discover Modes" and cmd_type_name == "ACK":
                msg = d['d'][4:]
                # port capability (bits 0:1)
                port_cap = msg[0] & 0b11
                vdm_dp_port_cap_s = vdm_dp_port_cap[port_cap]
                # signaling (bits 5:2)
                sgn = (msg[0] >> 2) & 0b1111
                sgn_s = []
                for p in vdm_dp_sgn.keys():
                    if sgn & p:
                        sgn_s.append(vdm_dp_sgn[p])
                sgn_s = ",".join(sgn_s)
                # receptacle indication (bit 6)
                r_i = (msg[0] >> 6) & 0b1
                r_s = "re" if r_i else "pl"
                # usb2 signaling (bit 7)
                u2_i = (msg[0] >> 7) & 0b1
                u2_s = "n" if u2_i else "y"
                # dfp pin assignments (bits 15:8)
                dfp_assy_n = msg[1]
                dfp_assy_s = ""
                for p in vdm_dp_pin_assg.keys():
                    if dfp_assy_n & p:
                        dfp_assy_s += vdm_dp_pin_assg[p]
                # dfp pin assignments (bits 23:16)
                ufp_assy_n = msg[2]
                ufp_assy_s = ""
                for p in vdm_dp_pin_assg.keys():
                    if ufp_assy_n & p:
                        ufp_assy_s += vdm_dp_pin_assg[p]
                #res_byte = msg[3] # (bites 31:24, has to be 0)
                sys.stdout.write("\tModes: p_cap:{} sgn:{} ri:{} u2:{} d_ass:{} u_ass:{}\n".format(vdm_dp_port_cap_s, sgn_s, r_s, u2_s, dfp_assy_s, ufp_assy_s))
            elif cmd_name == "DP Status Update":
                msg = d['d'][4:]
                # dfp/ufp connected (bits 0:1)
                conn = msg[0] & 0b11
                conn_s = vdm_dp_port_conn[conn]
                # power (bit 2)
                pwr = (msg[0] >> 2) & 0b1
                pwr_s = "d" if pwr else "n"
                # enabled (bit 3)
                en = (msg[0] >> 3) & 0b1
                en_s = "y" if en else "n"
                # multi-function (bit 4)
                mf = (msg[0] >> 4) & 0b1
                mf_s = "p" if mf else "n"
                # usb switch req (bit 5)
                usw = (msg[0] >> 5) & 0b1
                usw_s = "r" if usw else "n"
                # dp exit req (bit 6)
                dpe = (msg[0] >> 6) & 0b1
                dpe_s = "r" if dpe else "n"
                # HPD state (bit 7)
                hpd = (msg[0] >> 7) & 0b1
                hpd_s = "h" if hpd else "l"
                # IRQ state (bit 8)
                irq = msg[1] & 0b1
                irq_s = str(irq)
                sys.stdout.write("\tStatus: conn:{} pwr:{} en:{} mf:{} usw:{} dpe:{} hpd:{} irq:{}\n".format(conn_s, pwr_s, en_s, mf_s, usw_s, dpe_s, hpd_s, irq_s))
            if cmd_name == "DP Configure" and cmd_type_name == "REQ":
                msg = d['d'][4:]
                # select configuration (bits 0:1)
                conf = msg[0] & 0b11
                conf_s = vdm_dp_port_conf[conf]
                # signaling (bits 5:2)
                sgn = (msg[0] >> 2) & 0b1111
                sgn_s = []
                for p in vdm_dp_sgn.keys():
                    if sgn & p:
                        sgn_s.append(vdm_dp_sgn[p])
                sgn_s = ",".join(sgn_s)
                if not sgn_s:
                    sgn_s = "UNSP"
                # reserved (bits 7:6)
                # ufp pin assignments (bits 15:8)
                ufp_assy_n = msg[1]
                ufp_assy_s = ""
                for p in vdm_dp_pin_assg.keys():
                    if ufp_assy_n & p:
                        ufp_assy_s += vdm_dp_pin_assg[p]
                #res_bytes = msg[2:] # (bytes 31:24, has to be 0)
                sys.stdout.write("\tConfigure: conf:{} sgn:{} p_ass:{}\n".format(conf_s, sgn_s, ufp_assy_s))
            #di = d
            #breakpoint()
    else:
        sys.stdout.write("VDM: unstr, m{}, d{}".format(svid_name, myhex(d["vdm_d"])))

########################
#
# Power profile selection example code
#
########################

def select_pdo_for_resistance(pdos, resistance = 8):
    # finding a PDO with maximum extractable power
    # for a given static resistance,
    # while making sure that we don't overcurrent the PSU
    # calculation storage lists
    power_levels = []
    currents = []
    for pdo in pdos:
        if pdo[0] != 'fixed': # skipping variable PDOs for now
            # keeping indices in sync
            power_levels.append(0); currents.append(0)
            continue
        t, voltage, max_current, oc, flags = pdo
        voltage = voltage / 1000
        max_current = max_current / 1000
        # calculating the power needed
        current = voltage / resistance
        current = current * 1.10 # adding 10% leeway
        if current > max_current: # current too high, skipping
            # keeping indices in sync
            power_levels.append(0); currents.append(0)
            continue
        power = voltage * current
        power_levels.append(power)
        currents.append(int(current*1000))
    # finding the maximum power level
    i = power_levels.index(max(power_levels))
    # returning the PDO index + current we'd need
    return i, currents[i]

def select_pdo_for_voltage(pdos, voltage=5, current=1000):
    for i, pdo in enumerate(pdos):
        if pdo[0] != 'fixed': # skipping variable PDOs
            continue
        t, pdo_voltage, max_current, oc, flags = pdo
        if pdo_voltage//1000 == voltage:
            current = current if current else max_current
            return i, current

def sink_flow():
  global pdo_requested, pdos, sent_messages
  reset_msg_id()
  try:
   timeout = 0.00001
   while True:
    if rxb_state()[0] == 0: # buffer non-empty
        d = get_message()
        msg_types = control_message_types if d["c"] else data_message_types
        msg_name = msg_types[d["t"]]
        # now we do things depending on the message type that we received
        if msg_name == "GoodCRC": # example
            pass # print("GoodCRC")
        elif msg_name == "Source_Capabilities":
            # need to request a PDO!
            pdos = get_pdos(d)
            pdo_i, current = select_pdo(pdos)
            # sending a message, need to increment message id
            request_fixed_pdo(pdo_i, current, current)
            # print("PDO requested!")
            pdo_requested = True
            sys.stdout.write(str(pdos))
            sys.stdout.write('\n')
        elif msg_name in ["Accept", "PS_RDY"]:
            print(get_adc_vbus(), "V")
        elif msg_name == "Vendor_Defined":
            parse_vdm(d)
            react_vdm(d)
        show_msg(d)
        for message in sent_messages:
            sys.stdout.write('> ')
            sys.stdout.write(myhex(message))
            sys.stdout.write('\n')
        sent_messages = []
    sleep(timeout) # so that ctrlc works
    if int_g() == 0:
        # needs sink detach processing here lmao
        i = interrupts()
        print(i)
        i_reg = i[2]
        if i_reg & 0x80: # I_VBUSOK
            pass # just a side effect of vbus being attached
        if i_reg & 0x40: # I_ACTIVITY
            print("I_ACTIVITY")
            pass # just a side effect of CC comms I think?
        if i_reg & 0x20: # I_COMP_CHNG
            print("I_COMP_CHNG")
            cc = find_cc(fn=measure_sink)
            if cc == 0:
                print("Disconnect detected!")
                return # we exiting this
        if i_reg & 0x10: # I_CRC_CHK
            pass # new CRC, just a side effect of CC comms
        if i_reg & 0x8: # I_ALERT
            print("I_ALERT")
        if i_reg & 0x4: # I_WAKE
            print("I_WAKE")
        if i_reg & 0x2: # I_COLLISION
            print("I_COLLISION")
        if i_reg & 0x1: # I_BC_LVL
            print("I_BC_LVL")
  except KeyboardInterrupt:
    print("CtrlC")
    sleep(1)
    raise
