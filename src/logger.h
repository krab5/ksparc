/*
 * logger.h -- defines a logger class for debugging
 * ------
 * Author: krab
 * Version: 0.1
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

/**
 * The Logger class is a singleton that can be used by every class whatsoever.
 * Its purpose is to create debugging logs in a file and provide two functions called log : one that write a string into a file and the other that returns the stream interface of the file, allowing for easy data writing.
 */
class Logger {
	public:
    /**
     * Static initialization of the singleton's instance.
     * @param file name of the output file (by default, output.log)
     */
		static void init(std::string file = "output.log");
    /**
     * Static destruction of the singleton's instance.
     */
    static void destroy();

    /**
     * Log a string message
     * @param s the message
     */
    static void log(std::string s);
    /**
     * Return the stream of the file
     * @returns the output stream interface of the log file
     */
    static std::ostream& log();


	private:
    /**
     * Private constructor
     * @param file name of the log file
     */
    Logger(std::string file);
    /**
     * Private destructor
     */
		~Logger();
  
    /**
     * File stream
     */
    std::fstream _stream;
    /**
     * Private static instance
     */
    static Logger* _instance;
};

#endif // LOGGER_H

