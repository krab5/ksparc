/*
 * abstractsparcengine.cpp -- implementation of the AbstractSparcEngine class
 * ------
 * Author: krab
 * Version: 0.1
 */
#include "abstractsparcengine.h"

// Cstr
AbstractSparcEngine::AbstractSparcEngine(
    AbstractMemory* mem,
    AbstractALU* alu,
    /*AbstractFPU* fpu,*/
    WindowRegisters* registers,
    SpecialRegister* psr,
    SpecialRegister* wim,
    SpecialRegister* tbr,
    SpecialRegister* y,
    SpecialRegister* pc,
    SpecialRegister* npc,
    SpecialRegister* fsr
    ) {
  _mem = mem;
  _alu = alu;
  // _fpu = fpu;
  _reg = registers;
  _psr = psr;
  _wim = wim;
  _tbr = tbr;
  _y = y;
  _pc = pc;
  _npc = npc;
  _fsr = fsr;
}

// Dstr
AbstractSparcEngine::~AbstractSparcEngine() {
}

// Component getters
AbstractMemory* AbstractSparcEngine::memory() {
  return _mem;
}

AbstractALU* AbstractSparcEngine::alu() {
  return _alu;
}

/*AbstractFPU* AbstractSparcEngine::fpu() {
 * return _fpu;
}*/

WindowRegisters* AbstractSparcEngine::registers() {
  return _reg;
}

SpecialRegister* AbstractSparcEngine::psr() {
  return _psr;
}

SpecialRegister* AbstractSparcEngine::wim() {
  return _wim;
}

SpecialRegister* AbstractSparcEngine::tbr() {
  return _tbr;
}

SpecialRegister* AbstractSparcEngine::y() {
  return _y;
}

SpecialRegister* AbstractSparcEngine::pc() {
  return _pc;
}

SpecialRegister* AbstractSparcEngine::npc() {
  return _npc;
}

SpecialRegister* AbstractSparcEngine::fsr() {
  return _fsr;
}




