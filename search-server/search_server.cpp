#include <cmath>
#include <numeric>
#include "search_server.h"
using namespace std;

SearchServer::SearchServer(const string_view stop_words_text)
	: SearchServer(
		SplitIntoWords(stop_words_text)) 
{
}

SearchServer::SearchServer(const string& stop_words_text)
	: SearchServer(static_cast<string_view>(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings)
{
	if ((document_id < 0) || (documents_.count(document_id) > 0)) {
		throw invalid_argument("Invalid document_id"s);
	}
	auto words = SplitIntoWordsNoStop(document);
	
	for (string_view word : words) {
		string_view_maker_[document_id].push_back(static_cast<string>(word));
	}

	const double inv_word_count = 1.0 / string_view_maker_[document_id].size();
	for (const string& word : string_view_maker_[document_id]) {
		word_to_document_freqs_[static_cast<string_view>(word)][document_id] += inv_word_count;
		word_freqs_in_document_[document_id][static_cast<string_view>(word)] += inv_word_count;
	}

	documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
	document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const
{
	return FindTopDocuments(
		raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
			return document_status == status;
		});
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const
{
	return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const
{
	return documents_.size();
}


tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const
{
	auto query = ParseQuery(raw_query, true);

	if (any_of(query.minus_words.begin(), query.minus_words.end(),
		[&](string_view word) {
			return word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id);
		})) {
		return { vector<string_view>(), documents_.at(document_id).status };
	}

	vector<string_view> matched_words;

	for (string_view word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			matched_words.push_back(word);
		}
	}

	return { matched_words, documents_.at(document_id).status };
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::parallel_policy&, string_view raw_query, int document_id) const
{
	const auto query = ParseQuery(raw_query);

	if (any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
		[&](string_view word) {
			return word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id);
		})) {
		return { vector<string_view>(), documents_.at(document_id).status};
	}

	vector<string_view> matched_words(query.plus_words.size());
	matched_words.reserve(query.plus_words.size());

	auto new_size = copy_if(execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
		[&](string_view word) {
			return (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id));
		});

	matched_words.resize(distance(matched_words.begin(), new_size));
	sort(matched_words.begin(), matched_words.end());
	matched_words.erase(unique(matched_words.begin(), matched_words.end()), matched_words.end());

	return { matched_words, documents_.at(document_id).status };
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::sequenced_policy&, string_view raw_query, int document_id) const
{
	return MatchDocument(raw_query, document_id);
}

std::set<int>::const_iterator SearchServer::begin()
{
	return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end()
{
	return document_ids_.end();
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
	static map<std::string_view, double> word_freqs;
	
	if (word_freqs_in_document_.find(document_id) != word_freqs_in_document_.end()) {
	
		word_freqs = word_freqs_in_document_.at(document_id);	
	}
	return word_freqs;
}

void SearchServer::RemoveDocument(int document_id)
{
	if (document_ids_.find(document_id) != document_ids_.end()) {
		document_ids_.erase(document_id);
		documents_.erase(document_id);
		for (const auto& [word, _] : word_freqs_in_document_.at(document_id)) {
			word_to_document_freqs_.at(word).erase(document_id);
			if (word_to_document_freqs_.at(word).empty()) {
				word_to_document_freqs_.erase(word);
			}
		}
		word_freqs_in_document_.erase(document_id);
	}
}

void SearchServer::RemoveDocument(const execution::parallel_policy&, int document_id)
{
	if (document_ids_.find(document_id) != document_ids_.end()) {
		document_ids_.erase(document_id);
		documents_.erase(document_id);

		vector<string_view> word_to_dell(word_freqs_in_document_.at(document_id).size());
		word_to_dell.reserve(word_freqs_in_document_.at(document_id).size());

		transform(execution::par, word_freqs_in_document_.at(document_id).begin(), word_freqs_in_document_.at(document_id).end(), word_to_dell.begin(),
			[](const auto& word_freq) {return word_freq.first; });

		for_each(execution::par, word_to_dell.begin(), word_to_dell.end(),
			[&](const auto word) {
				word_to_document_freqs_.at(word).erase(document_id);
				if (word_to_document_freqs_.at(word).empty()) {
					word_to_document_freqs_.erase(word);
				}
			});
		
		word_freqs_in_document_.erase(document_id);
	}
}

void SearchServer::RemoveDocument(const execution::sequenced_policy&, int document_id)
{
	if (document_ids_.find(document_id) != document_ids_.end()) {
		document_ids_.erase(document_id);
		documents_.erase(document_id);

		for_each(execution::seq, word_freqs_in_document_.at(document_id).begin(), word_freqs_in_document_.at(document_id).end(),
			[&](const auto& word_freq) {
				word_to_document_freqs_.at(word_freq.first).erase(document_id);
				if (word_to_document_freqs_.at(word_freq.first).empty()) {
					word_to_document_freqs_.erase(word_freq.first);
				}
			});

		word_freqs_in_document_.erase(document_id);
	}
}


bool SearchServer::IsStopWord(string_view word) const
{
	return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(string_view word)
{
	return none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
		});
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const
{
	vector<string_view> words;
	for (string_view word : SplitIntoWordsView(text)) {
		if (!IsValidWord(word)) {
			throw invalid_argument("Word "s + static_cast<string>(word) + " is invalid"s);
		}
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings)
{
	if (ratings.empty()) {
		return 0;
	}

	return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const
{
	if (text.empty()) {
		throw invalid_argument("Query word is empty"s);
	}
	string_view word = text;
	bool is_minus = false;
	if (word[0] == '-') {
		is_minus = true;
		word = word.substr(1);
	}
	if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
		throw invalid_argument("Query word "s + static_cast<string>(text) + " is invalid");
	}

	return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(string_view text, bool sort_result) const
{
	Query result;
	for (string_view word : SplitIntoWordsView(text)) {
		const auto query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				result.minus_words.push_back(query_word.data);
			}
			else {
				result.plus_words.push_back(query_word.data);
			}
		}
	}

	if (sort_result) {
		sort(result.minus_words.begin(), result.minus_words.end());
		result.minus_words.erase(unique(result.minus_words.begin(), result.minus_words.end()), result.minus_words.end());

		sort(result.plus_words.begin(), result.plus_words.end());
		result.plus_words.erase(unique(result.plus_words.begin(), result.plus_words.end()), result.plus_words.end());
	}

	return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const
{
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

