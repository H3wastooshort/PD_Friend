//stuff only relevant to SRC
class PDStack_SRC : public PDStack {
public:
PDStack_SRC(FUSB302 fusb) : PDStack(fusb) {};

void init_src() {
  init_universal();
  fusb->set_controls_source();
  fusb->set_roles(true, true);
  fusb->disable_pulldowns();
}

bool attach_src() {
  fusb->disable_pulldowns();
  fusb->set_wake(true);
  fusb->enable_pullups();
  fusb->set_mdac(0b111111);

  uint8_t cc = fusb->find_cc_source();

  if (cc == 0) return false;
  //else
  fusb->set_cc(cc);
  return true;
}
};