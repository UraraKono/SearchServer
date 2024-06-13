/*
 * Copyright ©2024 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2024 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "./FileReader.hpp"
#include "./HttpUtils.hpp"

using std::ifstream;
using std::string;

namespace searchserver {

bool FileReader::read_file(string* str) {
  // Read the file into memory, and store the file contents in the
  // output parameter "str."  Be careful to handle binary data
  // correctly; i.e., you probably want to use the two-argument
  // constructor to std::string (the one that includes a length as a
  // second argument).

  // TODO: implement
  // open file
  std::ifstream file(this->fname_, std::ifstream::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file:" << this->fname_ << std::endl;
    return false;
  }
  // std::cout << "file.is_open(): " << file.is_open() << std::endl;

  // get length of file
  file.seekg(0, file.end);
  int length = file.tellg();
  file.seekg(0, file.beg);

  // std::cout << "length = " << length << std::endl;

  // read file
  char* buffer = new char[length];
  if (file.read(buffer, length)) {
    // std::cout << "constructing std::string" << std::endl;
    *str = std::string(buffer, length);
    // string str2(buffer, length);
    // std::cout << "str2 constructed" << std::endl;
    //*str = std::move(str2);
    // str = &str2;
    // std::cout << "constructed string" << std::endl;
  } else {
    std::cerr << "Failed to read file:" << this->fname_ << std::endl;
    file.close();
    delete[] buffer;
    return false;
  }

  file.close();
  delete[] buffer;
  return true;
}

}  // namespace searchserver
