/*
 * specialregister.cpp -- implements the SpecialRegister class
 */
#include "specialregister.h"

// Cstr default
SpecialRegister::SpecialRegister() : Register() {
}

// Cstr
SpecialRegister::SpecialRegister(uint32_t* content) :
  Register(content) {

}

// Dstr
SpecialRegister::~SpecialRegister() {

}

// Get a field
uint32_t SpecialRegister::getField(uint32_t from, uint32_t size) const {
  return sub(read(), from, size);
}

// Set a field
void SpecialRegister::setField(uint32_t from, uint32_t size, uint32_t value) {
  // It may seems complicated, but basically we :
  // - read the current value
  // - set the bits corresponding to the field we want to modify to 0
  // - or' this last number with the new value we want, shifted so it fit
  write((read() & (~((0xFFFFFFFF >> (32 - size)) << from))) | (value << from));
}


 
