/*
 * specialregister.h -- defines the SpecialRegister state
 */
#ifndef SPECIALREGISTER_H
#define SPECIALREGISTER_H

#include "utils.h"
#include "register.h"

// Some fields  
#define PSR_IMPL  28, 4   // CPU's implementation
#define PSR_VERS  24, 4   // CPU's version
#define PSR_ICC   20, 4   // Condition codes
#define PSR_ICC_N 23, 1   // is negative
#define PSR_ICC_Z 22, 1   // is zero
#define PSR_ICC_V 21, 1   // is overflow
#define PSR_ICC_C 20, 1   // is carry set
#define PSR_EC    13, 1   // enable co-processor
#define PSR_EF    12, 1   // enable fpu
#define PSR_PIL   8, 4    // processor interruption level
#define PSR_S     7, 1    // supervisor mode
#define PSR_PS    6, 1    // value of S before the most recent trap
#define PSR_ET    5, 1    // enable traps
#define PSR_CWP   0, 5    // window counter

#define TBR_TBA   12, 20  // trap base address
#define TBR_TT    4, 8    // trap type

#define FPU_ROUND 30, 2   // FPU round type (floor, ceil, middle)
#define FPU_TEM   23, 5   // Trap enable mask (which fpu traps are to be considered)
#define FPU_NS    22, 1   // Is the FPU non-standard (exotic way of doing things)
#define FPU_IMPL  20, 2   // FPU implementation
#define FPU_VERS  17, 3   // FPU version
#define FPU_FTT   14, 3   // floating point trap type : what caused the trap
#define FPU_FCC   10, 2   // FPU condition codes
#define FPU_AEXC  5, 5    // exception handling related
#define FPU_CEXC  0, 5    // exception handling related

/**
 * This class represent a SpecialRegister. This is mainly a register with a few funcitonnalities, and is more a way of putting things that a real feature.
 * Special registers represents every registers that are used in another way than just storing plain data : CPU's status, special adressing, etc.
 */
class SpecialRegister : public Register {
	public:
    /**
     * Default constructor
     */
    SpecialRegister();
    /**
     * Constructor
     * @see Register::Register()
     */
		SpecialRegister(uint32_t* content);
    /**
     * Destructor
     */
		~SpecialRegister();

    /**
     * Get a field from the register
     * @param from bit from which to get the field
     * @param size number of bits to retrieve
     * @returns value of the field
     */
    uint32_t getField(uint32_t from, uint32_t size) const;
    /**
     * Set a field of the register
     * @param from starting point of the field
     * @param size number of bits of the field
     * @param value new value for the field
     */
    void setField(uint32_t from, uint32_t size, uint32_t value);
    
	private:

};

#endif // SPECIALREGISTER_H

