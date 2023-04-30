//#include "process_queries.h"
//#include "search_server.h"
//
//#include <iostream>
//#include <string>
//#include <vector>
//
//using namespace std;
//
//int main() {
//    //{
//    //    SearchServer search_server("and with"s);
//    //    int id = 0;
//    //    for (
//    //        const string& text : {
//    //            "funny pet and nasty rat"s,
//    //            "funny pet with curly hair"s,
//    //            "funny pet and not very nasty rat"s,
//    //            "pet with rat and rat and rat"s,
//    //            "nasty rat with curly hair"s,
//    //        }
//    //        ) {
//    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//    //    }
//    //    const vector<string> queries = {
//    //        "nasty rat -not"s,
//    //        "not very funny nasty pet"s,
//    //        "curly hair"s
//    //    };
//    //    id = 0;
//
//    //    for (
//    //        const auto& documents : ProcessQueries(search_server, queries)
//    //        ) {
//    //        cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
//    //    }
//
//    //    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
//    //        cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
//    //    }
//    //}
//    //{
//
//    //    SearchServer search_server("and with"s);
//
//    //    int id = 0;
//    //    for (
//    //        const string& text : {
//    //            "funny pet and nasty rat"s,
//    //            "funny pet with curly hair"s,
//    //            "funny pet and not very nasty rat"s,
//    //            "pet with rat and rat and rat"s,
//    //            "nasty rat with curly hair"s,
//    //        }
//    //        ) {
//    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//    //    }
//
//    //    const string query = "curly and funny"s;
//
//    //    auto report = [&search_server, &query] {
//    //        cout << search_server.GetDocumentCount() << " documents total, "s
//    //            << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
//    //    };
//
//    //    report();
//    //    // однопоточна€ верси€
//    //    search_server.RemoveDocument(5);
//    //    report();
//    //    // однопоточна€ верси€
//    //    search_server.RemoveDocument(execution::seq, 1);
//    //    report();
//    //    // многопоточна€ верси€
//    //    search_server.RemoveDocument(execution::par, 2);
//    //    report();
//    //}
//    {
//
//        SearchServer search_server("and with"s);
//
//        int id = 0;
//        for (
//            const string& text : {
//                "funny pet and nasty rat"s,
//                "funny pet with curly hair"s,
//                "funny pet and not very nasty rat"s,
//                "pet with rat and rat and rat"s,
//                "nasty rat with curly hair"s,
//            }
//            ) {
//            search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//        }
//
//
//
//        const string query = "curly and funny -not"s;
//
//        {
//            const auto [words, status] = search_server.MatchDocument(query, 1);
//            cout << words.size() << " words for document 1"s << endl;
//            // 1 words for document 1
//        }
//
//        {
//            const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
//            cout << words.size() << " words for document 2"s << endl;
//            // 2 words for document 2
//        }
//
//        {
//            const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
//            cout << words.size() << " words for document 3"s << endl;
//            // 0 words for document 3
//        }
//    }
//
//    return 0;
//}

//#include "search_server.h"
//
//#include <execution>
//#include <iostream>
//#include <random>
//#include <string>
//#include <vector>
//
//
//#include "log_duration.h"
//
//using namespace std;
//
//string GenerateWord(mt19937& generator, int max_length) {
//    const int length = uniform_int_distribution(1, max_length)(generator);
//    string word;
//    word.reserve(length);
//    for (int i = 0; i < length; ++i) {
//        word.push_back(uniform_int_distribution(0, 26)(generator) + 'a');
//    }
//    return word;
//}
//
//vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
//    vector<string> words;
//    words.reserve(word_count);
//    for (int i = 0; i < word_count; ++i) {
//        words.push_back(GenerateWord(generator, max_length));
//    }
//    sort(words.begin(), words.end());
//    words.erase(unique(words.begin(), words.end()), words.end());
//    return words;
//}
//
//string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
//    string query;
//    for (int i = 0; i < word_count; ++i) {
//        if (!query.empty()) {
//            query.push_back(' ');
//        }
//        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
//            query.push_back('-');
//        }
//        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//    }
//    return query;
//}
//
//vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
//    vector<string> queries;
//    queries.reserve(query_count);
//    for (int i = 0; i < query_count; ++i) {
//        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
//    }
//    return queries;
//}
//
//template <typename ExecutionPolicy>
//void Test(string mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
//    LOG_DURATION(mark);
//    const int document_count = search_server.GetDocumentCount();
//    int word_count = 0;
//    for (int id = 0; id < document_count; ++id) {
//        const auto [words, status] = search_server.MatchDocument(policy, query, id);
//        word_count += words.size();
//    }
//    cout << word_count << endl;
//}
//
//#define TEST(policy) Test(#policy, search_server, query, execution::policy)
//
//ostream& operator <<(ostream& os, DocumentStatus status) {
//    vector<string> DocStatus = { "ACTUAL"s, "IRRELEVANT"s, "BANNED"s, "REMOVED"s, };
//    os << DocStatus[static_cast<int>(status)];
//    return os;
//}
//
//template <typename T, typename U>
//void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
//    const string& func, unsigned line, const string& hint) {
//    if (t != u) {
//        cerr << boolalpha;
//        cerr << file << "("s << line << "): "s << func << ": "s;
//        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
//        cerr << t << " != "s << u << "."s;
//        if (!hint.empty()) {
//            cerr << " Hint: "s << hint;
//        }
//        cerr << endl;
//        abort();
//    }
//}
//
//#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
//    const string& hint) {
//    if (!value) {
//        cerr << file << "("s << line << "): "s << func << ": "s;
//        cerr << "ASSERT("s << expr_str << ") failed."s;
//        if (!hint.empty()) {
//            cerr << " Hint: "s << hint;
//        }
//        cerr << endl;
//        abort();
//    }
//}
//
//#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
//
//#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
//
//
//// ѕоддержка стоп-слов. —топ-слова исключаютс€ из текста документов.
//void TestExcludeStopWordsFromAddedDocumentContent() {
//    const int doc_id = 42;
//    const string content = "cat in the city"s;
//    const vector<int> ratings = { 1, 2, 3 };
//    {
//        SearchServer server(";"s);
//        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
//        const auto found_docs = server.FindTopDocuments("in"s);
//        ASSERT_EQUAL(found_docs.size(), 1u);
//        const Document& doc0 = found_docs[0];
//        ASSERT_EQUAL(doc0.id, doc_id);
//    }
//
//    {
//        SearchServer server("in the"s);
//        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
//        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
//    }
//}
//
////ƒобавление документов.ƒобавленный документ должен находитьс€ по поисковому запросу, который содержит слова из документа.
//void TestAddDocument()
//{
//    vector<int> DocId = { 0,1,2,3 };
//
//    {
//        SearchServer server(";"s);
//        server.AddDocument(DocId[0], "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
//        server.AddDocument(DocId[1], "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//        server.AddDocument(DocId[2], "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
//        server.AddDocument(DocId[3], "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
//
//        const auto& found_docs = server.FindTopDocuments("fluffy soigne cat"s);
//
//        ASSERT_EQUAL(found_docs.size(), 3);
//
//        for (const Document& doc : found_docs) {
//            ASSERT(count(DocId.begin(), DocId.end(), doc.id));
//        }
//    }
//
//    {
//        SearchServer server(";"s);
//        server.AddDocument(DocId[0], "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
//        server.AddDocument(DocId[1], "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//        server.AddDocument(DocId[2], "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
//        server.AddDocument(DocId[3], "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
//
//        const auto& found_docs = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::BANNED);
//
//        ASSERT_EQUAL(found_docs.size(), 1);
//        const Document& doc0 = found_docs[0];
//        ASSERT_EQUAL(doc0.id, DocId[3]);
//    }
//}
//
//
////ѕоддержка минус-слов. ƒокументы, содержащие минус-слова из поискового запроса, не должны включатьс€ в результаты поиска.
//void TestMinusWords()
//{
//    const int doc_id = 42;
//    const string content = "cat in the city"s;
//    const vector<int> ratings = { 1, 2, 3 };
//
//    {
//        SearchServer server(";"s);
//        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
//        ASSERT(server.FindTopDocuments("-in"s).empty());
//    }
//
//}
//
////—оответствие документов поисковому запросу.ѕри этом должны быть возвращены все слова из поискового запроса,
////присутствующие в документе.≈сли есть соответствие хот€ бы по одному минус - слову, должен возвращатьс€ пустой список слов.
//void TestMatchedDocuments()
//{
//    {
//        SearchServer server(";"s);
//        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
//        const int document_count = server.GetDocumentCount();
//        for (int document_id = 0; document_id < document_count; ++document_id)
//        {
//            //свер€ем запрос с документом
//            const auto& [words, status] = server.MatchDocument("fluffy cat"s, document_id);
//            ASSERT_EQUAL(words.size(), 1);
//            ASSERT_EQUAL(document_id, 0);
//            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
//        }
//    }
//
//    {
//        SearchServer server("cat"s);
//        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
//        const int document_count = server.GetDocumentCount();
//        for (int document_id = 0; document_id < document_count; ++document_id)
//        {
//            //свер€ем запрос с документом, но накладываем еще стоп слово
//            const auto& [words, status] = server.MatchDocument("fluffy cat"s, document_id);
//            ASSERT_EQUAL(words.size(), 0);
//            ASSERT_EQUAL(document_id, 0);
//            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
//        }
//    }
//
//    {
//        SearchServer server(";"s);
//        server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, -3 });
//        const int document_count = server.GetDocumentCount();
//        for (int document_id = 0; document_id < document_count; ++document_id)
//        {
//            //свер€ем запрос с документом, но добавл€ем минус слово в запрос
//            const auto& [words, status] = server.MatchDocument("fluffy -cat"s, document_id);
//            ASSERT_EQUAL(words.size(), 0);
//            ASSERT_EQUAL(document_id, 0);
//            ASSERT_EQUAL(status, DocumentStatus::ACTUAL);
//        }
//    }
//}
//
////—ортировка найденных документов по релевантности. 
//void RelevanceRest() {
//    SearchServer server(";"s);
//    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
//    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
//    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
//
//    const auto& documents = server.FindTopDocuments("fluffy soigne cat"s);
//
//    int doc_size = static_cast<int>(documents.size());
//
//    for (int i = 0; i < doc_size - 1; ++i)
//    {
//        ASSERT_HINT(documents[i].relevance > documents[i + 1].relevance, "order is not observed");
//    }
//}
//
////¬ычисление рейтинга документов. –ейтинг добавленного документа равен среднему арифметическому оценок документа.
//void RaitingTest() {
//    SearchServer server(";"s);
//    vector<int> RaitingDoc1 = { 8, -3 };
//    vector<int> RaitingDoc2 = { 7, 2, 7 };
//    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, RaitingDoc1);
//    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, RaitingDoc2);
//
//    const auto& documents = server.FindTopDocuments("fluffy soigne cat"s);
//
//    ASSERT_EQUAL(documents[0].rating, (accumulate(RaitingDoc2.begin(), RaitingDoc2.end(), 0) / static_cast<int>(RaitingDoc2.size())));
//    ASSERT_EQUAL(documents[1].rating, (accumulate(RaitingDoc1.begin(), RaitingDoc1.end(), 0) / static_cast<int>(RaitingDoc1.size())));
//}
//
////‘ильтраци€ результатов поиска с использованием предиката, задаваемого пользователем.
//void UserPredTest() {
//    SearchServer server(";"s);
//    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
//    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2 });
//    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
//
//    const auto documents = server.FindTopDocuments("fluffy soigne cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
//
//    for (const auto& doc : documents) {
//        ASSERT(!(doc.id % 2));
//    }
//}
//
//// ѕоиск документов, имеющих заданный статус.
//void UserStatusTest() {
//    SearchServer server(":"s);
//    server.AddDocument(0, "white cat and fancy collar"s, DocumentStatus::ACTUAL, { 8, -3 });
//    server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::IRRELEVANT, { 7, 2, 7 });
//    server.AddDocument(2, "soigne dog expressive eyes"s, DocumentStatus::REMOVED, { 5, -12, 2 });
//    server.AddDocument(3, "soigne starling eugeny"s, DocumentStatus::BANNED, { 9 });
//    {
//        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::ACTUAL);
//        ASSERT_EQUAL(documents.size(), 1);
//    }
//
//    {
//        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::IRRELEVANT);
//        ASSERT_EQUAL(documents.size(), 1);
//    }
//
//    {
//        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::REMOVED);
//        ASSERT_EQUAL(documents.size(), 1);
//    }
//
//    {
//        const auto documents = server.FindTopDocuments("fluffy soigne cat"s, DocumentStatus::BANNED);
//        ASSERT_EQUAL(documents.size(), 1);
//    }
//}
//
////  орректность рассчЄта релевантности документов.
//void RelevanceTest() {
//    vector<int> DocId = { 0,1,2,3 };
//    vector<string> DocText = { "white cat and fancy collar"s ,
//                               "fluffy cat fluffy tail"s ,
//                               "soigne dog expressive eyes"s ,
//                               "soigne starling eugeny"s };
//
//    SearchServer server(":"s);
//
//    server.AddDocument(DocId[0], DocText[0], DocumentStatus::ACTUAL, { 8, -3 });
//    server.AddDocument(DocId[1], DocText[1], DocumentStatus::ACTUAL, { 7, 2, 7 });
//    server.AddDocument(DocId[2], DocText[2], DocumentStatus::ACTUAL, { 5, -12, 2 });
//    server.AddDocument(DocId[3], DocText[3], DocumentStatus::ACTUAL, { 9 });
//
//    const auto documents = server.FindTopDocuments("fluffy soigne cat"s);
//
//    //–ассчЄт релевантности документов дл€ сравнени€ с рассчЄтом из класса
//    map<string, map<int, double>> word_to_document_freqs_;
//    vector<string> PlusWords = { "fluffy"s, "soigne"s, "cat"s };
//    vector<string> DocWords;
//    map<int, double> document_to_relevance;
//
//    for (int i = 0; i < static_cast<int>(DocText.size()); i++) {
//        DocWords = SplitIntoWords(DocText[i]);
//        const double inv_word_count = 1.0 / DocWords.size();
//        for (int j = 0; j < static_cast<int>(DocWords.size()); j++) {
//            word_to_document_freqs_[DocWords[j]][DocId[i]] += inv_word_count;
//        }
//    }
//
//    for (const string& word : PlusWords) {
//        if (word_to_document_freqs_.count(word) == 0) {
//            continue;
//        }
//        const double IDF = log(DocId.size() * 1.0 / word_to_document_freqs_.at(word).size());
//        for (const auto& [document_id, TF] : word_to_document_freqs_.at(word)) {
//
//            document_to_relevance[document_id] += TF * IDF;
//        }
//    }
//
//    ////////////////////////////////////////////////////////////////////////
//    //проверим сходство рассчЄтных значений и значений полученных  в классе
//
//    for (const auto& [id, rel] : document_to_relevance) {
//        for (const Document& doc : documents) {
//            if (doc.id == id) {
//                ASSERT((abs(rel - doc.relevance) < 1e-6));
//            }
//        }
//    }
//}
//
//#define RUN_TEST(func) func(); 
//
//// ‘ункци€ TestSearchServer €вл€етс€ точкой входа дл€ запуска тестов
//void TestSearchServer() {
//    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
//    RUN_TEST(TestAddDocument);
//    RUN_TEST(TestMinusWords);
//    RUN_TEST(TestMatchedDocuments);
//    RUN_TEST(RelevanceRest);
//    RUN_TEST(RaitingTest);
//    RUN_TEST(UserStatusTest);
//    RUN_TEST(UserPredTest);
//    RUN_TEST(RelevanceTest);
//}
//
//int main() {
//
//    TestSearchServer();
//
//    cout << "Search server testing finished"s << endl;
//
//    mt19937 generator;
//
//    const auto dictionary = GenerateDictionary(generator, 1000, 10);
//    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
//
//    const string query = GenerateQuery(generator, dictionary, 500, 0.1);
//
//    SearchServer search_server(dictionary[0]);
//    for (size_t i = 0; i < documents.size(); ++i) {
//        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
//    }
//
//    TEST(seq);
//    TEST(par);
//}

//#include "process_queries.h"
//#include "search_server.h"
//#include <execution>
//#include <iostream>
//#include <string>
//#include <vector>
//using namespace std;
//void PrintDocument(const Document& document) {
//    cout << "{ "s
//        << "document_id = "s << document.id << ", "s
//        << "relevance = "s << document.relevance << ", "s
//        << "rating = "s << document.rating << " }"s << endl;
//}
//int main() {
//    SearchServer search_server("and with"s);
//    int id = 0;
//    for (
//        const string& text : {
//            "white cat and yellow hat"s,
//            "curly cat curly tail"s,
//            "nasty dog with big eyes"s,
//            "nasty pigeon john"s,
//        }
//        ) {
//        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//    }
//    cout << "ACTUAL by default:"s << endl;
//    // последовательна€ верси€
//    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
//        PrintDocument(document);
//    }
//    cout << "BANNED:"s << endl;
//    // последовательна€ верси€
//    for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
//        PrintDocument(document);
//    }
//    cout << "Even ids:"s << endl;
//    // параллельна€ верси€
//    for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//        PrintDocument(document);
//    }
//    return 0;
//}

#include "search_server.h"
#include "log_duration.h"
#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;
string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution(0, 26)(generator) + 'a');
    }
    return word;
}
vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}
string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}
vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}
template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(string(mark));
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}
#define TEST(policy) Test(#policy, search_server, queries, execution::policy)
int main() {
    {
        mt19937 generator;
        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
        }
        const auto queries = GenerateQueries(generator, dictionary, 100, 70);
        LogDuration sdf("seq");
        TEST(seq);
    }

    {
        mt19937 generator;
        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
        }
        const auto queries = GenerateQueries(generator, dictionary, 100, 70);
        LogDuration par("par");
        TEST(par);
    }
}