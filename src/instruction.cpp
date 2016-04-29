/*
 * instruction.cpp -- implementation of the Instruction class
 * ------
 * Author: krab
 * Version: 0.1
 */
#include "instruction.h"

// Cstr
Instruction::Instruction(uint32_t content) {
  _content = content;
}

// Dstr
Instruction::~Instruction() {
}

// Copy operator
Instruction& Instruction::operator=(const Instruction& i) {
  if (&i != this) {
    _content = i._content;
  }
  return *this;
}

// Content getter and setter
uint32_t Instruction::getContent() const {
  return _content;
}

void Instruction::setContent(uint32_t val) {
  _content = val;
}

// Get a field
uint32_t Instruction::getField(uint32_t from, uint32_t size) const {
  return sub(_content, from, size);
}

// Make instructions
Instruction Instruction::makeInstruction(uint32_t op, uint32_t disp) {
  uint32_t inst = (op << 30) | (disp & 0x3FFFFFFF);
  Instruction in(inst);
  return in;
}

Instruction Instruction::makeInstruction(uint32_t op, uint32_t rd, uint32_t op2, uint32_t imm22) {
  uint32_t inst =
    ( op                  << 30) |
    ((rd    & 0x0000001F) << 25) |
    ((op2   & 0x00000007) << 22) |
     (imm22 & 0x003FFFFF);
  Instruction in(inst);
  return in;
}

Instruction Instruction::makeInstruction(uint32_t op, uint32_t a, uint32_t cond, uint32_t op2, uint32_t disp22) {
  uint32_t inst =
    ( op                  << 30) |
    ((a     & 0x00000001) << 29) |
    ((cond  & 0x0000000F) << 25) |
    ((op2   & 0x00000007) << 22) |
    (disp22 & 0x003FFFFF);
  Instruction in(inst);
  return in; 
}

Instruction Instruction::makeInstruction(uint32_t op, uint32_t rd, uint32_t op3, uint32_t rs1, uint32_t i, uint32_t asi, uint32_t rs2OrSimm13) {
  uint32_t inst =
    ( op                  << 30) |
    ((rd    & 0x0000001F) << 25) |
    ((op3   & 0x0000003F) << 19) |
    ((rs1   & 0x0000001F) << 14) |
    ((i     & 0x00000001) << 13);

  if ((i & 0x00000001) == 1) {
    inst |= rs2OrSimm13 & 0x00001FFF;
  } else {
    inst |=
      ((asi & 0x0000007F) << 5) |
       (rs2OrSimm13 & 0x0000001F);
  }

  Instruction in(inst);
  return in;
}

Instruction Instruction::makeInstruction(uint32_t op, uint32_t rd, uint32_t op3, uint32_t rs1, uint32_t opf, uint32_t rs2) {
  uint32_t inst =
    ( op                  << 30) |
    ((rd    & 0x0000001F) << 25) |
    ((op3   & 0x0000003F) << 19) |
    ((rs1   & 0x0000001F) << 14) |
    ((opf   & 0x000001FF) << 5) |
     (rs2   & 0x0000001F);

  Instruction in(inst);
  return in;
}



