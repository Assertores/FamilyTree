#include "src/setup_view.hpp"

#include <imgui_adapter.hpp>

static constexpr auto theWindowWidth = 1280;
static constexpr auto theWindowHight = 720;

ImGuiWindowFlags theWindowFlags =
	// NOLINTNEXTLINE
	ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar;

void DoUi();

int
main() { // NOLINT(bugprone-exception-escape)
	auto window = ImGuiAdapter::CreateWindows();
	std::shared_ptr<ui::View> view = std::make_shared<ui::SetupView>();
	view->Enter();

	for (window->Init(theWindowWidth, theWindowHight, "Family Tree UI", {}); window->Update();
		 window->Show()) {
		ImGui::SetNextWindowPos({});
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("MainWindow", nullptr, theWindowFlags);
		auto newView = view->Print([](const auto&) {});
		if (newView) {
			view->Exit();
			view = std::move(newView);
			view->Enter();
		}
		ImGui::End();
	}
	window->Shutdown();
}

// NOTE: this is here sothat no extra console window is opened.
#include <windows.h>
int APIENTRY
WinMain(HINSTANCE /*unused*/, HINSTANCE /*unused*/, PSTR /*unused*/, int /*unused*/) {
	return main();
}
