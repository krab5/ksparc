/*
 * asmerror.h -- defines an ensemble of errors for error treatment in the assembler
 * -----
 * Author: krab
 * Version: 0.1
 */
#ifndef ASMERROR_H
#define ASMERROR_H

#include <string>
#include <vector>

/**
 * Defines an opcode with its name, code and parameters requisites
 */
struct OpCode {
  uint32_t code;    //!< Binary code
  std::string name; //!< Name of the opcode
  size_t numparams; //!< Typical number of parameters
  size_t mnumparams;//!< Minimum number of parameters
  std::string arg1; //!< Name of the requested first argument
  std::string arg2; //!< Name of the requested second argument
  std::string arg3; //!< Name of the requested third argument
  bool implemented; //!< Indicates wheter or not the opcode has been implemented

  /**
   * Constructor for 0 argument opcode
   * @param c code of the opcode
   * @param n name of the opcode
   * @param i has been implemented (true by default)
   */
  OpCode(uint32_t c, std::string n, size_t mn = 0, bool i = true) : code(c), name(n), numparams(0), arg1(""), arg2(""), arg3(""), mnumparams(mn), implemented(i) {}
  /**
   * Constructor for 0 argument opcode
   * @param c code of the opcode
   * @param n name of the opcode
   * @param a1 first argument
   * @param mn minimal amount of argument to provide
   * @param i has been implemented (true by default)
   */
  OpCode(uint32_t c, std::string n, std::string a1, size_t mn = 1, bool i = true) : code(c), name(n), numparams(1), arg1(a1), arg2(""), arg3(""), mnumparams(mn), implemented(i) {}
  /**
   * Constructor for 0 argument opcode
   * @param c code of the opcode
   * @param n name of the opcode
   * @param a1 first argument
   * @param a2 second argument
   * @param mn minimal amount of argument to provide
   * @param i has been implemented (true by default)
   */
  OpCode(uint32_t c, std::string n, std::string a1, std::string a2, size_t mn = 2, bool i = true) : code(c), name(n), numparams(2), arg1(a1), arg2(a2), arg3(""), mnumparams(mn), implemented(i) {}
  /**
   * Constructor for 0 argument opcode
   * @param c code of the opcode
   * @param n name of the opcode
   * @param a1 first argument
   * @param a2 second argument
   * @param a3 third argument
   * @param mn minimal amount of argument to provide
   * @param i has been implemented (true by default)
   */
  OpCode(uint32_t c, std::string n, std::string a1, std::string a2, std::string a3, size_t mn = 3, bool i = true) : code(c), name(n), numparams(3), arg1(a1), arg2(a2), arg3(a3), mnumparams(mn), implemented(i) {}
};

namespace ASMError {

  /**
   * Base class for an error, with a message and a line.
   *
   * Errors basically works like exceptions. They have two main fields : the message of the error, and the line where it happened.
   */
  class Error {
    public:
      /**
       * Constructor
       * @param message message of the error
       * @param line line where the problem happened
       */
      Error(std::string message, size_t line);
      /**
       * Destructor
       */
      ~Error();

      /**
       * Get the line
       * @param the line where the problem happened
       */
      size_t line();
      /**
       * Get the message
       * @param the message corresponding to the error
       */
      std::string getMessage();
      /**
       * Set the message
       * @param new message
       */
      void setMessage(std::string s);

    private:
      std::string _msg;
      size_t _line;
  };

  /**
   * Encountered an unexpected opcode.
   *
   * The error stores the bad opcode.
   */
  class UnexpectedOpcodeError : public Error {
    public:
      UnexpectedOpcodeError(std::string s, size_t line);
      ~UnexpectedOpcodeError();
  };

  /**
   * Encountered an unimplemented opcode.
   *
   * The error stores the bad opcode.
   */
  class UnimplementedOpcodeError : public Error {
    public:
      UnimplementedOpcodeError(std::string s, size_t line);
      ~UnimplementedOpcodeError();
  };

  /**
   * Encountered a badly formated number.
   *
   * The error stores the bad number.
   */
  class WrongNumberFormatError : public Error {
    public:
      WrongNumberFormatError(std::string s, size_t line);
      ~WrongNumberFormatError();
  };

  /**
   * Encountered a badly formated address.
   */
  class WrongAddressFormatError : public Error {
    public:
      WrongAddressFormatError(size_t line);
      ~WrongAddressFormatError();
  };

  /**
   * Encountered and invalid r register number.
   *
   * The error stores the number of the register.
   */
  class InvalidRRegisterNumberError : public Error {
    public:
      InvalidRRegisterNumberError(uint32_t reg, size_t line);
      ~InvalidRRegisterNumberError();
  };

  /**
   * Encountered and invalid o, i, l or g register number.
   *
   * The error stores the number of the register.
   */
  class InvalidOILGRegisterNumberError : public Error {
    public:
      InvalidOILGRegisterNumberError(uint32_t reg, size_t line);
      ~InvalidOILGRegisterNumberError();
  };

  /**
   * Encountered and invalid register name.
   *
   * The error stores the bad register name.
   */
  class InvalidRegisterNameError : public Error {
    public:
      InvalidRegisterNameError(char c, size_t line);
      ~InvalidRegisterNameError();
  };

  /**
   * Encountered and invalid special register name.
   *
   * The error stores the bad register name.
   */
  class InvalidSpecialRegisterNameError : public Error {
    public:
      InvalidSpecialRegisterNameError(std::string s, size_t line);
      ~InvalidSpecialRegisterNameError();
  };

  /**
   * Trying to define an already defined label.
   *
   * The error stores the label and where it was defined.
   */
  class LabelAlreadyDefinedError : public Error {
    public:
      LabelAlreadyDefinedError(std::string lbl, uint32_t instaddr, size_t line);
      ~LabelAlreadyDefinedError();
  };

  /**
   * Try to call for an undefined label.
   *
   * The error stores the requested label.
   */
  class LabelNotFoundError : public Error {
    public:
      LabelNotFoundError(std::string lbl, size_t line);
      ~LabelNotFoundError();
  };

  /**
   * Encountered an invalid number of parameters for an opcode.
   *
   * The error stores the requested opcode (including the good number of parameters)
   */
  class InvalidParamNumberError : public Error {
    public:
      InvalidParamNumberError(OpCode oc, size_t line);
      ~InvalidParamNumberError();
  };

  /**
   * Defines a list of errors.
   */
  class ErrorList : public std::vector<ASMError::Error> {
    public:
      ErrorList();
      ~ErrorList();
      void add(std::string s, size_t l);
  };

  /**
   * Base class for defining a warning.
   */
  class Warning {
    public:
      /**
       * Constructor
       * @param message the message of the warning
       * @param line the line here it happened
       */
      Warning(std::string message, size_t line);
      /**
       * Destructor
       */
      ~Warning();

      /**
       * Get the line
       * @returns the line
       */
      size_t line();
      /**
       * Get the message
       * @returns the message
       */
      std::string getMessage();
      /**
       * Set the message
       * @param str new message
       */
      void setMessage(std::string str);

    private:
      std::string _msg;
      size_t _line;
  };

  /**
   * Too much parameters for an opcode.
   *
   * The warning stores the opcode of the warning.
   */
  class TooMuchParametersWarning : public Warning {
    public:
      TooMuchParametersWarning(OpCode oc, size_t line);
      ~TooMuchParametersWarning();
  };
  
  /**
   * Defines a list of warnings
   */
  class WarningList : public std::vector<ASMError::Warning> {
    public:
      WarningList();
      ~WarningList();
      void add(std::string s, size_t l);
  };

}

#endif // ASMERROR_H

