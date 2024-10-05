#include "view.hpp"

#include <imgui.h>

#include <array>
#include <iostream>

// clang-format off
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
// clang-format on

void
View::Print() {
	if (myContext == nullptr) {
		if (ImGui::Button("Load Data")) {
			auto folder = PrivGetFolder();
			if (!folder.empty()) {
				myContext = CreateWithCSVAndJSON(folder.c_str(), nullptr);
			}
		}
		return;
	}

	ImGui::BeginTabBar("Menu");
	if (ImGui::BeginTabItem("Search")) {
		if (mySearchResults.empty()) {
			PrivShowFilters();
			if (ImGui::Button("Search")) {
				PrivDoSearch();
			}
		} else {
			for (const auto& person : mySearchResults) {
				PrivShowPerson(person);
			}
			if (ImGui::Button("Clear")) {
				mySearchResults.clear();
			}
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Graph")) {
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}

void
View::Shutdown() {
	if (myContext != nullptr) {
		Free(myContext, nullptr);
	}
	myContext = nullptr;
}

std::string
View::PrivGetFolder() {
	BROWSEINFOW setting{};
	setting.ulFlags = BIF_RETURNONLYFSDIRS;
	auto* result = SHBrowseForFolderW(&setting);
	if (result == nullptr) {
		return {};
	}
	// std::array<wchar_t, MAX_PATH> szDirectory{};
	std::wstring szDirectory{};
	szDirectory.resize(MAX_PATH);
	SHGetPathFromIDListW(result, szDirectory.data());

	int nameLengthAsUtf8Char = WideCharToMultiByte(
		CP_UTF8,
		0,
		szDirectory.c_str(),
		static_cast<int>(szDirectory.size()),
		nullptr,
		0,
		nullptr,
		nullptr);
	if (nameLengthAsUtf8Char == 0) {
		return {};
	}
	std::string convertedName{};
	convertedName.resize(nameLengthAsUtf8Char);
	WideCharToMultiByte(
		CP_UTF8,
		0,
		szDirectory.c_str(),
		static_cast<int>(szDirectory.size()),
		convertedName.data(),
		nameLengthAsUtf8Char,
		nullptr,
		nullptr);

	return convertedName;
}

void
View::PrivShowFilters() {
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
View::PrivDoSearch() {
	Person prototype{};
	prototype.title = myTitleFilter.data();
	std::vector<const char*> firstNames{};
	firstNames.reserve(myFirstNameFilters.size());
	for (const auto& name : myFirstNameFilters) {
		firstNames.emplace_back(name.data());
	}
	prototype.firstNameCount = firstNames.size();
	prototype.firstNames = firstNames.data();
	prototype.titleOfNobility = myTitleOfNobilityFilter.data();
	std::vector<const char*> lastNames{};
	lastNames.reserve(myLastNameFilters.size());
	for (const auto& name : myLastNameFilters) {
		lastNames.emplace_back(name.data());
	}
	prototype.lastNameCount = lastNames.size();
	prototype.lastNames = lastNames.data();
	size_t size = 0;
	auto* persons = GetPersonsMatchingPattern(myContext, prototype, 1, &size, nullptr);
	for (int i = 0; i < size; i++) {
		mySearchResults.emplace_back(persons[i]);
	}

	myTitleFilter = {};
	myFirstNameFilters.clear();
	myTitleOfNobilityFilter = {};
	myLastNameFilters.clear();
}

void
View::PrivShowPerson(const Person& aPerson) {
	ImGui::PushID(&aPerson);
	const auto personId = std::to_string(aPerson.id);

	if (ImGui::CollapsingHeader(
			(personId + ' ' + aPerson.firstNames[0] + ' ' + aPerson.lastNames[0]).c_str())) {
		ImGui::TextUnformatted(personId.c_str());
		ImGui::SameLine();
		if (IsDefaultString(myContext, aPerson.title) == 0) {
			ImGui::TextUnformatted(aPerson.title);
			ImGui::SameLine();
		}
		for (int i = 0; i < aPerson.firstNameCount; i++) {
			ImGui::TextUnformatted(aPerson.firstNames[i]);
			ImGui::SameLine();
		}
		if (IsDefaultString(myContext, aPerson.titleOfNobility) == 0) {
			ImGui::TextUnformatted(aPerson.titleOfNobility);
			ImGui::SameLine();
		}
		for (int i = 0; i < aPerson.lastNameCount; i++) {
			ImGui::TextUnformatted(aPerson.lastNames[i]);
			ImGui::SameLine();
		}
		ImGui::NewLine();

		if (IsDefaultString(myContext, aPerson.dateOfBirth) == 0) {
			ImGui::TextUnformatted("Birth");
			ImGui::SameLine();
			ImGui::TextUnformatted(aPerson.dateOfBirth);
			ImGui::SameLine();
			ImGui::TextUnformatted("at");
			ImGui::SameLine();
			ImGui::TextUnformatted(aPerson.placeOfBirth);
		}

		if (IsDefaultString(myContext, aPerson.dateOfDeath) == 0) {
			ImGui::TextUnformatted("Death");
			ImGui::SameLine();
			ImGui::TextUnformatted(aPerson.dateOfDeath);
			ImGui::SameLine();
			ImGui::TextUnformatted("at");
			ImGui::SameLine();
			ImGui::TextUnformatted(aPerson.placeOfDeath);
		}

		if (IsDefaultString(myContext, aPerson.remark) == 0) {
			ImGui::NewLine();
			ImGui::TextUnformatted("Remarks:");
			ImGui::TextUnformatted(aPerson.remark);
		}
	}
	ImGui::PopID();
}