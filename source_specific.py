########################
#
# PSU request processing code
#
########################

def send_advertisement(psu_advertisement):
    #data = [bytes(a) for a in psu_advertisement]
    data = psu_advertisement
    send_command(0b1, data, power_role=1, data_role=1)

def process_psu_request(psu_advertisement, d):
    print(d)
    profile = ((d["d"][3] >> 4)&0b111)-1
    print("Selected profile", profile)
    if profile not in range(len(psu_advertisement)):
        set_power_rail('off')
    else:
        send_command(0b11, [], power_role=1, data_role=1) # Accept
        sleep(0.1)
        if profile == 0:
            set_power_rail('5V')
        elif profile == 1:
            set_power_rail('VIN')
        send_command(0b110, [], power_role=1, data_role=1) # PS_RDY

def source_flow():
  global psu_advertisement, advertisement_counter, sent_messages
  psu_advertisement = create_pdo('fixed', 5000, 1500, 0, 8) + \
                       create_pdo('fixed', 19000, 5000, 0, 0)
  counter = 0
  reset_msg_id()
  sleep(0.3)
  print("sending advertisement")
  send_advertisement(psu_advertisement)
  advertisement_counter = 1
  profile_selected = False
  try:
   timeout = 0.00001
   while True:
    if rxb_state()[0] == 0: # buffer non-empty
        d = get_message()
        msg_types = control_message_types if d["c"] else data_message_types
        msg_name = msg_types[d["t"]]
        # now we do things depending on the message type that we received
        if msg_name == "GoodCRC": # example
            print("GoodCRC")
        elif msg_name == "Request":
            profile_selected = True
            process_psu_request(psu_advertisement, d)
        show_msg(d)
    for message in sent_messages:
        sys.stdout.write('> ')
        sys.stdout.write(myhex(message))
        sys.stdout.write('\n')
    sent_messages = []
    sleep(timeout) # so that ctrlc works
    counter += 1
    if counter == 10000:
        counter = 0
        if not profile_selected and advertisement_counter < 30:
            print("sending advertisement")
            send_advertisement(psu_advertisement)
            advertisement_counter += 1
    if int_g() == 0:
        i = interrupts()
        print(i)
        i_reg = i[2]
        if i_reg & 0x80: # I_VBUSOK
            print("I_VBUSOK")
            #pass # just a side effect of vbus being attached
        if i_reg & 0x40: # I_ACTIVITY
            print("I_ACTIVITY")
            pass # just a side effect of CC comms I think?
        if i_reg & 0x20: # I_COMP_CHNG
            print("I_COMP_CHNG")
            # this is where detach can occur, let's check
            cc = find_cc(fn=measure_source)
            if cc == 0:
                print("Disconnect detected!")
                return # we exiting this
        if i_reg & 0x10: # I_CRC_CHK
            pass # new CRC, just a side effect of CC comms
        if i_reg & 0x8: # I_ALERT
            print("I_ALERT")
            x = i2c.readfrom_mem(0x22, 0x41, 1)[0]
            print(bin(x))
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
