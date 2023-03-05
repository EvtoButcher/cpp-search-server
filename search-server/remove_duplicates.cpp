#include <iostream>
#include <string>
#include <set>
#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server)
{

    set<int> to_dell;
    map<int, set<string>> words_docs;
    
    for (const int sample_document_id : search_server) {
        for (const auto& [word, _] : search_server.GetWordFrequencies(sample_document_id)) {
            words_docs[sample_document_id].insert(word);
        }
    }
    for (const int sample_document_id : search_server) {
        int new_bound = sample_document_id;
        for (auto iter_compared_document = words_docs.upper_bound(sample_document_id); iter_compared_document != words_docs.end(); iter_compared_document = words_docs.upper_bound(new_bound)) {
            if (words_docs.at(sample_document_id) == words_docs.at((*iter_compared_document).first)) {
                sample_document_id > (*iter_compared_document).first ? to_dell.insert(sample_document_id) : to_dell.insert((*iter_compared_document).first);
            }
            new_bound = (*iter_compared_document).first;
        }
    }

    for (auto id : to_dell) {
        cout << "Found duplicate document id "s << id << "\n";
        search_server.RemoveDocument(id);
    }
}
