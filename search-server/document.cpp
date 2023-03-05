#include "document.h"
using namespace std;

Document::Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating) {
}

std::ostream& operator<<(std::ostream& output, const Document& doc)
{
    return output << "document_id = "s << doc.id
        << ", relevance = "s << doc.relevance
        << ", rating = "s << doc.rating;
}