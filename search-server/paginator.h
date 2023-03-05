#pragma once
#include <vector>
#include <string>

template <typename Iterator>
struct IteratorRange
{
    IteratorRange(const Iterator& begin_docs, const Iterator& end_docs)
        : page_begin(begin_docs)
        , page_end(end_docs)
        , page_size(distance(begin_docs, end_docs)) {

    }

    Iterator page_begin;
    Iterator page_end;
    size_t page_size = 0;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, const IteratorRange<Iterator>& docs_range) {

    for (auto iter = docs_range.page_begin; iter < docs_range.page_end; ++iter) {
        output << std::string("{ ") << *iter << std::string(" }");
    }

    return output;
}

template <typename Iterator>
class Paginator
{
public:
    Paginator(const Iterator& begin_docs, const Iterator& end_docs, const size_t num_pages);

    auto begin() const;

    auto end() const;

    size_t size() const;

private:
    std::vector<IteratorRange<Iterator>> pages;
};


template<typename Iterator>
inline Paginator<Iterator>::Paginator(const Iterator& begin_docs, const Iterator& end_docs, const size_t num_pages)
{
    auto begin_doc = begin_docs;
    auto end_doc = end_docs;
    for (; begin_doc < end_doc; advance(begin_doc, num_pages)) {
        if (static_cast<size_t>(distance(begin_doc, end_doc)) >= num_pages) {

            pages.push_back(IteratorRange(begin_doc, next(begin_doc, num_pages)));
        }
        else {
            pages.push_back(IteratorRange(begin_doc, end_doc));
            break;
        }
    }
}

template<typename Iterator>
inline auto Paginator<Iterator>::begin() const
{
    return pages.begin();
}

template<typename Iterator>
inline auto Paginator<Iterator>::end() const
{
    return pages.end();
}

template<typename Iterator>
inline size_t Paginator<Iterator>::size() const
{
    return pages.size();
}