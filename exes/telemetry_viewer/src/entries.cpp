#include "entries.hpp"

#include <imgui.h>

#include <array>
#include <fstream>
#include <iostream>

namespace ui {
static constexpr auto theIndentWidth = 10;

class Event : public Entry {
public:
	explicit Event(std::string aText)
		: myText(std::move(aText)) {}

	void Show() const override { ImGui::TextUnformatted(myText.c_str()); }

private:
	std::string myText;
};

class Success : public Entry {
public:
	void Show() const override { ImGui::TextUnformatted("SUCCESS"); }
};

class Failed : public Entry {
public:
	explicit Failed(std::string aText)
		: myText(std::move(aText)) {}

	void Show() const override { ImGui::TextUnformatted(("FAILED: " + myText).c_str()); }

private:
	std::string myText;
};

Trace::Trace(std::filesystem::path aPath)
	: myName(aPath.filename().u8string()) {
	std::ifstream file(aPath);

	std::string line;
	while (std::getline(file, line)) {
		std::string_view lineView = line;
		std::array<std::string_view, 3> entries;
		auto toSplit = lineView.find_first_of('|');
		if (toSplit == std::string::npos) {
			continue;
		}
		entries[1] = lineView.substr(toSplit + 1);
		entries[0] = lineView.substr(0, toSplit);

		toSplit = entries[1].find_first_of('|');
		if (toSplit == std::string::npos) {
			continue;
		}
		myIsValid = true;

		entries[2] = entries[1].substr(toSplit + 1);
		entries[1] = entries[1].substr(0, toSplit);

		toSplit = entries[2].find_first_of('|');
		if (toSplit != std::string::npos) {
			entries[2] = entries[2].substr(0, toSplit);
		}
		if (entries[1] == " ") {
			myName = std::string(entries[2]);
		} else if (entries[1] == "e") {
			myEntries.emplace_back(std::make_shared<Event>(std::string(entries[2])));
		} else if (entries[1] == "t") {
			myEntries.emplace_back(std::make_shared<Trace>(
				aPath.parent_path() / (std::string(entries[2]) + ".trace")));
		} else if (entries[1] == "S") {
			myEntries.emplace_back(std::make_shared<Success>());
		} else if (entries[1] == "F") {
			myEntries.emplace_back(std::make_shared<Failed>(std::string(entries[2])));
		} else if (entries[1] == "o") {
			// NOTE: ignore pointer to origin.
		} else {
			std::cerr << "Unrecogniced Entry: " << entries[1] << '\n';
		}
	}
}

std::string
Trace::GetName() const {
	return myName;
}

void
Trace::Show() const {
	if (!myIsValid) {
		ImGui::TextUnformatted("!!This Trace Is missing!!");
		return;
	}

	ImGui::PushID(this);
	if (ImGui::CollapsingHeader(myName.c_str())) {
		ImGui::Indent(theIndentWidth);
		for (const auto& entry : myEntries) {
			entry->Show();
		}
		ImGui::Unindent(theIndentWidth);
	}
	ImGui::PopID();
}
} // namespace ui
