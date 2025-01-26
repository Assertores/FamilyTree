#include "src/setup_view.hpp"

#include <imgui_adapter.hpp>

static constexpr auto theWindowWidth = 1280;
static constexpr auto theWindowHight = 720;

class Window {
public:
	explicit Window(std::shared_ptr<ui::View> aStartView)
		: myView(std::move(aStartView)) {}
	virtual ~Window() = default;

	virtual void Enter() = 0;
	virtual void Exit() = 0;

	std::shared_ptr<ui::View> myView; // NOLINT
	bool myIsOpen = true;			  // NOLINT
};

class MainWindow : public Window {
public:
	using Window::Window;

	void Enter() override {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::Begin(
			"MainWindow",
			nullptr,
			// NOLINTNEXTLINE
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
	}
	void Exit() override { ImGui::End(); }
};

class PopupWindow : public Window {
public:
	PopupWindow(std::string aName, std::shared_ptr<ui::View> aStartView)
		: Window(std::move(aStartView))
		, myName(std::move(aName)) {};

	void Enter() override {
		ImGui::Begin(
			myName.c_str(),
			&myIsOpen,
			// NOLINTNEXTLINE
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
	}
	void Exit() override { ImGui::End(); }

private:
	std::string myName;
};

int
main() { // NOLINT(bugprone-exception-escape)
	auto window = ImGuiAdapter::CreateWindows();

	auto mainWindow = std::make_unique<MainWindow>(std::make_shared<ui::SetupView>());
	mainWindow->myView->Enter();

	std::vector<std::unique_ptr<Window>> windows;
	windows.emplace_back(std::move(mainWindow));

	for (window->Init(theWindowWidth, theWindowHight, "Family Tree UI", {}); window->Update();
		 window->Show()) {
		// ImGui::SetNextWindowPos({});
		// ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		for (size_t i = 0; i < windows.size(); i++) {
			windows[i]->Enter();
			auto newView = windows[i]->myView->Print([&](auto aView) {
				aView->Enter();
				auto newWindow = std::make_unique<PopupWindow>(
					"##" + std::to_string(windows.size()),
					std::move(aView));
				windows.emplace_back(std::move(newWindow));
			});
			if (newView) {
				windows[i]->myView->Exit();
				windows[i]->myView = std::move(newView);
				windows[i]->myView->Enter();
			}
			windows[i]->Exit();
			if (!windows[i]->myIsOpen) {
				windows.erase(windows.begin() + i); // NOLINT
				i--;
			}
		}
	}
	window->Shutdown();
}

// NOTE: this is here sothat no extra console window is opened.
#include <windows.h>
int APIENTRY
WinMain(HINSTANCE /*unused*/, HINSTANCE /*unused*/, PSTR /*unused*/, int /*unused*/) {
	return main();
}
