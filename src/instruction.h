/*
 * instruction.h -- defines the Instruction class
 * ------
 * Author: krab
 * Version: 0.1
 */
/** @file */
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include "utils.h"

// Defines some macros for asking fields with getField method
#define INST_OP         30, 2
#define INST_DISP30     0, 30
#define INST_RD         25, 5
#define INST_A          29, 1
#define INST_COND       25, 4
#define INST_OP2        22, 3
#define INST_IMM22      0, 22
#define INST_DISP22     0, 22
#define INST_OP3        19, 6
#define INST_RS1        14, 5
#define INST_I          13, 1
#define INST_ASI        5, 8
#define INST_RS2        0, 5
#define INST_SIMM13     0, 13
#define INST_OPF        5, 9

// Defines what represents each values
#define INST_OP_CALL    1   // Calls
#define INST_OP_BR      0   // Branch and SETHI
#define INST_OP_MEM     3   // Load/Store
#define INST_OP_OTHER   2   // Arithmetics, logics, others

#define INST_OP2_BICC   2   // branch if condition code
#define INST_OP2_SETHI  4   // set the 22 most significatn bits of a register
#define INST_OP2_FBFCC  6   // branch if floating point unit condition code
#define INST_OP2_CBCCC  7   // branch if coprocessor condition code

// OP3 for OP = 2
#define INST_OP3_RDY    0x28  // read special register y
#define INST_OP3_RDPSR  0x29  // read special register psr
#define INST_OP3_RDWIM  0x2A  // read special register wim
#define INST_OP3_RDTBR  0x2B  // read special register tbr
#define INST_OP3_WRY    0x30  // write special register y
#define INST_OP3_WRPSR  0x31  // write special register psr
#define INST_OP3_WRWIM  0x32  // write special register wim
#define INST_OP3_WRTBR  0x33  // write special register tbr
#define INST_OP3_FPOP1  0x34  // floating point operation 1 (typically, not modifying condition codes)
#define INST_OP3_FPOP2  0x35  // floating point operation 2
#define INST_OP3_CPOP1  0x36  // co-processor operation 1 (typically, not modifying condition codes)
#define INST_OP3_CPOP2  0x37  // co-processor operation 2
#define INST_OP3_JMPL   0x38  // jump and link
#define INST_OP3_RETT   0x39  // return from trap
#define INST_OP3_TICC   0x3A  // trap if condition code
#define INST_OP3_FLUSH  0x3B  // flush a buffer
#define INST_OP3_SAVE   0x3C  // save context
#define INST_OP3_REST   0x3D  // restore context

// OP3 for OP = 3
#define INST_OP3_LDSB   0x09  // load signed byte
#define INST_OP3_LDSH   0x0A  // load signed halfword
#define INST_OP3_LDUB   0x01  // load unsigned byte
#define INST_OP3_LDUH   0x02  // load unsigned halfword
#define INST_OP3_LD     0x00  // load a word
#define INST_OP3_LDD    0x03  // load a double word
#define INST_OP3_LDF    0x20  // load simple-precision floating point
#define INST_OP3_LDDF   0x23  // load double-precision floating point
#define INST_OP3_LDFSR  0x21  // load fsr
#define INST_OP3_LDC    0x30  // load coproc register
#define INST_OP3_LDDC   0x33  // load coproc double register
#define INST_OP3_LDCSR  0x31  // load csr
#define INST_OP3_STB    0x05  // store byte
#define INST_OP3_STH    0x06  // store halfword
#define INST_OP3_ST     0x04  // store word
#define INST_OP3_STD    0x07  // store double word
#define INST_OP3_STF    0x44  // store simple-precision floating point
#define INST_OP3_STDF   0x47  // store double-precision floating point
#define INST_OP3_STFSR  0x45  // store fsr
#define INST_OP3_STC    0x64  // store coproc register
#define INST_OP3_STDC   0x67  // store coproc double register
#define INST_OP3_STCSR  0x65  // store csr

// Conditions code for branches
#define INST_COND_ALWAYS  0x8   // always
#define INST_COND_NEVER   0x0   // never
#define INST_COND_NEQ     0x9   // not equal
#define INST_COND_EQ      0x1
#define INST_COND_GT      0xA
#define INST_COND_LET     0x2
#define INST_COND_GET     0xB
#define INST_COND_LT      0x3
#define INST_COND_UGT     0xC
#define INST_COND_ULET    0x4
#define INST_COND_CCLR    0xD
#define INST_COND_CSET    0x5
#define INST_COND_POS     0xE
#define INST_COND_NEG     0x6
#define INST_COND_OCLR    0xF
#define INST_COND_OSET    0x7

#define INST_FCOND_ALWAYS   0x8
#define INST_FCOND_NEVER    0x0
#define INST_FCOND_U        0x7
#define INST_FCOND_G        0x6
#define INST_FCOND_UG       0x5
#define INST_FCOND_L        0x4
#define INST_FCOND_UL       0x3
#define INST_FCOND_LG       0x2
#define INST_FCOND_NE       0x1
#define INST_FCOND_E        0x9
#define INST_FCOND_UE       0xA
#define INST_FCOND_GE       0xB
#define INST_FCOND_UGE      0xC
#define INST_FCOND_LE       0xD
#define INST_FCOND_ULE      0xE
#define INST_FCOND_O        0xF

#define INST_CCOND_ALWAYS   0x8
#define INST_CCOND_NEVER    0x0
#define INST_CCOND_3        0x7
#define INST_CCOND_2        0x6
#define INST_CCOND_23       0x5
#define INST_CCOND_1        0x4
#define INST_CCOND_13       0x3
#define INST_CCOND_12       0x2
#define INST_CCOND_123      0x1
#define INST_CCOND_0        0x9
#define INST_CCOND_03       0xA
#define INST_CCOND_02       0xB
#define INST_CCOND_023      0xC
#define INST_CCOND_01       0xD
#define INST_CCOND_013      0xE
#define INST_CCOND_012      0xF

/**
 * This class defines an instruction. This is a easy way of viewing the complex 32 bits codes.
 *
 * Most of all, this file includes a large number of fields, codes and so on that helps to determine what does what.
 */
class Instruction {
	public:
    /**
     * Constructor
     * @param content code of the instruction
     */
		Instruction(uint32_t content);
    /**
     * Destructor
     */
		~Instruction();

    /**
     * Copy operator
     */
    Instruction& operator=(const Instruction& i);
    
    /**
     * Get whole data of the instruction
     * @returns instruction content
     */
    uint32_t getContent() const;
    /**
     * Set the data of the instruction
     * @param value new content
     */
    void setContent(uint32_t value);
    
    /**
     * Get a particular part of the instruction, from bit "from" and of a size "size". Note that we count the bit from right to left.
     * This file defines a large amount of predefined fields, making easy the parsing of a code. For example :
     *    getField(INST_COND)
     * retrieves the condition field of an instruction.
     * @param from from where to count
     * @param size how many bit to take
     */
    uint32_t getField(uint32_t from, uint32_t size) const;

    /**
     * Build an instruction from its fields.
     * This one builds a format 1 instruction (call)
     * @param op main op number (should be INST_OP_CALL)
     * @param disp displacement of the call
     * @returns the corresponding instruction
     */
    static Instruction makeInstruction(uint32_t op, uint32_t disp);
    /**
     * Build an instruction from its fields.
     * This one builds a format 2 instruction for SETHI.
     * @param op main op number (should be INST_OP_BR)
     * @param rd destination register number
     * @param op2 secondary op number (should be 4 == INST_OP2_SETHI)
     * @param imm22 unsigned 22 bits data
     * @returns the corresponding instruction
     */
    static Instruction makeInstruction(uint32_t op, uint32_t rd,                uint32_t op2,  uint32_t imm22);
    /**
     * Build an instruction from its fields.
     * This one builds a format 2 instruction for branches.
     * @param op main op number (should be INST_OP_BR)
     * @param a annulment bit
     * @param cond condition of the branch
     * @param op2 type of branch (INST_OP2_BICC for standard branches, INST_OP2_FBFCC for FPU related branches or INST_OP2_CBCCC for coproc related branches)
     * @param disp22 22-bit signed number specifying the displacement of the branch
     * @returns the corresponding instruction
     */
    static Instruction makeInstruction(uint32_t op, uint32_t a,  uint32_t cond, uint32_t op2, uint32_t disp22);
    /**
     * Build an instruction from its fields.
     * This one builds a format 3 instruction for memory instructions and arithmetic/logic instructions (plus some others like JMPL, SAVE, etc.)
     * @param op main op number (should be INST_OP_OTHER or INST_OP_MEM)
     * @param rd destination register
     * @param op3 operation to do (a ALU_OP_* or INST_OP3_*)
     * @param rs1 source register 1
     * @param i if equals 0, specifies that we should use a source register as second operand. Else, we use a signed 13 bits constant
     * @param asi address space id; used for special loads and stores (into other memories). This is not used for now
     * @param rs2OrSimm13 source register 2 or 13 bits signed constant
     * @returns the corresponding instruction
     */
    static Instruction makeInstruction(uint32_t op, uint32_t rd,                uint32_t op3,  uint32_t rs1, uint32_t i, uint32_t asi, uint32_t rs2OrSimm13);
    /**
     * Build an instruction from its fields.
     * This one builds a format 3 instruction for "exotic" instructions (e.g.: FPU and co-processor operations)
     * @param op main op number (should be INST_OP_OTHER)
     * @param rd destination register
     * @param op3 operation to do (INST_OP3_*, probably INST_OP3_FPOP* or INST_OP3_CPOP*)
     * @param rs1 source register 1
     * @param opf operation specifier relative to the device (fmul, fadd, etc.)
     * @param rs2 source register 2
     * @returns the corresponding instruction
     */
    static Instruction makeInstruction(uint32_t op, uint32_t rd,                uint32_t op3,  uint32_t rs1, uint32_t opf,             uint32_t rs2);

	private:
    /** Content of the instruction. This is a 32 bits unisnged integer */
    uint32_t _content;
};

#endif // INSTRUCTION_H

