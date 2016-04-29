/*
 * register.cpp -- implements the register class
 * ------
 * Author: krab
 * Version: 0.1
 */
#include "register.h"

// Default cstr
Register::Register() {
  _content = new uint32_t;
}

// Cstr
Register::Register(uint32_t* content) {
  _content = content;
}

// Dstr
Register::~Register() {
}

// Acessor
uint32_t Register::read() const {
  return *_content;
}

// Mutator
void Register::write(const uint32_t value) {
  *_content = value;
}



