#include "search_view.hpp"

#include <imgui.h>

#include <array>
#include <iostream>

namespace ui {
constexpr auto theLocDefaultString = "N/A";

SearchView::SearchView(std::shared_ptr<ContextAdapter> aContext)
	: myContext(std::move(aContext)) {}

std::shared_ptr<View>
SearchView::Print() {
	if (mySearchResults.empty()) {
		PrivShowFilters();
		if (ImGui::Button("Search")) {
			PrivDoSearch();
		}
		return nullptr;
	}

	for (const auto& person : mySearchResults) {
		PrivShowPerson(person);
	}
	if (ImGui::Button("Clear")) {
		mySearchResults.clear();
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

void
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
	mySearchResults = myContext->GetPersonsMatchingPattern(std::move(prototype), 1);

	myTitleFilter = {};
	myFirstNameFilters.clear();
	myTitleOfNobilityFilter = {};
	myLastNameFilters.clear();
}

void
SearchView::PrivShowPerson(const Person& aPerson) {
	ImGui::PushID(&aPerson);

	if (ImGui::CollapsingHeader(
			(std::to_string(aPerson.id) + ' ' + aPerson.firstNames[0] + ' ' + aPerson.lastNames[0])
				.c_str())) {
		if (ImGui::Button("Show Images")) {
			myContext->ShowImagesOfPerson(aPerson.id);
		}
		ImGui::SameLine();
		if (ImGui::Button("Play Audio")) {
			myContext->PlayPerson(aPerson.id);
		}

		PrivShowName(aPerson);

		ImGui::TextUnformatted("Gender");
		ImGui::SameLine();
		ImGui::TextUnformatted(aPerson.gender.value_or(theLocDefaultString).c_str());

		PrivShowDates(aPerson);
		PrivShowProfessions(aPerson);
		PrivShowResidence(aPerson);
		PrivShowRemarks(aPerson);
	}
	ImGui::PopID();
}

void
SearchView::PrivShowName(const Person& aPerson) {
	if (aPerson.title.has_value()) {
		ImGui::TextUnformatted(aPerson.title.value().c_str());
		ImGui::SameLine();
	}
	for (const auto& firstName : aPerson.firstNames) {
		ImGui::TextUnformatted(firstName.c_str());
		ImGui::SameLine();
	}
	if (aPerson.titleOfNobility.has_value()) {
		ImGui::TextUnformatted(aPerson.titleOfNobility.value().c_str());
		ImGui::SameLine();
	}
	for (const auto& lastName : aPerson.lastNames) {
		ImGui::TextUnformatted(lastName.c_str());
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

void
SearchView::PrivShowDates(const Person& aPerson) {
	if (aPerson.dateOfBirth.has_value()) {
		ImGui::TextUnformatted("Birth");
		ImGui::SameLine();
		ImGui::TextUnformatted(aPerson.dateOfBirth.value().c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("at");
		ImGui::SameLine();
		ImGui::TextUnformatted(aPerson.placeOfBirth.value_or(theLocDefaultString).c_str());
	}

	if (aPerson.dateOfDeath.has_value()) {
		ImGui::TextUnformatted("Death");
		ImGui::SameLine();
		ImGui::TextUnformatted(aPerson.dateOfDeath.value().c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("at");
		ImGui::SameLine();
		ImGui::TextUnformatted(aPerson.placeOfDeath.value_or(theLocDefaultString).c_str());
	}
}

void
SearchView::PrivShowProfessions(const Person& aPerson) {
	if (aPerson.professions.empty()) {
		return;
	}

	ImGui::TextUnformatted("Professions:");
	for (const auto& provession : aPerson.professions) {
		ImGui::TextUnformatted(" -");
		ImGui::SameLine();
		ImGui::TextUnformatted(provession.c_str());
	}
}

void
SearchView::PrivShowResidence(const Person& aPerson) {
	if (aPerson.placeOfResidences.empty()) {
		return;
	}

	ImGui::TextUnformatted("Residence of:");
	for (const auto& recidence : aPerson.placeOfResidences) {
		ImGui::TextUnformatted(" -");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.name.value_or(theLocDefaultString).c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("from");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.startDate.value_or(theLocDefaultString).c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("to");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.endDate.value_or(theLocDefaultString).c_str());
	}
}

void
SearchView::PrivShowRemarks(const Person& aPerson) {
	if (aPerson.remark.has_value()) {
		ImGui::NewLine();
		ImGui::TextUnformatted("Remarks:");
		ImGui::TextUnformatted(aPerson.remark.value().c_str());
	}
}
} // namespace ui
