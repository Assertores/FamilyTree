#include "src/imgui_adapter.hpp"

static constexpr auto theWindowWidth = 1280;
static constexpr auto theWindowHight = 720;

int
main() { // NOLINT(bugprone-exception-escape)
	auto window = ImGuiAdapter::CreateWindows();
	for (window->Init(theWindowWidth, theWindowHight, "Family Tree UI", {}); window->Update();
		 window->Show()) {
	}
	window->Shutdown();
}