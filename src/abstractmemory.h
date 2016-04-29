/*
 * abstractmemory.h -- abstract class for defining a memory device
 * ------
 * Author: krab
 * Version: 0.1
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stdexcept>

#include "utils.h"
#include "instruction.h"
#include "register.h"

/**
 * Represents an abstract virtual memory device for using with the kSPARC engine.
 *
 * AbstractMemory concentrates mainly two functions : read and write, which retrieve/put
 * from/into the memory.
 *
 * On the subject of alignment
 * ===========================
 *
 * Memory alignment problems are left to the implementations, as they should be considered
 * in read and write.
 *
 * In fact, alignment does not represent an important problem in a virtual device, but
 * realistic behaviours cannot be achieved whithout it.
 *
 * This is why a "BadAlignmentException" exception class for handling that kind of problem.
 */
class AbstractMemory {
	public:
    /**
     * Defines an exception that can be raised when accessing memory
     */
    class BadAlignmentException : public std::logic_error {
      public:
        BadAlignmentException();
        ~BadAlignmentException();
    };

    /**
     * Constructor
     *
     * @param size  requested size for the memory
     */
		AbstractMemory(const uint32_t size);

    /**
     * Destructor
     */
		virtual ~AbstractMemory();

    // General methods
    /**
     * Returns the size of the memory
     * @returns the size
     */
    uint32_t getSize() const;

    // Read functions
    /**
     * Core function of the virtual device : read data from the memory
     * @param address address of the data to be retrieved
     * @param size size of the data we want
     * @param data the result of the call
     * @see write()
     */
    virtual void read(uint32_t address, uint32_t size, uint8_t* data) const = 0;

    /**
     * Read a byte from the memory
     * @param address address of the data
     * @returns the data
     * @see read()
     */
    uint8_t readByte(uint32_t address) const;
    
    /**
     * Read a byte from the memory and put it into a register
     * @param address address of the data
     * @param rd destination register where to put the data
     * @see readByte()
     */
    void readByte(uint32_t address, Register* rd) const;

    /**
     * Read a halfword (16-bit data) from the memory
     * @param address address of the data
     * @returns the data
     * @see read()
     */
    uint16_t readHalfword(uint32_t address) const;
    /**
     * Read a halfword from the memory and put it into a register
     * @param address address of the data
     * @param rd destination register where to put the data
     * @see readHalfword()
     */
    void readHalfword(uint32_t address, Register* rd) const;
    
    /**
     * Read a word (32 bits) from the memory
     * @param address address of the data
     * @returns the data
     * @see read()
     */
    uint32_t readWord(uint32_t address) const;
    /**
     * Read a word from the memory and put it into a register
     * @param address address of the data
     * @param rd destination register where to put the data
     * @see readWord()
     */
    void readWord(uint32_t address, Register* rd) const;

    /**
     * Read a double word (64 bits) from the memory
     * @param address address of the data
     * @returns the data
     * @see read()
     */
    uint64_t readDoubleword(uint32_t address) const;
    /**
     * Read a double word from the memory and put it into a register.
     * As a double word cannot be stored in a single register, we use two, theoretically
     * aligned, such that data = rdeven|rdodd
     * @param address address of the data
     * @param rdeven even destination register where to put the MSB of the data
     * @param rdodd odd destination register where to put the LSB of the data
     * @see readDoubleword()
     */
    void readDoubleword(uint32_t address, Register* rdeven, Register* rdodd) const;

    /**
     * Utility function that reads an instruction from the memory
     * @param address address of the instruction
     * @returns the instruction
     */
    Instruction readInstruction(uint32_t address) const;

    // Write functions
    /**
     * Core function of the virtual device : write data to the memory
     * @param address address of the data to write
     * @param size size of the data
     * @param data the data to write
     * @see read()
     */
    virtual void write(uint32_t address, uint8_t* data, uint32_t size) = 0;

    /**
     * Write a byte in the memory.
     * @param address where to put the data
     * @param data data to write
     * @see write()
     */
    void writeByte(uint32_t address, uint8_t data);
    /**
     * Write a byte read from a register to the memory
     * @param adress where to put the data
     * @param rd register from which to take data
     * @see writeByte()
     */
    void writeByte(uint32_t address, const Register* rd);

    /**
     * Write a half word (16 bits) in the memory.
     * @param address where to put the data
     * @param data data to write
     * @see write()
     */
    void writeHalfword(uint32_t address, uint16_t data);
    /**
     * Write a half word read from a register to the memory
     * @param adress where to put the data
     * @param rd register from which to take data
     * @see writeHalfword()
     */
    void writeHalfword(uint32_t address, const Register* rd);
 
    /**
     * Write a word (32 bits) in the memory.
     * @param address where to put the data
     * @param data data to write
     * @see write()
     */   
    void writeWord(uint32_t address, uint32_t data);
    /**
     * Write a word read from a register to the memory
     * @param adress where to put the data
     * @param rd register from which to take data
     * @see writeWord()
     */
    void writeWord(uint32_t address, const Register* rd);

    /**
     * Write a double word (64 bits) in the memory.
     * @param address where to put the data
     * @param data data to write
     * @see write()
     */
    void writeDoubleword(uint32_t address, uint64_t data);
    /**
     * Write a double word read from a register to the memory
     * @param adress where to put the data
     * @param rd register from which to take data
     * @see writeDoubleword()
     */
    void writeDoubleword(uint32_t address, const Register* rdeven, const Register* rdodd);

	private:
    //!< Size of the memory
    uint32_t _size;

};

#endif // MEMORY_H

