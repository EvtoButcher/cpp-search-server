#include "process_queries.h"

using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
    vector<vector<Document>> answer(queries.size());
    answer.reserve(queries.size() );

    transform(execution::par, queries.begin(), queries.end(), answer.begin(), [&search_server](const string& query) {return search_server.FindTopDocuments(query); });

    return answer;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    auto doc_list = ProcessQueries(search_server, queries);

    vector<Document> answer;
    answer.reserve(doc_list.size());

    for (auto& doc : doc_list) {
        move(doc.begin(), doc.end(), back_inserter(answer));
    }

    return answer;
}
