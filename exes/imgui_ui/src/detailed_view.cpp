#include "detailed_view.hpp"

#include <imgui.h>

#include <string>

namespace ui {
namespace {
constexpr auto theDefaultString = "N/A";
}

DetaildView::DetaildView(std::shared_ptr<ContextAdapter> aContext, Person aPerson)
	: myContext(std::move(aContext))
	, myPerson(std::move(aPerson)) {}

std::shared_ptr<View>
DetaildView::Print(WindowFactory aWindowFactory) {
	ImGui::PushID(&myPerson);
	if (ImGui::Button("Show Images")) {
		myContext->ShowImagesOfPerson(myPerson.id);
	}
	ImGui::SameLine();
	if (ImGui::Button("Play Audio")) {
		myContext->PlayPerson(myPerson.id);
	}

	PrivShowName();

	ImGui::TextUnformatted("Gender");
	ImGui::SameLine();
	ImGui::TextUnformatted(myPerson.gender.value_or(theDefaultString).c_str());

	PrivShowDates();
	PrivShowProfessions();
	PrivShowResidence();
	PrivShowRemarks();
	ImGui::PopID();

	return nullptr;
}

void
DetaildView::PrivShowName() {
	if (myPerson.title.has_value()) {
		ImGui::TextUnformatted(myPerson.title.value().c_str());
		ImGui::SameLine();
	}
	for (const auto& firstName : myPerson.firstNames) {
		ImGui::TextUnformatted(firstName.c_str());
		ImGui::SameLine();
	}
	if (myPerson.titleOfNobility.has_value()) {
		ImGui::TextUnformatted(myPerson.titleOfNobility.value().c_str());
		ImGui::SameLine();
	}
	for (const auto& lastName : myPerson.lastNames) {
		ImGui::TextUnformatted(lastName.c_str());
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

void
DetaildView::PrivShowDates() {
	if (myPerson.dateOfBirth.has_value()) {
		ImGui::TextUnformatted("Birth");
		ImGui::SameLine();
		ImGui::TextUnformatted(myPerson.dateOfBirth.value().c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("at");
		ImGui::SameLine();
		ImGui::TextUnformatted(myPerson.placeOfBirth.value_or(theDefaultString).c_str());
	}

	if (myPerson.dateOfDeath.has_value()) {
		ImGui::TextUnformatted("Death");
		ImGui::SameLine();
		ImGui::TextUnformatted(myPerson.dateOfDeath.value().c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("at");
		ImGui::SameLine();
		ImGui::TextUnformatted(myPerson.placeOfDeath.value_or(theDefaultString).c_str());
	}
}

void
DetaildView::PrivShowProfessions() {
	if (myPerson.professions.empty()) {
		return;
	}

	ImGui::TextUnformatted("Professions:");
	for (const auto& provession : myPerson.professions) {
		ImGui::TextUnformatted(" -");
		ImGui::SameLine();
		ImGui::TextUnformatted(provession.c_str());
	}
}

void
DetaildView::PrivShowResidence() {
	if (myPerson.placeOfResidences.empty()) {
		return;
	}

	ImGui::TextUnformatted("Residence of:");
	for (const auto& recidence : myPerson.placeOfResidences) {
		ImGui::TextUnformatted(" -");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.name.value_or(theDefaultString).c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("from");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.startDate.value_or(theDefaultString).c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted("to");
		ImGui::SameLine();
		ImGui::TextUnformatted(recidence.endDate.value_or(theDefaultString).c_str());
	}
}

void
DetaildView::PrivShowRemarks() {
	if (myPerson.remark.has_value()) {
		ImGui::NewLine();
		ImGui::TextUnformatted("Remarks:");
		ImGui::TextUnformatted(myPerson.remark.value().c_str());
	}
}
} // namespace ui
