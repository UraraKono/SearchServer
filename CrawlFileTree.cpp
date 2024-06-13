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

#include "./CrawlFileTree.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

#include "./FileReader.hpp"

using std::string;
using std::vector;

namespace searchserver {

//////////////////////////////////////////////////////////////////////////////
// Internal helper functions and constants
//////////////////////////////////////////////////////////////////////////////

// Recursively descend into the passed-in directory, looking for files and
// subdirectories.  Any encountered files are processed via handle_file(); any
// subdirectories are recusively handled by handle_dir().
//
// Note that opendir()/readdir() iterates through a directory's entries in an
// unspecified order; since we need the ordering to be consistent in order
// to generate consistent DocTables and MemIndices, we do two passes over the
// contents: the first to extract the data necessary for populating
// entry_name_st and the second to actually handle the recursive call.
static void handle_dir(const string& dir_path, DIR* d, WordIndex* index);

// Read and parse the specified file, then inject it into the MemIndex.
static void handle_file(const string& fpath, WordIndex* index);

//////////////////////////////////////////////////////////////////////////////
// Externally-exported functions
//////////////////////////////////////////////////////////////////////////////

bool crawl_filetree(const string& root_dir, WordIndex* index) {
  struct stat root_stat;
  DIR* rd;

  // Verify we got some valid args.
  if (index == nullptr) {
    return false;
  }

  // Verify that root_dir is a directory.
  if (stat(root_dir.c_str(), &root_stat) == -1) {
    // We got some kind of error stat'ing the file. Give up
    // and return an error.
    return false;
  }

  if (!S_ISDIR(root_stat.st_mode)) {
    // It isn't a directory, so give up.
    return false;
  }

  // Try to open the directory using opendir().  If we fail, (e.g., we don't
  // have permissions on the directory), return a failure. ("man 3 opendir")
  rd = opendir(root_dir.c_str());
  if (rd == NULL) {
    return false;
  }

  // Begin the recursive handling of the directory.
  // std::cout << "calling handle_dir" << std::endl;
  handle_dir(root_dir, rd, index);

  // All done.  Release and/or transfer ownership of resources.
  closedir(rd);
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// Internal helper functions
//////////////////////////////////////////////////////////////////////////////

static void handle_dir(const string& dir_path, DIR* d, WordIndex* index) {
  // We make two passes through the directory.  The first gets the list of
  // all the metadata necessary to process its entries; the second iterates
  // does the actual recursive descent.

  struct dirent* dirent;
  struct stat st;

  // Use the "readdir()" system call to read the directory entries in a
  // loop ("man 3 readdir").  Exit out of the loop when we reach the end
  // of the directory.

  // First pass, to populate the "entries" list of item metadata.
  for (dirent = readdir(d); dirent != NULL; dirent = readdir(d)) {
    // If the directory entry is named "." or "..", ignore it.  Use the C
    // "continue" expression to begin the next iteration of the loop.  What
    // field in the dirent could we use to find out the name of the entry?
    // How do you compare strings in C?
    if ((strcmp(dirent->d_name, ".") == 0) ||
        (strcmp(dirent->d_name, "..") == 0)) {
      // We need to pre-emptively decrement 'i' so that we don't erroneously
      // include "." and ".." in our final entry count.
      continue;
    }

    // We need to append the name of the file to the name of the directory
    // we're in to get the full filename. So, we'll malloc space for:
    //     dirpath + "/" + dirent->d_name + '\0'
    string path = dir_path;
    string entry_name = dirent->d_name;
    if (dir_path.back() != '/') {
      path += '/';
    }
    path += entry_name;

    // Use the "stat()" system call to ask the operating system to give us
    // information about the file identified by the directory entry (see
    // also "man 2 stat").
    if (stat(path.c_str(), &st) == 0) {
      // Test to see if the file is a "regular file" using the S_ISREG() macro
      // described in the stat man page. If so, we'll process the file by
      // eventually invoking the handle_file() private helper function in our
      // second pass.
      //
      // On the other hand, if the file turns out to be a directory (which you
      // can find out using the S_ISDIR() macro described on the same page),
      // then we'll need to recursively process it using/ handle_dir() in our
      // second pass.
      //
      // If it is neither, skip the file.

      if (S_ISREG(st.st_mode)) {
        // std::cout << "calling handle_file in handle_dir. path:" << path
        //           << ", index: " << index << std::endl;
        handle_file(path, index);
      } else if (S_ISDIR(st.st_mode)) {
        DIR* sub_dir = opendir(path.c_str());
        if (sub_dir != NULL) {
          handle_dir(path, sub_dir, index);
          closedir(sub_dir);
        }
      } else {
        // This is neither a file nor a directory, so ignore it.  As before,
        // we pre-emptively decrement 'i' so that it doesn't get included
        // in our final entry count.
        continue;
      }
    }
  }
}

static bool is_not_alpha(int c) {
  // return !std::isalpha(static_cast<unsigned char>(c));
  return !isalpha(c);
  // return (
  // isalpha(c) ==
  // 0);  // isalpha() returns nonzero value if c is alphabetic, zero if not
}

static void handle_file(const string& fpath, WordIndex* index) {
  // TODO: implement
  // std::cout << "beginning of handle_file()" << std::endl;
  // Read the contents of the specified file into a string
  FileReader fileReader{fpath};
  string contents{};
  // string* contents{};
  //   std::cout << "about to read file" << std::endl;
  bool file_read = fileReader.read_file(&contents);
  // bool file_read = fileReader.read_file(contents);
  // std::cout << "finished fileReader.read_file()" << std::endl;
  if (file_read) {
    // std::cout << "file read" << std::endl;
  } else {
    // std::cout << "file not read" << std::endl;
  }
  // Search the string for all tokens, where anything that is
  // not an alphabetic character is considered a delimiter
  // A delimiter marks the end of a token and is not considered a valid part of
  // the token
  vector<string> splitVec{};
  // std::cout
  //     << "about to call boost::split() in handle_file(). size of the
  //     splitvec: "
  //     << splitVec.size() << std::endl;
  // boost::split(splitVec, *contents, is_not_alpha, boost::token_compress_on);
  boost::split(splitVec, contents, is_not_alpha, boost::token_compress_on);
  // boost::split(splitVec, contents, [](char c) { return
  // !isalpha(static_cast<unsigned char>(c)); }, boost::token_compress_on);
  // boost::split(
  //     splitVec, contents,
  //     [](char c) { return !boost::algorithm::is_alpha()(c); },
  //     boost::token_compress_on);

  // Record each non empty token as a word into the Wordindex specified by
  // *index
  for (string& token : splitVec) {
    // convert to lower-case
    boost::to_lower(token);
    // std::cout << "token: " << token << std::endl;

    if (!token.empty()) {
      index->record(token, fpath);
    }
  }

  // std::cout << "reached end of handle_file()" << std::endl;

  // Your implementation should also be case in-sensitive and record every word
  // in all lower-case
}

}  // namespace searchserver
