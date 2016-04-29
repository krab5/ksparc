/*
 * windowregisters.cpp -- implement the WindowRegister class
 */
#include "windowregisters.h"

using namespace std;

// Cstr
WindowRegisters::WindowRegisters(const uint32_t wsize, SpecialRegister* psr, SpecialRegister* wim) : _wsize(wsize) {
  _psr = psr;
  _wim = wim;

  /*
   * Registers :
   * [0..NREGGLOB-1]                                                                      globals
   * [cwp*(NREGIO+NREGLOC)+(NREGGLOB..NREGGLOB+NREGIO-1)]                                 outs
   * [cwp*(NREGIO+NREGLOC)+(NREGGLOB+NREGIO..NREGGLOB+NREGIO+NREGLOC-1)]                  locals
   * [cwp*(NREGIO+NREGLOC)+(NREGGLOB+NREGIO+NREGLOC..NREGGLOB+NREGIO+NREGLOC+NREGIO-1)]   ins
   */
  _registers = new Register[NREGGLOB+_wsize*NREGIO+_wsize*NREGLOC];

  _registers[0].write(0x00000000); // %g0 always equals to 0
}

// Dstr
WindowRegisters::~WindowRegisters() {
}

// Window size acessor
uint32_t WindowRegisters::getWindowSize() const {
  return _wsize;
}

// Get a pointer to a register
Register* WindowRegisters::get(uint32_t nb) {
  uint32_t cwp = _psr->getField(PSR_CWP);
  if (nb == 0) {
    Register* r = new Register();
    r->write(0);
    return r; // these registers should always be given by copy
  } else if (nb < NREGGLOB) { // Want to access global registers
    return &_registers[nb];
  } else if (cwp == _wsize - 1 && nb >= NREGIO+NREGLOC+NREGGLOB) {
    // Want to access input of the last window, which are the
    // outputs of the first
    return &_registers[nb-NREGLOC-NREGIO];
  } else {
    return &_registers[cwp*(NREGIO+NREGLOC)+nb];
  }
}

// Read write operations
uint32_t WindowRegisters::read(uint32_t nb) const {
  uint32_t cwp = _psr->getField(PSR_CWP);
  if (nb == 0) {
    return 0;
  } else if (nb < NREGGLOB) { // Want to access global registers
    return _registers[nb].read();
  } else if (cwp == _wsize - 1 && nb >= NREGIO+NREGLOC+NREGGLOB) {
    // Want to access input of the last window, which are the
    // outputs of the first
    return _registers[nb-NREGLOC-NREGIO].read();
  } else {
    return _registers[cwp*(NREGIO+NREGLOC)+nb].read();
  }
}

void WindowRegisters::write(uint32_t nb, uint32_t data) {
  return get(nb)->write(data);
}

// Save and restore context
void WindowRegisters::save() {
  uint32_t cwp = _psr->getField(PSR_CWP);
  if (cwp == _wsize - 1) {// overflow !
    _wim->setField(_wsize, 0, 1);
    cwp = 0;
  } else {
    cwp = cwp + 1;
  }

  _psr->setField(PSR_CWP, cwp);
}

void WindowRegisters::restore() {
  uint32_t cwp = _psr->getField(PSR_CWP);
  if (cwp == 0) {// underflow !
    _wim->setField(_wsize, 0, 1);
    cwp = _wsize - 1;
  } else {
    cwp = cwp - 1;
  }

  _psr->setField(PSR_CWP, cwp);
}



