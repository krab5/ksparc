/*
 * simplealu.h -- defines a simple arithmetic and logic unit
 */
#ifndef SIMPLEALU_H
#define SIMPLEALU_H

#include "abstractalu.h"

/**
 * The SimpleALU class defines a simple arithmetic and logic unit for use in the SPARC engine.
 * It is simple as it is not quite reallisic; reals algorithms are used for multiplication and division, but not for any other operation. Plus, every operations are done in 1 cycle, which is wrong !
 * It is a good base though.
 */
class SimpleALU : public AbstractALU {
	public:
    /**
     * Constructor.
     * @param psr processor state register (contains condition flags)
     * @param y multiplication and division extension register
     */
		SimpleALU(SpecialRegister* psr, Register* y);
    /**
     * Destructor
     */
		~SimpleALU();

    /**
     * Calculate something
     * @see AbstractAlu::calc()
     */
    void calc(uint8_t op, const Register* rs1, const Register* rs2, Register* rd);
    /**
     * Calculate something
     * @see AbstractAlu::calc()
     */
    void calc(uint8_t op, const Register* rs1, uint32_t simm, Register* rd);

    /**
     * Substract a value from a double word represented by two aligned registers.
     * This makes the 64 bit substraction : even|odd = even|odd - term; with even as the MSB register and odd as the LSB one
     * @param even MSB register
     * @param odd LSB register
     * @param term term to substract
     */
    static void dualSub(Register* even, Register* odd, uint64_t term);
    /**
     * Add a value to a double word represented by two aligned registers.
     * This makes the 64 bit addition : even|odd = even|odd + term; with even as the MSB register and odd as the LSB one
     * @param even MSB register
     * @param odd LSB register
     * @param term term to add
     */
    static void dualAdd(Register* even, Register* odd, uint64_t term);
    /**
     * Shift left a double word represented by two aligned registers.
     * This makes a 64 bit shifting.
     * @param nb number of bits to shift
     * @param reven MSB register
     * @param rodd LSB register
     */
    static void shiftLeftDual(uint32_t nb, Register* reven, Register *rodd);
    /**
     * Shift right a double word represented by two aligned registers.
     * This makes a 64 bit shifting.
     * @param nb number of bits to shift
     * @param reven MSB register
     * @param rodd LSB register
     */
    static void shiftRightDual(uint32_t nb, Register* reven, Register* rodd);
    /**
     * Complement a double word represented by two aligned registers.
     * This makes the operation : reven|rodd = -(reven|rodd)
     * @param reven MSB register
     * @param rodd LSB register
     */
    static void complementDual(Register* reven, Register* rodd);

	protected:

	private:
    /**
     * Unsigned multiplication algorithm (quite standard).
     * @param multiplicand left number of the product
     * @param multiplier right number of the product
     * @param rd destination register
     */
    void umult(uint32_t multiplicand, uint32_t multiplier, Register* rd);
    /**
     * Unsigned division algorithm (quite standard).
     * Note that the Y register contains the modulus of the division at the end of the algorithm.
     * @param dividend left number of the division
     * @param divisor right number of the division
     * @param rd destination register
     */
    void udiv(uint32_t dividend, uint32_t divisor, Register* rd);

};

#endif // SIMPLEALU_H

