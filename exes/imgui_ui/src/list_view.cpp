#include "list_view.hpp"

#include <imgui.h>

#include <array>
#include <iostream>

namespace ui {
constexpr auto theLocDefaultString = "N/A";

ListView::ListView(
	std::shared_ptr<ContextAdapter> aContext,
	std::vector<Person> aSearchResults,
	std::shared_ptr<View> aClearView)
	: myContext(std::move(aContext))
	, mySearchResults(std::move(aSearchResults))
	, myClearView(std::move(aClearView)) {}

std::shared_ptr<View>
ListView::Print() {
	for (const auto& person : mySearchResults) {
		PrivShowPerson(person);
	}
	if (ImGui::Button("Clear")) {
		return myClearView;
	}
	return nullptr;
}

void
ListView::PrivShowPerson(const Person& aPerson) {
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
ListView::PrivShowName(const Person& aPerson) {
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
ListView::PrivShowDates(const Person& aPerson) {
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
ListView::PrivShowProfessions(const Person& aPerson) {
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
ListView::PrivShowResidence(const Person& aPerson) {
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
ListView::PrivShowRemarks(const Person& aPerson) {
	if (aPerson.remark.has_value()) {
		ImGui::NewLine();
		ImGui::TextUnformatted("Remarks:");
		ImGui::TextUnformatted(aPerson.remark.value().c_str());
	}
}
} // namespace ui
