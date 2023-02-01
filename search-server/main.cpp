#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
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

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

bool CheckDobleMinus(const string& symbol) {
    return symbol[0] == '-';
}
bool MinusEndWord(const string& word) {
    return word[word.size() - 1] == '-';
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
    {

        for (const string& word : stop_words_) {
            if (IsValidWord(word)) {
                throw invalid_argument("incorrect stop word"s);
            }
        }
    }

    explicit SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
    {
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {

        const vector<string> words = SplitIntoWordsNoStop(document);

        if (document_id <= SearchServer::INVALID_DOCUMENT_ID || documents_.count(document_id) > 0) {
            throw invalid_argument("incorrect document ID"s);
        }
        for (const string& word : words) {
            if (IsValidWord(word)) {
                throw invalid_argument("incorrect word in document"s);
            }
        }

        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        index_id.push_back(document_id);
    }

    template <typename DocumentPredicate>
    vector<Document>FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        const Query query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

    vector<Document>FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }

    vector<Document>FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    tuple< vector<string>, DocumentStatus > MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);

        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        tuple< vector<string>, DocumentStatus> result = { matched_words, documents_.at(document_id).status };
        return result;
    }

    int GetDocumentCount() const {
        return static_cast<int>(documents_.size());
    }

    int GetDocumentId(int index) const {
        if (index < 0 || index > index_id.size()) {
            throw out_of_range("incorrect index"s);
        }
        return index_id.at(index);
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> index_id;
    inline static constexpr int INVALID_DOCUMENT_ID = -1;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    bool IsValidWord(const string& word) const {
        return !(none_of(word.begin(), word.end(),
            [](char c) { return c >= '\0' && c < ' '; }));
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    int ComputeAverageRating(const vector<int>& ratings) const {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        if (text == "-"s) {
            throw invalid_argument("incorrect word in query"s);
        }
        bool is_minus = false;

        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        if (CheckDobleMinus(text) || MinusEndWord(text) || IsValidWord(text)) {
            throw invalid_argument("incorrect word in query"s);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};

// ==================== для примера =========================
void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
int main() {
    try
    {
        SearchServer serch_server("dsdsd\x12sds ds ds df gf"s);
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in constructor: "s << e.what() << endl;
    }

    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(1, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument ID: "s << e.what() << endl;
    }
    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(2, "пушис\x12тый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in AddDocument: "s << e.what() << endl;
    }
    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(-2, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in ID: "s << e.what() << endl;
    }
    try//////////////////
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(2, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.FindTopDocuments("к\x12от"s);
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in FindTopDocuments: "s << e.what() << endl;
    }
    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(2, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.MatchDocument("ко\x12т"s, 2);
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in MatchDocument: "s << e.what() << endl;
    }
    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(2, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.GetDocumentId(-21);
    }
    catch (const out_of_range& e)
    {
        cout << "out_of_range: "s << e.what() << endl;
    }
    try
    {
        SearchServer serch_server("и на в"s);
        serch_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2 });
        serch_server.AddDocument(2, "пушистый кот"s, DocumentStatus::ACTUAL, { 1, 2 });
        for (const auto& doc : serch_server.FindTopDocuments("пушистый модный пес"s)) {
            PrintDocument(doc);
        }
    }
    catch (const invalid_argument& e)
    {
        cout << "invalid_argument in FindTopDocuments: "s << e.what() << endl;
    }