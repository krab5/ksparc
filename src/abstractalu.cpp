/*
 * abstractalu.cpp -- implementation of the AbstractALU
 * ------
 * Author: krab
 * Version: 0.1
 */
#include "abstractalu.h"

// Cstr
AbstractALU::AbstractALU(SpecialRegister* psr, Register* y) {
  _psr = psr;
  _y = y;
}

// Dstr
AbstractALU::~AbstractALU() {
}

// Condition codes
bool AbstractALU::getN() const {
  return _psr->getField(PSR_ICC_N) == 1;
}

bool AbstractALU::getZ() const {
  return _psr->getField(PSR_ICC_Z) == 1;
}

bool AbstractALU::getC() const {
  return _psr->getField(PSR_ICC_C) == 1;
}

bool AbstractALU::getV() const {
  return _psr->getField(PSR_ICC_V) == 1;
}

void AbstractALU::setN(bool v) {
  return _psr->setField(PSR_ICC_N, v ? 1 : 0);
}

void AbstractALU::setZ(bool v) {
  return _psr->setField(PSR_ICC_Z, v ? 1 : 0);
}

void AbstractALU::setC(bool v) {
  return _psr->setField(PSR_ICC_C, v ? 1 : 0);
}

void AbstractALU::setV(bool v) {
  return _psr->setField(PSR_ICC_V, v ? 1 : 0);
}

// Y getters and setters
Register* AbstractALU::getY() {
  return _y;
}

uint32_t AbstractALU::readY() const {
  return _y->read();
}

void AbstractALU::writeY(uint32_t v) {
  _y->write(v);
}


