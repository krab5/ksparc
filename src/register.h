/*
 * register.h -- defines the Register class
 */
#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>

/**
 * This class represent a register, which is a basic component of nearly every CPU architecture.
 * Registers are fast access data storage that are used for internal state storing and fast data manipulation. Often, they are used for intermediary calculus, function calls, etc.
 * A register is not more than a single 32 bit value (physically though, it is a little bit more complex, but these considerations are not relevant here).
 */
class Register {
	public:
    /**
     * Default constructor. Build a register with an internal content.
     */
    Register();
    /**
     * Constructor. Build a register from an existing content.
     * @param content pointer to the content of the register
     */
		Register(uint32_t* content);
    /**
     * Destructor
     */
		~Register();

    /**
     * Read the value from the register
     * @returns the content of the register
     * @see write()
     */
    uint32_t read() const;
    /**
     * Write a value in the register
     * @param new value
     * @see read()
     */
    void write(const uint32_t val);

	private:
    uint32_t* _content;
};

#endif // SPECIALREGISTER_H

