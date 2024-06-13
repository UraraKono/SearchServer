#ifndef WORD_INDEX_HPP_
#define WORD_INDEX_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "./Result.hpp"

using std::string;
using std::unordered_map;
using std::vector;

namespace searchserver {

// A WordIndex is used to keep track of which documents contain certain words
// and how many occurances there are of that word in the document
class WordIndex {
 public:
  // Constructs an empty WordIndex that stores
  // no words or documents to start
  WordIndex();

  // Returns the number of unique words recorded in the index
  size_t num_words();

  // Record an occurance of a document having the specified word show up in it
  //
  // Arguments:
  //  - word: the word found in the specified document
  //  - doc_name: the name of the document the word occurance showed up in
  //
  // Returns: None
  void record(const string& word, const string& doc_name);

  // Lookup a word in the index, getting a sorted list of all documents that
  // contain the word and a rank which is the number of occurances of that word
  // in the document
  //
  // Arguments:
  //  - word: a word we are looking up results for
  //
  // Returns:
  //  - A list of results. Each result contains a document name and the number
  //    of recorded occurances of the specified word in that document. The list
  //    is sorted with documents with the highest rank at the front.
  vector<Result> lookup_word(const string& word);

  // Lookup a query (multiple words) in the index, getting a sorted list of all
  // documents that contain each word in the query and a rank which is the
  // number of occurances of each word in the document.
  //
  // Every Result in the returned vector contains every word in the input query
  // In other words, the returned vector is the intersection of looking up
  // each word in the query
  //
  // Arguments:
  //  - word: a word we are looking up results for
  //
  // Returns:
  //  - A list of results. Each result contains a document name and the sum of
  //  the
  //    number of recorded occurances of the each query word in that document.
  //    The list is sorted with documents with the highest rank at the front.
  vector<Result> lookup_query(const vector<string>& query);

  // delete cctor and op=
  WordIndex(const WordIndex& other) = delete;
  WordIndex& operator=(const WordIndex& other) = delete;

 private:
  // TODO: add
  // key: word, value: unordered_map with key: document name, word count
  unordered_map<string, unordered_map<string, size_t>> index_;
};

}  // namespace searchserver

#endif  // WORD_INDEX_HPP_
