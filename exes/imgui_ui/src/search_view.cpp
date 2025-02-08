#include "search_view.hpp"

#include "list_view.hpp"

#include <imgui.h>

#include <array>
#include <iostream>

namespace ui {
constexpr auto theLocDefaultString = "N/A";

#ifndef _WIN32
#include <filesystem>

std::shared_ptr<View>
View::CreateStartView() {
	return std::make_shared<SearchView>(ContextAdapter::Create(std::filesystem::current_path()));
}
#endif

SearchView::SearchView(std::shared_ptr<ContextAdapter> aContext)
	: myContext(std::move(aContext)) {}

std::shared_ptr<View>
SearchView::Print(WindowFactory aWindowFactory) {
	PrivShowFilters();
	if (ImGui::Button("Search")) {
		auto searchResult = PrivDoSearch();
		return std::make_shared<ListView>(myContext, std::move(searchResult), shared_from_this());
	}
	return nullptr;
}

void
SearchView::PrivShowFilters() {
	if (myFirstNameFilters.empty()) {
		myFirstNameFilters.emplace_back();
	}
	if (myLastNameFilters.empty()) {
		myLastNameFilters.emplace_back();
	}

	ImGui::InputText("Title", myTitleFilter.data(), myTitleFilter.size());
	for (auto& field : myFirstNameFilters) {
		ImGui::PushID(&field);
		ImGui::InputText("First Name", field.data(), field.size());
		ImGui::PopID();
	}
	if (ImGui::Button("+##FirstName")) {
		myFirstNameFilters.emplace_back();
	}
	ImGui::InputText(
		"Title of Nobility",
		myTitleOfNobilityFilter.data(),
		myTitleOfNobilityFilter.size());
	for (auto& field : myLastNameFilters) {
		ImGui::PushID(&field);
		ImGui::InputText("Last Name", field.data(), field.size());
		ImGui::PopID();
	}
	if (ImGui::Button("+##LastName")) {
		myLastNameFilters.emplace_back();
	}
}

std::vector<Person>
SearchView::PrivDoSearch() {
	Person prototype{};
	prototype.title = myTitleFilter.data();
	for (const auto& name : myFirstNameFilters) {
		prototype.firstNames.emplace_back(name.data());
	}
	prototype.titleOfNobility = myTitleOfNobilityFilter.data();
	for (const auto& name : myLastNameFilters) {
		prototype.lastNames.emplace_back(name.data());
	}
	return myContext->GetPersonsMatchingPattern(std::move(prototype), 1);
}
} // namespace ui
