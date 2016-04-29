/*
 * logger.cpp -- implementation of the Logger class
 */
#include "logger.h"

using namespace std;

// Singleton instance
Logger* Logger::_instance = 0;

// Static initialization
void Logger::init(string file) {
  _instance = new Logger(file);
}

// Static destruction
void Logger::destroy() {
  delete _instance;
}

// Cstr
Logger::Logger(string file) {
  _stream.open(file, ios::out);
}

// Dstr
Logger::~Logger() {
  _stream.close();
}

// Log a string
void Logger::log(string s) {
  _instance->_stream << s << endl;
}

// Return stream
ostream& Logger::log() {
  return _instance->_stream;
}



