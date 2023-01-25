#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double MAX_DEVIATION = 1e-6;

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
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }
    template<typename SortBy>
    vector<Document> FindTopDocuments(const string& raw_query, SortBy SortKey) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, SortKey);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < MAX_DEVIATION) {
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

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus statuss = DocumentStatus::ACTUAL) const {

        return FindTopDocuments(raw_query, [statuss](int document_id, DocumentStatus status, int rating) {return status == statuss; });
    }


    int GetDocumentCount() const {
        return static_cast<int>(documents_.size());
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
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
        return { matched_words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;

        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

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


    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename SortBy>
    vector<Document> FindAllDocuments(const Query& query, SortBy SortKey) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double IDF = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, TF] : word_to_document_freqs_.at(word)) {
                const auto& DocumentById = documents_.at(document_id);
                if (SortKey(document_id, DocumentById.status, DocumentById.rating)) {
                    document_to_relevance[document_id] += TF * IDF;
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


ostream& operator <<(ostream& os, DocumentStatus status) {
    vector<string> DocStatus = { "ACTUAL"s, "IRRELEVANT"s, "BANNED"s, "REMOVED"s, };
    os << DocStatus[static_cast<int>(status)];
    return os;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))


// Поддержка стоп-слов. Стоп-слова исключаются из текста документов.
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

//Добавление документов.Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestAddDocument()
{
    vector<int> DocId = { 0,1,2,3 };

    {
        SearchServer server;
        server.AddDocument(DocId[0], "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
        server.AddDocument(DocId[1], "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        server.AddDocument(DocId[2], "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
        server.AddDocument(DocId[3], "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });

        const auto& found_docs = server.FindTopDocuments("fluffy soigne cat"s);

        ASSERT_EQUAL(found_docs.size(), 3);

        for (const Document& doc : found_docs) {
            ASSERT(count(DocId.begin(), DocId.end(), doc.id));
        }
    }

    {
        SearchServer server;
        server.AddDocument(DocId[0], "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
        server.AddDocument(DocId[1], "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        server.AddDocument(DocId[2], "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
        server.AddDocument(DocId[3], "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });

        const auto& found_docs = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::BANNED);

        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, DocId[3]);
    }
}


//Поддержка минус-слов. Документы, содержащие минус-слова из поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("-in"s).empty());
    }

}

//Соответствие документов поисковому запросу.При этом должны быть возвращены все слова из поискового запроса,
//присутствующие в документе.Если есть соответствие хотя бы по одному минус - слову, должен возвращаться пустой список слов.
void TestMatchedDocuments()
{
    {
        SearchServer server;
        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
        const int document_count = server.GetDocumentCount();
        for (int document_id = 0; document_id < document_count; ++document_id)
        {
            //сверяем запрос с документом
            const auto& [words, status] = server.MatchDocument("fluffy cat"s, document_id);
            ASSERT_EQUAL(words.size(), 1);
            ASSERT_EQUAL(document_id, 0);
            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
        }
    }

    {
        SearchServer server;
        server.SetStopWords("cat"s);
        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
        const int document_count = server.GetDocumentCount();
        for (int document_id = 0; document_id < document_count; ++document_id)
        {
            //сверяем запрос с документом, но накладываем еще стоп слово
            const auto& [words, status] = server.MatchDocument("fluffy cat"s, document_id);
            ASSERT_EQUAL(words.size(), 0);
            ASSERT_EQUAL(document_id, 0);
            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
        }
    }

    {
        SearchServer server;
        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
        const int document_count = server.GetDocumentCount();
        for (int document_id = 0; document_id < document_count; ++document_id)
        {
            //сверяем запрос с документом, но добавляем минус слово в запрос
            const auto& [words, status] = server.MatchDocument("fluffy -cat"s, document_id);
            ASSERT_EQUAL(words.size(), 0);
            ASSERT_EQUAL(document_id, 0);
            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
        }
    }
}

//Сортировка найденных документов по релевантности. 
void RelevanceRest() {
    SearchServer server;
    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });

    const auto& documents = server.FindTopDocuments("fluffy soigne cat"s);

    int doc_size = static_cast<int>(documents.size());

    for (int i = 0; i < doc_size - 1; ++i)
    {
        ASSERT_HINT(documents[i].relevance > documents[i + 1].relevance, "order is not observed");
    }
}

//Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void RaitingTest() {
    SearchServer server;
    vector<int> RaitingDoc1 = { 8, -3 };
    vector<int> RaitingDoc2 = { 7, 2, 7 };
    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, RaitingDoc1);
    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, RaitingDoc2);

    const auto& documents = server.FindTopDocuments("fluffy soigne cat"s);

    ASSERT_EQUAL(documents[0].rating, (accumulate(RaitingDoc2.begin(), RaitingDoc2.end(), 0) / static_cast<int>(RaitingDoc2.size())));
    ASSERT_EQUAL(documents[1].rating, (accumulate(RaitingDoc1.begin(), RaitingDoc1.end(), 0) / static_cast<int>(RaitingDoc1.size())));
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void UserPredTest() {
    SearchServer server;
    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });

    const auto documents = server.FindTopDocuments("fluffy soigne cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });

    for (const auto& doc : documents) {
        ASSERT(!(doc.id % 2));
    }
}

// Поиск документов, имеющих заданный статус.
void UserStatusTest() {
    SearchServer server;
    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::IRRELEVANT, { 7, 2, 7 });
    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::REMOVED, { 5, -12, 2 });
    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
    {
        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL(documents.size(), 1);
    }

    {
        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL(documents.size(), 1);
    }

    {
        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL(documents.size(), 1);
    }

    {
        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(documents.size(), 1);
    }
}

// Корректность рассчёта релевантности документов.
void RelevanceTest() {
    vector<int> DocId = { 0,1,2,3 };
    vector<string> DocText = { "white cat and fancy collar"s ,
                               "fluffy cat fluffy tail"s ,
                               "soigne dog expressive eyes"s ,
                               "soigne starling eugeny"s };

    SearchServer server;

    server.AddDocument(DocId[0], DocText[0], DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(DocId[1], DocText[1], DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(DocId[2], DocText[2], DocumentStatus::ACTUAL, { 5, -12, 2 });
    server.AddDocument(DocId[3], DocText[3], DocumentStatus::ACTUAL, { 9 });

    const auto documents = server.FindTopDocuments("fluffy soigne cat"s);

    //Рассчёт релевантности документов для сравнения с рассчётом из класса
    map<string, map<int, double>> word_to_document_freqs_;
    vector<string> PlusWords = { "fluffy"s, "soigne"s, "cat"s };
    vector<string> DocWords;
    map<int, double> document_to_relevance;

    for (int i = 0; i < static_cast<int>(DocText.size()); i++) {
        DocWords = SplitIntoWords(DocText[i]);
        const double inv_word_count = 1.0 / DocWords.size();
        for (int j = 0; j < static_cast<int>(DocWords.size()); j++) {
            word_to_document_freqs_[DocWords[j]][DocId[i]] += inv_word_count;
        }
    }

    for (const string& word : PlusWords) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double IDF = log(DocId.size() * 1.0 / word_to_document_freqs_.at(word).size());
        for (const auto& [document_id, TF] : word_to_document_freqs_.at(word)) {

            document_to_relevance[document_id] += TF * IDF;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    //проверим сходство рассчётных значений и значений полученных  в классе

    for (const auto& [id, rel] : document_to_relevance) {
        for (const Document& doc : documents) {
            if (doc.id == id) {
                ASSERT((abs(rel - doc.relevance) < MAX_DEVIATION));
            }
        }
    }
}

#define RUN_TEST(func) func(); 

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchedDocuments);
    RUN_TEST(RelevanceRest);
    RUN_TEST(RaitingTest);
    RUN_TEST(UserStatusTest);
    RUN_TEST(UserPredTest);
    RUN_TEST(RelevanceTest);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();

    cout << "Search server testing finished"s << endl;
}