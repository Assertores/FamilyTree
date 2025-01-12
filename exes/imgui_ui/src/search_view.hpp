#pragma once

#include "view.hpp"

#include <context_adapter.hpp>

#include <array>
#include <string>
#include <vector>

namespace ui {

static constexpr auto theInputfieldSize = 256;

class SearchView final : public View {
public:
	explicit SearchView(std::shared_ptr<ContextAdapter> aContext);
	std::shared_ptr<View> Print() override;

private:
	std::shared_ptr<ContextAdapter> myContext;
	std::vector<Person> mySearchResults;
	std::array<char, theInputfieldSize> myTitleFilter{};
	std::vector<std::array<char, theInputfieldSize>> myFirstNameFilters;
	std::array<char, theInputfieldSize> myTitleOfNobilityFilter{};
	std::vector<std::array<char, theInputfieldSize>> myLastNameFilters;

	void PrivShowFilters();
	void PrivDoSearch();
	void PrivShowPerson(const Person& aPerson);

	void PrivShowName(const Person& aPerson);
	void PrivShowDates(const Person& aPerson);
	void PrivShowProfessions(const Person& aPerson);
	void PrivShowResidence(const Person& aPerson);
	void PrivShowRemarks(const Person& aPerson);
};
} // namespace ui
