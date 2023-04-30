#include "string_processing.h"
using namespace std;

vector<string> SplitIntoWords(string_view text)
{
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

vector<string_view> SplitIntoWordsView(string_view str) {
    vector<string_view> words;
  
    auto pos = str.find_first_not_of(" ");
    const auto pos_end = str.npos;
    
    while (pos != pos_end) {
       
        auto space = str.find(' ', pos);
        
        words.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
        
        pos = str.find_first_not_of(" ", space);
    }

    return words;
}
