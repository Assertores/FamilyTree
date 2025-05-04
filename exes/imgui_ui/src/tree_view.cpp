#include "tree_view.hpp"

#include <imgui_adapter/imgui_adapter.hpp>

#include <set>

namespace ui {
constexpr auto theBoxHight = 80;
constexpr auto theBoxWidth = 120;
constexpr auto theBoxVerticalSpacing = 40;
constexpr auto theBoxHorizontalSpacing = 5;
constexpr auto theBoxHorizontalCenter = theBoxWidth / 2;
constexpr auto theBoxVerticalCenter = theBoxHight / 2;
constexpr auto theFamilySpacing = 4;
constexpr auto thePortOffset = 2;

TreeView::TreeView(
	std::shared_ptr<ContextAdapter> aContext,
	PersonId aPerson,
	size_t aDistance,
	std::shared_ptr<View> aBackView)
	: myContext(std::move(aContext))
	, myPerson(aPerson)
	, myDistance(aDistance)
	, myBackView(std::move(aBackView)) {}

void
TreeView::Enter() {
	myContext->BuildGraph(myPerson, myDistance, *this);
	PrivCalculateFamiltyMetadata();
}

std::shared_ptr<View>
TreeView::Print(WindowFactory aWindowFactory) {
	if (ImGui::Button("Back")) {
		return myBackView;
	}

	// BEGINNOLINT
	constexpr auto second = 0xFFFFFFFF;
	ImGui::BeginChild(
		"tree",
		{},
		ImGuiChildFlags_None,
		// NOLINTNEXTLINE
		ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	ImVec2 windowPos = ImGui::GetWindowPos();
	auto* canvas = ImGui::GetWindowDrawList();
	const auto scrollXOffset = ImGui::GetScrollX();
	const auto scrollYOffset = ImGui::GetScrollY();
	for (const auto& family : myFamilies) {
		canvas->AddLine(
			{family.myLeft + windowPos.x + theBoxHorizontalCenter - scrollXOffset,
			 family.myHorizontalLine + windowPos.y - scrollYOffset},
			{family.myRight + windowPos.x + theBoxHorizontalCenter - scrollXOffset,
			 family.myHorizontalLine + windowPos.y - scrollYOffset},
			second);
		for (const auto& personId : family.myParents) {
			auto target = myPersons.at(personId);
			canvas->AddLine(
				{target.myX + windowPos.x + theBoxHorizontalCenter + thePortOffset - scrollXOffset,
				 target.myY + windowPos.y + theBoxVerticalCenter - scrollYOffset},
				{target.myX + windowPos.x + theBoxHorizontalCenter + thePortOffset - scrollXOffset,
				 family.myHorizontalLine + windowPos.y - scrollYOffset},
				second);
		}
		for (const auto& personId : family.myChildrens) {
			auto target = myPersons.at(personId);
			canvas->AddLine(
				ImVec2{
					target.myX + windowPos.x + theBoxHorizontalCenter - thePortOffset
						- scrollXOffset,
					target.myY + windowPos.y + theBoxVerticalCenter - scrollYOffset},
				ImVec2{
					target.myX + windowPos.x + theBoxHorizontalCenter - thePortOffset
						- scrollXOffset,
					family.myHorizontalLine + windowPos.y - scrollYOffset},
				second);
		}
	}
	constexpr ImVec4 boxColor = {0.2, 0.2, 0.2, 1};
	ImGui::PushStyleColor(ImGuiCol_ChildBg, boxColor);
	for (const auto& [_, person] : myPersons) {
		ImGui::SetCursorPos({person.myX, person.myY});
		ImGui::PushID(&person);
		ImGui::BeginChild("", {theBoxWidth, theBoxHight});
		ImGui::TextUnformatted(person.myPersonData.firstNames[0].c_str());
		ImGui::SameLine();
		ImGui::TextUnformatted(person.myPersonData.lastNames[0].c_str());
		ImGui::EndChild();
		ImGui::PopID();
	}
	ImGui::PopStyleColor();
	ImGui::EndChild();

	// ENDNOLINT
	return nullptr;
}

void
TreeView::PersonInGenerationStrategy(PersonId aId, int aGeneration) {
	if (myPersons.find(aId) != myPersons.end()) {
		// Person Already Exists
		return;
	}
	LocPerson person{};
	person.myPersonData = myContext->GetPerson(aId);
	person.myY = static_cast<float>(aGeneration * (theBoxHight + theBoxVerticalSpacing));
	for (const auto& [_, it] : myPersons) {
		if (it.myY != person.myY) {
			continue;
		}
		if (it.myX >= person.myX) {
			person.myX = it.myX + (theBoxWidth + theBoxHorizontalSpacing);
		}
	}
	myPersons.emplace(aId, person);
}

void
TreeView::FamilieStrategy(std::vector<PersonId> aParents, std::vector<PersonId> aChilds) {
	for (const auto& personId : aParents) {
		if (myPersons.find(personId) == myPersons.end()) {
			return;
		}
	}
	for (const auto& personId : aChilds) {
		if (myPersons.find(personId) == myPersons.end()) {
			return;
		}
	}

	Family newFamily(aParents, aChilds);
	for (const auto& family : myFamilies) {
		if (family == newFamily) {
			return;
		}
	}

	myFamilies.emplace_back(std::move(newFamily));
}

void
TreeView::PrivCalculateFamiltyMetadata() {
	std::set<float> takenHights;

	for (auto& family : myFamilies) {
		family.myRight = -std::numeric_limits<float>::min();
		family.myLeft = std::numeric_limits<float>::max();
		family.myHorizontalLine = -std::numeric_limits<float>::max();

		for (const auto& personId : family.myParents) {
			const auto& person = myPersons.at(personId);
			family.myLeft = std::min(family.myLeft, person.myX + thePortOffset);
			family.myRight = std::max(family.myRight, person.myX + thePortOffset);
			family.myHorizontalLine = std::max(family.myHorizontalLine, person.myY);
		}
		for (const auto& personId : family.myChildrens) {
			const auto& person = myPersons.at(personId);
			family.myLeft = std::min(family.myLeft, person.myX - thePortOffset);
			family.myRight = std::max(family.myRight, person.myX - thePortOffset);
		}
		// NOLINTNEXTLINE
		family.myHorizontalLine += (float)theBoxHight + theFamilySpacing;
		while (takenHights.find(family.myHorizontalLine) != takenHights.end()) {
			family.myHorizontalLine += theFamilySpacing;
		}
		takenHights.insert(family.myHorizontalLine);
	}
}

bool
TreeView::Family::operator==(const Family& aOther) const {
	if (myParents != aOther.myParents) {
		return false;
	}
	if (myChildrens != aOther.myChildrens) {
		return false;
	}
	return true;
}
} // namespace ui
