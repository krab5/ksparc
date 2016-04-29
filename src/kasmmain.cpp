/*
 * kasmmain.cpp -- main function for the assembler program
 * ----
 * This assembler takes a kASM formatted file in input, and gives
 * a binary output that can be used with a ksparc engine to run.
 * 
 * There is two usages :
 *  kasm <input> <output>
 *      for building output from input
 *  kasm <input>
 *      for building input.kbin from input
 * author: krab
 * version: 0.1
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include "logger.h"
#include "assembler.h"

using namespace std;

/*
 * getExtension -- get the extension of a file name
 * filename: the file from which to retrieve the extension
 */
string getExtension(string filename) {
  string ext = "";
  auto it = filename.begin();
  
  while ((*it) != '.' && it != filename.end())
    it++;

  if (it == filename.end())
    return ext;

  while (it != filename.end()) {
    ext += (*it);
    it++;
  }

  return ext;
}

/*
 * replaceExtension -- replace the extension of filename by next
 */
void replaceExtension(string& filename, string next) {
  string ext = getExtension(filename);
  filename.replace(filename.size() - ext.size(), ext.size(), next);
}

/*
 * readFile -- create a list of lines from file named str
 */
vector<string> readFile(string str) {
  ifstream file(str, ios::in);
  vector<string> lines;
  string line;
  while (getline(file, line)) {
    lines.push_back(line);
  }
  file.close();
  return lines;
}

/*
 * writeFile -- write instructions in a binary file
 */
void writeFile(string str, Assembler::InstructionList il) {
  ofstream file(str, ios::out | ios::binary);
  uint32_t ct;
  char sv;

  for (auto it = il.begin(); it != il.end(); it++) {
    ct = (*it).getContent();
    for (uint8_t i = 0; i < 4; i++) {
      sv = static_cast<char>((uint8_t)((*it).getField((3-i)*8, 8)));
      file.write(&sv, sizeof(sv));
    }
  }

  file.close();
}

/*
 * main function
 */
int main(int argc, char* argv[]) {
  // Process input
  if (argc < 2) {
    cout << "Usage: kasm <input> <output>" << endl;
    return 0;
  }

  string infile(argv[1]), outfile(argv[1]);

  if (argc >= 3)
    outfile = argv[2];
  else
    replaceExtension(outfile, ".kbin");

  // Read the input file
  vector<string> lines = readFile(infile);
  Assembler::InstructionList il;

  // Build the label database
  Assembler asmbl;
  cout << "Building label database..." << endl;
  asmbl.parseForLabels(lines);

  // Compile the file
  cout << "Compiling..." << endl;
  il = asmbl.readAll(lines);

  // Treat errors
  if (asmbl.hasErrors()) {
    ASMError::ErrorList errl = asmbl.errors();
    for (auto it = errl.begin(); it != errl.end(); it++)
      cout << "Error at line " << (*it).line() << ": " << (*it).getMessage() << endl;

    cout << "Compilation aborted !" << endl;
    return -1;
  }

  // Treat warnings
  if (asmbl.hasWarnings()) {
    ASMError::WarningList warnl = asmbl.warnings();
    for (auto it = warnl.begin(); it != warnl.end(); it++)
      cout << "Warning at line " << (*it).line() << ": " << (*it).getMessage() << endl;
  }

  // Write output
  cout << "Write output '" << outfile << "'..." << endl;
  writeFile(outfile, il);

  // End
  cout << "Compilation done !" << endl;
  return 0;
}


