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

#include <boost/algorithm/string.hpp>
// #include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "./HttpConnection.hpp"
#include "./HttpRequest.hpp"
#include "./HttpUtils.hpp"

using std::map;
using std::optional;
using std::string;
using std::vector;

namespace searchserver {

static const char* kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

// reads till eof from specified fd. nullopt on error

bool HttpConnection::next_request(HttpRequest* request) {
  // Use "wrapped_read" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header.
  //
  // Once you've seen the request header, use parse_request()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes next_request()!

  // TODO: implement
  while (true) {
    if (buffer_.size() == 0) {
      int res = wrapped_read(fd_, &buffer_);

      if (res == -1) {
        std::cerr << "res == -1, returning false" << std::endl;
        return false;
      } else if (res == 0) {
        // std::cerr << "res == 0" << std::endl;
        return false;
      }
    }
    size_t end_pos = buffer_.find(kHeaderEnd);
    if (end_pos != string::npos) {
      string request_str = buffer_.substr(0, end_pos);

      if (buffer_.size() > end_pos + kHeaderEndLen) {
        buffer_ = buffer_.substr(end_pos + kHeaderEndLen);
      } else {
        buffer_ = "";
      }
      return parse_request(request_str, request);
    } else {
      wrapped_read(fd_, &buffer_);
    }
  }
}

bool HttpConnection::write_response(const HttpResponse& response) {
  // Implement so that the response is converted to a string
  // and written out to the socket for this connection

  // TODO: implement
  string responseString = response.GenerateResponseString();
  if (wrapped_write(fd_, responseString) ==
      static_cast<int>(responseString.size())) {
    return true;
  }
  return false;
}

bool HttpConnection::parse_request(const string& request, HttpRequest* out) {
  // std::cout << "entered parse_request()" << std::endl;
  HttpRequest req("/");  // by default, get "/".

  // Split the request into lines.
  // std::cout << "splitting request into lines" << std::endl;
  vector<string> requestSplit{};

  // boost::split_regex(requestSplit, request, boost::regex("\r\n"));
  // boost::trim(request);
  boost::split(requestSplit, request, boost::is_any_of("\r\n"),
               boost::token_compress_on);
  if (requestSplit.empty()) {
    std::cout << "requestSplit is empty" << std::endl;
    return false;
  }

  // Extract the URI from the first line
  // and store it in req.URI.
  // std::cout << "extracting URI from first line" << std::endl;
  vector<string> firstLineSplit{};
  boost::split(firstLineSplit, requestSplit.at(0), boost::is_any_of(" "),
               boost::token_compress_on);
  if (firstLineSplit.at(0) != "GET") {
    return false;
  }
  string& uri = firstLineSplit.at(1);
  req.set_uri(uri);

  // For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
  // at HttpRequest.h for details about the HTTP header format that
  // you need to parse.
  // std::cout << "extracting the other lines" << std::endl;
  for (auto it = requestSplit.begin() + 1; it < requestSplit.end(); ++it) {
    if (!it->empty()) {
      vector<string> lineSplit{};
      // boost::split_regex(lineSplit, *it, boost::regex(": "));
      // std::cout << "splitting line" << std::endl;
      boost::split(lineSplit, *it, boost::is_any_of(": "),
                   boost::token_compress_on);
      // std::cout << "size of lineSplit: " << lineSplit.size() << std::endl;
      // std::cout << "contents of lineSplit: [" << lineSplit.at(0) << "]"
      //           << std::endl;
      // std::cout << lineSplit.at(0) << " " << lineSplit.at(1) << std::endl;
      // std::cout << "converting header name to lowercase" << std::endl;
      boost::to_lower(lineSplit.at(0));
      // std::cout << "adding header" << std::endl;
      req.AddHeader(lineSplit.at(0), lineSplit.at(1));
    }
  }

  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.

  // If a request is malfrormed, return false, otherwise true and
  // the parsed request is returned via *out
  // std::cout << "returning from parse_request()" << std::endl;
  *out = std::move(req);
  return true;

  // TODO: implement
}

}  // namespace searchserver
