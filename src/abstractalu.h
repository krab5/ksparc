/*
 * abstractalu.h -- abstract class for defining the ALU
 * -----
 * Author: krab
 * Version: 0.1
 */
/** @file */
#ifndef ABSTRACTALU_H
#define ABSTRACTALU_H

#include "register.h"
#include "specialregister.h"
#include "utils.h"

// Calculate
#define ALU_OP_ADD    0x00    // addition
#define ALU_OP_AND    0x01    // bitwise and
#define ALU_OP_OR     0x02    // bitwise or
#define ALU_OP_XOR    0x03    // bitwise xor
#define ALU_OP_SUB    0x04    // substraction
#define ALU_OP_ANDN   0x05    // bitwise and with ~right
#define ALU_OP_ORN    0x06    // birwise or with ~right
#define ALU_OP_XNOR   0x07    // bitwise xor with ~right
#define ALU_OP_ADDX   0x08    // extended add : add two operands + carry
#define ALU_OP_UMUL   0x0A    // unsigned mult
#define ALU_OP_SMUL   0x0B    // signed mult
#define ALU_OP_SUBX   0x0C    // extended sub : sub two operands + carry
#define ALU_OP_UDIV   0x0E    // unsigned div
#define ALU_OP_SDIV   0x0F    // signed div

// Calc and change ICC flags
#define ALU_OP_ADDcc  0x10
#define ALU_OP_ANDcc  0x11
#define ALU_OP_ORcc   0x12
#define ALU_OP_XORcc  0x13
#define ALU_OP_SUBcc  0x14
#define ALU_OP_ANDNcc 0x15
#define ALU_OP_ORNcc  0x16
#define ALU_OP_XNORcc 0x17
#define ALU_OP_ADDXcc 0x18
#define ALU_OP_UMULcc 0x1A
#define ALU_OP_SMULcc 0x1B
#define ALU_OP_SUBXcc 0x1C
#define ALU_OP_UDIVcc 0x1E
#define ALU_OP_SDIVcc 0x1F

#define ALU_OP_SLL    0x25    // shift left logical
#define ALU_OP_SRL    0x26    // shift right logical
#define ALU_OP_SRA    0x27    // shift right arithmetic : preserve the sign of the result

/**
 * Represents an abstract arithmetic and logic unit.
 *
 * An ALU is a device capable of making simple operations; that is : simple arithmetics and simple logic.
 *
 * Basically, it is able to do additions, substractions, multiplications, divisions, and, or, xor and shifting, with signed or unsigned datas.
 *
 * Most of the operations (except for shift) have two versions : one that modifies the condition codes (*cc) and one that does not.
 *
 * The operations all takes two parameters and give one result; the first parameter is always a register, and the second one can be either a register or a direct constant value.
 *
 * The 'Y' register
 * ================
 *
 * The y register is mainly used in the multiplication and division algorithm. Typically, the multiplication takes two 32-bit operands and gives a 64-bit result, from which the most significant bits are stored in y. For the division though, y contains the rest of the division.
 */
class AbstractALU {
	public:
    /**
     * Constructor
     *
     * @param psr the processor state register; this will be modified by the *cc operations
     * @param y the multiplication and division extension register
     */
		AbstractALU(SpecialRegister* psr, Register* y);

    /**
     * Destructor
     */
		~AbstractALU();

    /**
     * Ask the ALU to make a calculus from two source registers, putting the result in a destination register
     * @param op operation to make
     * @param rs1 source register 1
     * @param rs2 source register 2
     * @param rd destination register
     */
    virtual void calc(uint8_t op, const Register* rs1, const Register* rs2, Register* rd) = 0;
    /**
     * Ask the ALU to make a calculus from a source register and a constant, putting the result in a destination register
     * @param op operation to make
     * @param rs1 source register 1
     * @param simm the constant
     * @param rd destination register
     */
    virtual void calc(uint8_t op, const Register* rs1, uint32_t simm, Register* rd) = 0;

  protected:
    /**
     * Get the 'N' condition code (for negative)
     * @returns the N condition code
     * @see setN()
     */
    bool getN() const;
    /**
     * Get the 'Z' condition code (for zero)
     * @returns the Z condition code
     * @see setZ()
     */
    bool getZ() const;
    /**
     * Get the 'C' condition code (for carry)
     * @returns the C condition code
     * @see setC()
     */
    bool getC() const;
    /**
     * Get the 'V' condition code (for overflow)
     * @returns the V condition code
     * @see setV()
     */
    bool getV() const;

    /**
     * Set the 'N' condition code
     * @param n new value
     * @see getN()
     */
    void setN(bool);
    /**
     * Set the 'Z' condition code
     * @param z new value
     * @see getZ()
     */
    void setZ(bool);
    /**
     * Set the 'C' condition code
     * @param c new value
     * @see getC()
     */
    void setC(bool);
    /**
     * Set the 'V' condition code
     * @param v new value
     * @see getV()
     */
    void setV(bool);

    /**
     * Get the y register
     * @returns the register
     */
    Register* getY();
    /**
     * Read the content of the y register
     * @returns content of y
     * @see writeY()
     */
    uint32_t readY() const;
    /**
     * Write data in the y register
     * @param d data to write in y
     * @see readY()
     */
    void writeY(uint32_t);

	private:
    SpecialRegister *_psr;
    Register* _y;
};

#endif // ABSTRACTALU_H

