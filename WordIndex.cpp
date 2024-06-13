#include "./WordIndex.hpp"
#include <algorithm>

namespace searchserver {

WordIndex::WordIndex() : index_{} {
  // TODO: implement
}

size_t WordIndex::num_words() {
  // TODO: implement
  return index_.size();
}

void WordIndex::record(const string& word, const string& doc_name) {
  // TODO: implement
  index_[word][doc_name]++;
}

vector<Result> WordIndex::lookup_word(const string& word) {
  vector<Result> result;
  // TODO: implement
  if (index_.find(word) != index_.end()) {
    for (const auto& [doc_name, count] : index_[word]) {
      result.emplace_back(doc_name, count);
    }
  }

  // sort
  std::sort(result.begin(), result.end());

  return result;
}

vector<Result> WordIndex::lookup_query(const vector<string>& query) {
  vector<Result> results;
  unordered_map<string, size_t> doc_count;

  for (const string& word : query) {
    vector<Result> word_results = lookup_word(word);
    for (const Result& res : word_results) {
      doc_count[res.doc_name] += res.rank;
    }
  }

  // filter out documents that do not contain all words
  for (const auto& [doc_name, count] : doc_count) {
    bool all_words_found = true;
    for (const string& word : query) {
      if (index_[word].find(doc_name) == index_[word].end()) {
        all_words_found = false;
        break;
      }
    }
    if (all_words_found) {
      results.emplace_back(doc_name, count);
    }
  }
  std::sort(results.begin(), results.end());

  return results;
}

}  // namespace searchserver
