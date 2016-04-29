/*
 * abstractmemory.cpp -- implementation of memory.h
 * ------
 * Author: krab
 * Version: 0.1
 */
#include "abstractmemory.h"

// Implements the BadAlignmentException class
AbstractMemory::BadAlignmentException::BadAlignmentException() : std::logic_error("This device has encountered a bad alignment problem") {
}

AbstractMemory::BadAlignmentException::~BadAlignmentException() {
}

// Implements the AbstractMemory class
// Cstr
AbstractMemory::AbstractMemory(const uint32_t size) : _size(size) {
}

// Dstr
AbstractMemory::~AbstractMemory() {
}

// Returns the size of the memory
uint32_t AbstractMemory::getSize() const {
  return _size;
}

// Read functions
// Read a byte
uint8_t AbstractMemory::readByte(uint32_t address) const {
  uint8_t res[1];
  read(address, 1, res);
  return res[0];
}

// Read a byte into a register
void AbstractMemory::readByte(uint32_t address, Register* rd) const {
  rd->write(0x0000000);
  rd->write(readByte(address));
}

// Read a halfword
uint16_t AbstractMemory::readHalfword(uint32_t address) const {
  uint8_t res[2];
  read(address, 2, res);
  return ((uint16_t)res[1] << 8)
    +  ((uint16_t)res[0]);
}

// Read a halfword into a register
void AbstractMemory::readHalfword(uint32_t address, Register* rd) const {
  rd->write(0x00000000);
  rd->write(readHalfword(address));
}

// Read a word
uint32_t AbstractMemory::readWord(uint32_t address) const {
  uint8_t res[4];
  read(address, 4, res);
  return ((uint32_t)res[0] << 24) + ((uint32_t)res[1] << 16)
    + ((uint32_t)res[2] << 8 ) + ((uint32_t)res[3]);
}

// Read a word into a register
void AbstractMemory::readWord(uint32_t address, Register* rd) const {
  rd->write(0x00000000);
  rd->write(readWord(address));
}

// Read a double word
uint64_t AbstractMemory::readDoubleword(uint32_t address) const {
  uint32_t res1 = readWord(address);   // most significant
  uint32_t res2 = readWord(address+4); // less significant
  return ((uint64_t)res1 << 32) + ((uint64_t)res2);
}

// Read a double word into two registers
void AbstractMemory::readDoubleword(uint32_t address, Register* rdeven, Register* rdodd) const {
  rdeven->write(readWord(address));
  rdodd->write(readWord(address+4));
}

// Read an instruction
Instruction AbstractMemory::readInstruction(uint32_t address) const {
  uint32_t inst = readWord(address);
  return Instruction(inst);
}

// Write functions
// Write a byte
void AbstractMemory::writeByte(uint32_t address, uint8_t data) {
  uint8_t d[1] = { data };
  write(address, d, 1);
}

// Write a byte from a register
void AbstractMemory::writeByte(uint32_t address, const Register* rd) {
  uint32_t rdata = rd->read();
  uint8_t data = (uint8_t)(rdata & 0x000000FF);
  writeByte(address, data);
}

// Write a half word
void AbstractMemory::writeHalfword(uint32_t address, uint16_t data) {
  uint8_t d[2] = {
    (uint8_t)((data & 0xFF00) >> 8),
    (uint8_t)( data & 0x00FF)
  };
  write(address, d, 2);
}

// Write a half word from register
void AbstractMemory::writeHalfword(uint32_t address, const Register* rd) {
  uint32_t rdata = rd->read();
  uint16_t data = (uint16_t)(rdata & 0x0000FFFF);
  writeHalfword(address, data);
}

// Write a word
void AbstractMemory::writeWord(uint32_t address, uint32_t data) {
  uint8_t d[4] = {
    (uint8_t)((data & 0xFF000000) >> 24),
    (uint8_t)((data & 0x00FF0000) >> 16),
    (uint8_t)((data & 0x0000FF00) >> 8),
    (uint8_t)( data & 0x000000FF)
  };
  write(address, d, 4);
}

// Write a word from register
void AbstractMemory::writeWord(uint32_t address, const Register* rd) {
  writeWord(address, rd->read());
}

// Write a double word
void AbstractMemory::writeDoubleword(uint32_t address, uint64_t data) {
  writeWord(address,   (uint32_t)((data & 0xFFFFFFFF00000000) >> 32));
  writeWord(address+1, (uint32_t)((data & 0x00000000FFFFFFFF)));
}

// Write a double word from two registers
void AbstractMemory::writeDoubleword(uint32_t address, const Register* rdeven, const Register* rdodd) {
  writeWord(address,  rdeven->read());
  writeWord(address+1, rdodd->read());
}



