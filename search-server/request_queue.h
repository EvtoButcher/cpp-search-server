#pragma once
#include <deque>
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    size_t GetNoResultRequests() const;

    size_t GetResultRequests() const;

private:
    enum class QueryStatus {
        RESULTS,
        NO_RESULTS,
    };

    struct QueryResult {
        QueryResult(std::string query, size_t res_count);

        std::string query;
        QueryStatus status;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& server;
};

template<typename DocumentPredicate>
inline std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    const auto answer = server.FindTopDocuments(raw_query, document_predicate);

    if (requests_.size() < min_in_day_) {
        requests_.push_back(QueryResult(raw_query, answer.size()));
    }
    else {
        requests_.pop_front();
        requests_.push_back(QueryResult(raw_query, answer.size()));
    }

    return answer;
}