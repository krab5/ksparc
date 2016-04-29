/*
 * simplememory.cpp -- implement the SimpleMemory class
 */
#include "simplememory.h"

// Cstr
SimpleMemory::SimpleMemory(uint32_t size) : AbstractMemory(size) {
  _content = new uint8_t[size];
}

// Dstr
SimpleMemory::~SimpleMemory() {
  delete _content;
}

// Read
void SimpleMemory::read(uint32_t address, uint32_t size, uint8_t* data) const {
  for (uint32_t i = 0; i < size; i++)
    data[i] = _content[address+i];
}

// Write
void SimpleMemory::write(uint32_t address, uint8_t* data, uint32_t size) {
  for (uint32_t i = 0; i < size; i++)
    _content[address+i] = data[i];
}



