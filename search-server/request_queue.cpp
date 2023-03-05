#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
    :server(search_server) {
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status)
{
    return  AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query)
{
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

size_t RequestQueue::GetNoResultRequests() const
{
    return count_if(requests_.begin(), requests_.end(), [](QueryResult answer) {
        return answer.status == QueryStatus::NO_RESULTS;
        });
}

size_t RequestQueue::GetResultRequests() const
{
    return count_if(requests_.begin(), requests_.end(), [](QueryResult answer) {
        return answer.status == QueryStatus::RESULTS;
        });
}

RequestQueue::QueryResult::QueryResult(std::string query, size_t res_count)
    : query(query) {
    if (res_count > 0) {
        status = QueryStatus::RESULTS;
    }
    else {
        status = QueryStatus::NO_RESULTS;
    }
}