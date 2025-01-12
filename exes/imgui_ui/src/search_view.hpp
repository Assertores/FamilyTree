#pragma once

#include "view.hpp"

#include <context_adapter.hpp>

#include <array>
#include <string>
#include <vector>

namespace ui {

static constexpr auto theInputfieldSize = 256;

class SearchView final
	: public View
	, public std::enable_shared_from_this<SearchView> {
public:
	explicit SearchView(std::shared_ptr<ContextAdapter> aContext);
	std::shared_ptr<View> Print() override;

private:
	std::shared_ptr<ContextAdapter> myContext;
	std::array<char, theInputfieldSize> myTitleFilter{};
	std::vector<std::array<char, theInputfieldSize>> myFirstNameFilters;
	std::array<char, theInputfieldSize> myTitleOfNobilityFilter{};
	std::vector<std::array<char, theInputfieldSize>> myLastNameFilters;

	void PrivShowFilters();
	std::vector<Person> PrivDoSearch();
};
} // namespace ui
