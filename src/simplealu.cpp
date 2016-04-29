/*
 * simplealu.cpp -- implementation of the SimpleALU class
 */
#include "simplealu.h"

// Cstr
SimpleALU::SimpleALU(SpecialRegister* psr, Register* y) : AbstractALU(psr, y) {
}

// Dstr
SimpleALU::~SimpleALU() {
}

// Shift left for two registers
void SimpleALU::shiftLeftDual(uint32_t nb, Register* reven, Register* rodd) {
  uint32_t re = reven->read();
  uint32_t ro = rodd->read();
  // In the even register (the MSB), we shift left and then we add the bits lost from shifting the odd register (LSB)
  reven->write((re << nb) | ((ro & (0xFFFFFFFF << (32 - nb))) >> (32 - nb)));
  rodd->write(ro << nb);
}

// Shift right for two registers
void SimpleALU::shiftRightDual(uint32_t nb, Register* reven, Register* rodd) {
  uint32_t re = reven->read();
  uint32_t ro = rodd->read();
  // In the odd register (the LSB), we shift right and then we add the bits lost from shifting the even register in the last bits of the register
  rodd->write((ro >> nb) | ((re & (0xFFFFFFFF >> (32 - nb))) << (32 - nb)));
  reven->write(re >> nb);
}

// Complement two registers (transform positive into negative integer)
void SimpleALU::complementDual(Register* reven, Register* rodd) {
  uint32_t notrodd = ~(rodd->read());
  reven->write(~(reven->read()) + (notrodd == 0xFFFFFFFF ? 1 : 0));
  rodd->write(notrodd + 1);
}

// Sub for two registers
void SimpleALU::dualSub(Register* even, Register* odd, uint64_t term) {
  uint32_t val = odd->read();
  odd->write(val - (uint32_t)(term & 0x00000000FFFFFFFF));

  if (odd->read() > val || odd->read() > ((uint32_t)(term & 0x00000000FFFFFFFF)))
    val = 1; // borrow
  else
    val = 0;

  even->write(even->read() - (uint32_t)((term & 0xFFFFFFFF00000000) >> 32) - val);
}

// Add for two registers
void SimpleALU::dualAdd(Register* even, Register* odd, uint64_t term) {
  uint32_t val = odd->read();
  odd->write(val + (uint32_t)(term & 0x00000000FFFFFFFF));

  if (odd->read() < val || odd->read() < ((uint32_t)(term & 0x00000000FFFFFFFF)))
    val = 1; // carry
  else
    val = 0;

  even->write(even->read() + (uint32_t)((term & 0xFFFFFFFF00000000) >> 32) + val);
}

// Unsigned multiplication algorithm
void SimpleALU::umult(uint32_t multiplicand, uint32_t multiplier, Register* rd) {
  rd->write(0);
  writeY(multiplier);
  for (uint32_t i = 0; i < 32; i++) {
    if (readY() & 0x00000001 == 1) {
      rd->write(multiplicand + rd->read());
    }
    shiftRightDual(1, rd, getY());
  }
}

// Unsigned division algorithm
void SimpleALU::udiv(uint32_t dividend, uint32_t divisor, Register* rd) {
  writeY(0);
  rd->write(dividend);

  for (uint32_t i = 0; i < 32; i++) {
    if (ISNEG(readY())) {
      shiftLeftDual(1, getY(), rd);
      writeY(readY() + divisor);
    } else {
      shiftLeftDual(1, getY(), rd);
      writeY(readY() - divisor);
    }

    rd->write(rd->read() + (ISNEG(readY()) ? 0 : 1));
  }

  if (ISNEG(readY())) {
    writeY(readY() + divisor);
  }
}

// Calculate from two registers
void SimpleALU::calc(uint8_t op, const Register* rs1, const Register* rs2, Register* rd) {
  calc(op, rs1, rs2->read(), rd);
}

// Calculate from a register and a constant
void SimpleALU::calc(uint8_t op, const Register* rs1, uint32_t simm, Register* rd) {
  uint8_t mainop = op & 0x0F;         // ADDcc and ADD are basically the same => this is mainop
  uint8_t optype = (op & 0xF0) >> 4;  // Does it modifies condition codes ?

  // Shift operation
  if (optype == 2) {
    uint32_t value = rs1->read();
    
    if (op == ALU_OP_SLL) { // shift left
      rd->write(value << simm);
    } else { // shift right
      uint32_t res = value >> simm;

      if (op == ALU_OP_SRA) // artihmetic shift (extends sign if needed)
        res = signext(res, 32 - simm);

      rd->write(res);
    }
  }
  // Any other operation (because they came in two versions, which is not the cas of shifts)
  else {
    if (mainop == ALU_OP_UMUL || mainop == ALU_OP_SMUL) {
      // special case : the multiplication
      uint32_t value = rs1->read();
      
      // If a signed multiplication is asked, we just unsignedly multiply magnitudes, and then apply the sign rule
      if (mainop == ALU_OP_SMUL) {
        umult(ISNEG(value) ? COMPL32(value) : value,
              ISNEG(simm) ? COMPL32(simm) : simm,
              rd);

        if (ISNEG(value) != ISNEG(simm)) // opposed signs !
          complementDual(rd, getY());
      } else {
        umult(value, simm, rd);
      }

      if (optype == 1) { // change ICC
        setC(false);
        setV(readY() != 0);
        setN(ISNEG(rd->read()) && mainop == ALU_OP_SMUL); // no need to consider sign if there is none !
        setZ(rd->read() == 0 && readY() == 0);
      }
    } else if (mainop == ALU_OP_UDIV || mainop == ALU_OP_SDIV) {
      // another special case : the division
      uint32_t value = rs1->read();
      
      // If a signed division is asked, we just unsignedly divide magnitudes, and then apply the sign rule
      if (mainop == ALU_OP_SMUL) {
        udiv(ISNEG(value) ? COMPL32(value) : value,
             ISNEG(simm) ? COMPL32(simm) : simm,
             rd);

        if (ISNEG(value) != ISNEG(simm)) // opposed signs !
          complementDual(rd, getY());
      } else {
        udiv(value, simm, rd);
      }

      if (optype == 1) { // change ICC
        setC(false);
        setV(false);
        setN(ISNEG(rd->read()) && mainop == ALU_OP_SMUL); // no need to consider sign if there is none !
        setZ(rd->read() == 0 && readY() == 0);
      }
    } else {
      // every other cases : standard, non-algorithmics operations
      uint32_t value = rs1->read();
      uint32_t res;

      switch (mainop) {
        // Arithmetics
        case ALU_OP_ADD:
        case ALU_OP_ADDX: // ADDX is extended add : it adds to the result the carry condition flag
          res = value + simm + (mainop == ALU_OP_ADDX ? (getC() ? 1 : 0) : 0);
          if (optype == 1) { // modify ICC
            if (sub(value, 31, 1) == sub(simm, 31, 1))
              setV(sub(res, 31, 1) != sub(value, 31, 1));
            else
              setV(false);

            setC(res < value || res < simm);
          }
          break;
        case ALU_OP_SUB:
        case ALU_OP_SUBX: // SUBX is extend sub : it subs from the result the carry condition flag
          res = value - simm - (mainop == ALU_OP_SUBX ? (getC() ? 1 : 0) : 0);
          if (optype == 1) { // modify ICC
            if (sub(value, 31, 1) != sub(simm, 31, 1))
              setV(sub(res, 31, 1) != sub(value, 31, 1));
            else
              setV(false);

            setC(res > value || res > simm);
          }
          break;
        // Logical
        case ALU_OP_AND:
        case ALU_OP_ANDN:
          res = value & (mainop == ALU_OP_ANDN ? ~simm : simm);
          break;
        case ALU_OP_OR:
        case ALU_OP_ORN:
          res = value | (mainop == ALU_OP_ORN ? ~simm : simm);
          break;
        case ALU_OP_XOR:
        case ALU_OP_XNOR:
          res = value ^ (mainop == ALU_OP_XNOR ? ~simm : simm);
          break;
      }

      if (optype == 1) { // modify ICC
        setZ(res == 0);
        setN(ISNEG(res));
        if (mainop != ALU_OP_ADD && mainop != ALU_OP_SUB &&
            mainop != ALU_OP_ADDX && mainop != ALU_OP_SUBX) {
          setC(false);
          setV(false);
        }
      }
      rd->write(res);
    }
  }
}


