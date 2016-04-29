/*
 * asmerror.cpp -- implements the classes of asmerror.h
 * -----
 * Author: krab
 * Version: 0.1
 */
#include <sstream>
#include "asmerror.h"

using namespace std;
using namespace ASMError;

Error::Error(std::string message, size_t line): _msg(message), _line(line) {
}

Error::~Error() {
}

size_t Error::line() {
  return _line;
}

string Error::getMessage() {
  return _msg;
}

void Error::setMessage(string s) {
  _msg = s;
}

UnexpectedOpcodeError::UnexpectedOpcodeError(string s, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Unexcpected opcode '" << s << "'";
  setMessage(ss.str());
}

UnexpectedOpcodeError::~UnexpectedOpcodeError() {
}

UnimplementedOpcodeError::UnimplementedOpcodeError(string s, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Opcode '" << s << "' has not been implemented yet !";
  setMessage(ss.str());
}

UnimplementedOpcodeError::~UnimplementedOpcodeError() {
}

WrongNumberFormatError::WrongNumberFormatError(string s, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Unexpected '" << s << "'; a number is required here";
  setMessage(ss.str());
}

WrongNumberFormatError::~WrongNumberFormatError() {
}

WrongAddressFormatError::WrongAddressFormatError(size_t line) : Error("Wrong address format; address are given by \"[reg1+reg2]\" or \"[reg1+cst]\", where reg1 and reg2 are registers, and cst is a 13-bit signed constant", line) {
}

WrongAddressFormatError::~WrongAddressFormatError() {
}

InvalidRRegisterNumberError::InvalidRRegisterNumberError(uint32_t reg, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Invalid register number " << reg << "; there are only 32 r registers";
  setMessage(ss.str());
}

InvalidRRegisterNumberError::~InvalidRRegisterNumberError() {
}

InvalidOILGRegisterNumberError::InvalidOILGRegisterNumberError(uint32_t reg, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Invalid register number " << reg << "; there are only 8 o, l, i and g registers";
  setMessage(ss.str());
}

InvalidOILGRegisterNumberError::~InvalidOILGRegisterNumberError() {
}

InvalidRegisterNameError::InvalidRegisterNameError(char c, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Invalid register name '" << c << "'; valid registers are r, o, i, l and g";
  setMessage(ss.str());
}

InvalidRegisterNameError::~InvalidRegisterNameError() {
}

InvalidSpecialRegisterNameError::InvalidSpecialRegisterNameError(string s, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Invalid special register name '" << s << "'; valid registers name are psr, tbr, y, wim";
  setMessage(ss.str());
}

InvalidSpecialRegisterNameError::~InvalidSpecialRegisterNameError() {
}

LabelAlreadyDefinedError::LabelAlreadyDefinedError(string lbl, uint32_t instaddr, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Label '" << lbl << "' has already been defined (at instruction number " << instaddr << ")";
  setMessage(ss.str());
}

LabelAlreadyDefinedError::~LabelAlreadyDefinedError() {
}

LabelNotFoundError::LabelNotFoundError(string lbl, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Label '" << lbl << "' not found";
  setMessage(ss.str());
}

LabelNotFoundError::~LabelNotFoundError() {
}

InvalidParamNumberError::InvalidParamNumberError(OpCode oc, size_t line) : Error("", line) {
  ostringstream ss;
  ss << "Opcode '" << oc.name << "' ";
  
  if (oc.mnumparams == 0)
    ss << "does not take any parameter";
  else {
    ss << "requires ";

    if (oc.mnumparams < oc.numparams)
      ss << "at least ";

    ss << oc.mnumparams << " parameter" << (oc.mnumparams > 1 ? "s" : "");

    if (oc.mnumparams < oc.numparams)
      ss << " but can accept up to " << oc.numparams;

    ss << " :";

    switch (oc.mnumparams) {
      case 1:
        ss << oc.arg1;
        if (oc.numparams == 2)
          ss << " (plus " << oc.arg2 << ")";
        else
          ss << " (plus " << oc.arg2 << " and " << oc.arg3 << ")";
        break;
      case 2:
        ss << oc.arg1 << " and " << oc.arg2;
        if (oc.numparams == 3)
          ss << " (plus " << oc.arg3 << ")";
        break;
      case 3:
        ss << oc.arg1 << ", " << oc.arg2 << " and " << oc.arg3;
        break;
    }
  }

  setMessage(ss.str());
}

InvalidParamNumberError::~InvalidParamNumberError() {
}

ErrorList::ErrorList() : vector<Error>() {
}

ErrorList::~ErrorList() {
}

void ErrorList::add(std::string s, size_t l) {
  Error e(s, l);
  push_back(e);
}

Warning::Warning(std::string message, size_t line): _msg(message), _line(line) {
}

Warning::~Warning() {
}

size_t Warning::line() { 
  return _line; 
}

string Warning::getMessage() { 
  return _msg; 
}

void Warning::setMessage(string s) {
  _msg = s;
}

TooMuchParametersWarning::TooMuchParametersWarning(OpCode oc, size_t line) : Warning("", line) {
  ostringstream ss;
  ss << "Opcode '" << oc.name << "' ";
  
  if (oc.numparams == 0)
    ss << "does not take any parameter.";
  else
    ss << "requires a maximum of " << oc.numparams << " parameter" << (oc.numparams > 1 ? "s" : "") << ".";
  
  ss << " The exceeding one(s) will be ignored";
  setMessage(ss.str());
}

TooMuchParametersWarning::~TooMuchParametersWarning() {
}

WarningList::WarningList() : vector<Warning>() {
}

WarningList::~WarningList() {
}

void WarningList::add(std::string s, size_t l) {
  Warning w(s, l);
  push_back(w);
}



