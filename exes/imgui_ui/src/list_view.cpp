#include "list_view.hpp"

#include "detailed_view.hpp"
#include "global_trace.hpp"
#include "tree_view.hpp"

#include <imgui.h>

#include <array>

namespace ui {
namespace {
constexpr auto theDefaultString = "N/A";
constexpr auto theDefaultTreeDistance = 5;
} // namespace

ListView::ListView(
	std::shared_ptr<ContextAdapter> aContext,
	std::vector<Person> aSearchResults,
	std::shared_ptr<View> aClearView)
	: myContext(std::move(aContext))
	, myClearView(std::move(aClearView)) {
	for (auto& person : aSearchResults) {
		mySearchResults.emplace_back(myContext, std::move(person));
	}
}

std::shared_ptr<View>
ListView::Print(WindowFactory aWindowFactory) {
	for (auto& person : mySearchResults) {
		ImGui::PushID(&person);
		if (ImGui::Button("Select")) {
			ImGui::PopID();
			return std::make_shared<TreeView>(
				myContext,
				person.GetId(),
				theDefaultTreeDistance,
				shared_from_this());
		}
		ImGui::SameLine();
		person.Print(aWindowFactory);
		ImGui::PopID();
	}
	if (ImGui::Button("Clear")) {
		theGlobalTrace->AddEvent("Clear List");
		return myClearView;
	}
	return nullptr;
}

ListView::PersonPrinter::PersonPrinter(std::shared_ptr<ContextAdapter> aContext, Person aPerson)
	: myContext(std::move(aContext))
	, myPerson(std::move(aPerson)) {}

void
ListView::PersonPrinter::Print(WindowFactory aWindowFactory) {
	auto startCurserPos = ImGui::GetCursorPos();
	if (ImGui::Selectable(
			"",
			false,
			ImGuiSelectableFlags_None,
			{ImGui::GetColumnWidth(), ImGui::GetTextLineHeightWithSpacing() * 2})) {
		theGlobalTrace->AddEvent(
			"Open Details of " + myPerson.firstNames[0] + " " + myPerson.lastNames[0]);
		aWindowFactory(std::make_shared<DetaildView>(myContext, myPerson));
	}
	auto endCurserPos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(startCurserPos);

	ImGui::TextUnformatted(
		myPerson.firstNames.empty() ? theDefaultString : myPerson.firstNames[0].c_str());
	ImGui::SameLine();
	ImGui::TextUnformatted(
		myPerson.lastNames.empty() ? theDefaultString : myPerson.lastNames[0].c_str());

	ImGui::TextUnformatted("Birth:");
	ImGui::SameLine();
	ImGui::TextUnformatted(myPerson.dateOfBirth.value_or(theDefaultString).c_str());
	ImGui::SameLine();
	ImGui::Spacing();
	ImGui::SameLine();
	ImGui::TextUnformatted("Death:");
	ImGui::SameLine();
	ImGui::TextUnformatted(myPerson.dateOfDeath.value_or(theDefaultString).c_str());

	ImGui::SetCursorPos(endCurserPos);
}

PersonId
ListView::PersonPrinter::GetId() const {
	return myPerson.id;
}
} // namespace ui
