#include <imgui_adapter.hpp>
#include <windows.h>

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static constexpr auto theWindowWidth = 1280;
static constexpr auto theWindowHight = 720;
static constexpr auto theIndentWidth = 10;

ImGuiWindowFlags theWindowFlags =
	// NOLINTNEXTLINE
	ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar;

static constexpr auto theInputfieldSize = 256;
static constexpr auto theRemarkfieldSize = theInputfieldSize * 5;

struct Residence {
	std::array<char, theInputfieldSize> myName{};
	std::array<char, theInputfieldSize> myStartDate{};
	std::array<char, theInputfieldSize> myEndDate{};
};

uint64_t myId{};
std::array<char, theInputfieldSize> myTitleFilter{};
std::vector<std::array<char, theInputfieldSize>> myFirstNameFilters;
std::array<char, theInputfieldSize> myTitleOfNobilityFilter{};
std::vector<std::array<char, theInputfieldSize>> myLastNameFilters;
std::array<char, theInputfieldSize> myGender{};
std::array<char, theInputfieldSize> myDateOfBirth{};
std::array<char, theInputfieldSize> myPlaceOfBirth{};
std::array<char, theInputfieldSize> myDateOfDeath{};
std::array<char, theInputfieldSize> myPlaceOfDeath{};
std::vector<Residence> myPlaceOfResidences;
std::vector<std::array<char, theInputfieldSize>> myProfessions;
std::array<char, theInputfieldSize> myAudio{};
std::vector<std::array<char, theInputfieldSize>> myImages;
std::array<char, theRemarkfieldSize> myRemarks{};

void
Clear() {
	if (ImGui::Button("Clear")) {
		myId = {};
		myTitleFilter = {};
		myFirstNameFilters = {};
		myTitleOfNobilityFilter = {};
		myLastNameFilters = {};
		myGender = {};
		myDateOfBirth = {};
		myPlaceOfBirth = {};
		myDateOfDeath = {};
		myPlaceOfDeath = {};
		myPlaceOfResidences = {};
		myProfessions = {};
		myAudio = {};
		myImages = {};
		myRemarks = {};
	}
	if (myFirstNameFilters.empty()) {
		myFirstNameFilters.emplace_back();
	}
	if (myLastNameFilters.empty()) {
		myLastNameFilters.emplace_back();
	}
	if (myPlaceOfResidences.empty()) {
		myPlaceOfResidences.emplace_back();
	}
	if (myProfessions.empty()) {
		myProfessions.emplace_back();
	}
	if (myImages.empty()) {
		myImages.emplace_back();
	}
}

void
UI() {
	ImGui::InputScalar("Id", ImGuiDataType_U64, &myId);
	ImGui::Separator();
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
	ImGui::InputText("Gender", myGender.data(), myGender.size());
	ImGui::InputText("Date of birth", myDateOfBirth.data(), myDateOfBirth.size());
	ImGui::InputText("Place of birth", myPlaceOfBirth.data(), myPlaceOfBirth.size());
	ImGui::InputText("Date of death", myDateOfDeath.data(), myDateOfDeath.size());
	ImGui::InputText("Place of death", myPlaceOfDeath.data(), myPlaceOfDeath.size());
	ImGui::TextUnformatted("Places of Residence:");
	ImGui::Indent(theIndentWidth);
	for (auto& residence : myPlaceOfResidences) {
		ImGui::PushID(&residence);
		ImGui::InputText("Name", residence.myName.data(), residence.myName.size());
		ImGui::InputText("Start date", residence.myStartDate.data(), residence.myStartDate.size());
		ImGui::InputText("End date", residence.myEndDate.data(), residence.myEndDate.size());
		ImGui::PopID();
	}
	if (ImGui::Button("+##PlaceOfResidence")) {
		myPlaceOfResidences.emplace_back();
	}
	ImGui::Unindent(theIndentWidth);

	for (auto& field : myProfessions) {
		ImGui::PushID(&field);
		ImGui::InputText("Profession", field.data(), field.size());
		ImGui::PopID();
	}
	if (ImGui::Button("+##Profession")) {
		myProfessions.emplace_back();
	}
	ImGui::InputText("Audio", myAudio.data(), myAudio.size());
	for (auto& field : myImages) {
		ImGui::PushID(&field);
		ImGui::InputText("Image", field.data(), field.size());
		ImGui::PopID();
	}
	if (ImGui::Button("+##Image")) {
		myImages.emplace_back();
	}
	ImGui::InputTextMultiline("Remarks", myRemarks.data(), myRemarks.size());
}

void
Save() { // NOLINT(readability-function-cognitive-complexity)
	if (!ImGui::Button("Save")) {
		return;
	}

	std::string builder;
	std::string content = "{\n\t\"Version\": \"3B5589D2-D9AF-40A8-BC40-574DAB6FFC57\",\n";
	content += "\t\"Id\": " + std::to_string(myId) + ",\n";

	if (myTitleFilter[0] != '\0') {
		content += "\t\"Title\": \"";
		content += myTitleFilter.data();
		content += "\",\n";
	}

	builder.clear();
	for (const auto element : myFirstNameFilters) {
		if (element[0] != '\0') {
			builder += "\t\t\"";
			builder += element.data();
			builder += "\",\n";
		}
	}
	if (!builder.empty()) {
		builder.resize(builder.size() - 2);
		content += "\t\"FirstNames\": [\n" + builder + "\n\t],\n";
	}

	if (myTitleOfNobilityFilter[0] != '\0') {
		content += "\t\"TitleOfNobility\": \"";
		content += myTitleOfNobilityFilter.data();
		content += "\",\n";
	}

	builder.clear();
	for (const auto element : myLastNameFilters) {
		if (element[0] != '\0') {
			builder += "\t\t\"";
			builder += element.data();
			builder += "\",\n";
		}
	}
	if (!builder.empty()) {
		builder.resize(builder.size() - 2);
		content += "\t\"LastNames\": [\n" + builder + "\n\t],\n";
	}

	if (myGender[0] != '\0') {
		content += "\t\"Gender\": \"";
		content += myGender.data();
		content += "\",\n";
	}

	if (myDateOfBirth[0] != '\0') {
		content += "\t\"DateOfBirth\": \"";
		content += myDateOfBirth.data();
		content += "\",\n";
	}

	if (myPlaceOfBirth[0] != '\0') {
		content += "\t\"PlaceOfBirth\": \"";
		content += myPlaceOfBirth.data();
		content += "\",\n";
	}

	if (myDateOfDeath[0] != '\0') {
		content += "\t\"DateOfDeath\": \"";
		content += myDateOfDeath.data();
		content += "\",\n";
	}

	if (myPlaceOfDeath[0] != '\0') {
		content += "\t\"PlaceOfDeath\": \"";
		content += myPlaceOfDeath.data();
		content += "\",\n";
	}

	builder.clear();
	for (const auto element : myProfessions) {
		if (element[0] != '\0') {
			builder += "\t\t\"";
			builder += element.data();
			builder += "\",\n";
		}
	}
	if (!builder.empty()) {
		builder.resize(builder.size() - 2);
		content += "\t\"Profession\": [\n" + builder + "\n\t],\n";
	}

	builder.clear();
	for (const auto element : myPlaceOfResidences) {
		if (element.myName[0] == '\0' && element.myStartDate[0] == '\0'
			&& element.myEndDate[0] == '\0') {
			continue;
		}
		builder += "\t\t{\n";
		if (element.myName[0] != '\0') {
			builder += "\t\t\t\"Name\": \"";
			builder += element.myName.data();
			builder += "\",\n";
		}
		if (element.myStartDate[0] != '\0') {
			builder += "\t\t\t\"StartDate\": \"";
			builder += element.myStartDate.data();
			builder += "\",\n";
		}
		if (element.myEndDate[0] != '\0') {
			builder += "\t\t\t\"EndDate\": \"";
			builder += element.myEndDate.data();
			builder += "\",\n";
		}
		builder.resize(builder.size() - 2);
		builder += "\n\t\t},\n";
	}
	if (!builder.empty()) {
		builder.resize(builder.size() - 2);
		content += "\t\"PlaceOfResidence\": [\n" + builder + "\n\t],\n";
	}

	if (myAudio[0] != '\0') {
		content += "\t\"Audio\": \"";
		content += myAudio.data();
		content += "\",\n";
	}

	builder.clear();
	for (const auto element : myImages) {
		if (element[0] != '\0') {
			builder += "\t\t\"";
			builder += element.data();
			builder += "\",\n";
		}
	}
	if (!builder.empty()) {
		builder.resize(builder.size() - 2);
		content += "\t\"Images\": [\n" + builder + "\n\t],\n";
	}

	std::string remarks = myRemarks.data();
	std::string::size_type pos = 0U;
	while ((pos = remarks.find('\n', pos)) != std::string::npos) {
		remarks.replace(pos, 1, "\\n");
		pos += 2;
	}
	if (!remarks.empty()) {
		content += "\t\"Remarks\": \"" + remarks + "\",\n";
	}

	content.resize(content.size() - 2);
	content += "\n}";

	std::ofstream file("data.json");
	file << content;
	file.close();
}

int
main() { // NOLINT(bugprone-exception-escape)
	auto window = ImGuiAdapter::CreateWindows();

	for (window->Init(theWindowWidth, theWindowHight, "Family Tree UI", {}); window->Update();
		 window->Show()) {
		ImGui::SetNextWindowPos({});
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("MainWindow", nullptr, theWindowFlags);

		Clear();
		UI();
		Save();

		ImGui::End();
	}
	window->Shutdown();
	return 0;
}

// NOTE: this is here sothat no extra console window is opened.
int APIENTRY
WinMain(HINSTANCE /*unused*/, HINSTANCE /*unused*/, PSTR /*unused*/, int /*unused*/) {
	return main();
}