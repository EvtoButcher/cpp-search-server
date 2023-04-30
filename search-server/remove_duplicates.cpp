#include <iostream>
#include <string>
#include <set>
#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server)
{
    set<int> to_dell;
    set<set<string>> words_docs;

    for (const int sample_document_id : search_server) {
        set<string> words;
        for (const auto& [word, _] : search_server.GetWordFrequencies(sample_document_id)) {
            words.insert(static_cast<string>(word));
        }
        if (!binary_search(words_docs.begin(), words_docs.end(), words)) {
            words_docs.insert(words);
        }
        else {
            to_dell.insert(sample_document_id);
        }
    }

    for (auto id : to_dell) {
        cout << "Found duplicate document id "s << id << "\n";
        search_server.RemoveDocument(id);
    }
}
