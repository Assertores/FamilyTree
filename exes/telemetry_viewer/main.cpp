#include <entries.hpp>
#include <imgui_adapter.hpp>
#include <utils.hpp>

static constexpr auto theWindowWidth = 1280;
static constexpr auto theWindowHight = 720;

void
SetupFrame() {
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

int
main() { // NOLINT(bugprone-exception-escape)
	auto window = ImGuiAdapter::CreateWindows();

	auto path = std::filesystem::current_path();

	window->Init(theWindowWidth, theWindowHight, "Family Tree UI", {});

	std::filesystem::path folder;
	bool stop = true;
	for (; window->Update(); window->Show()) {
		SetupFrame();
		if (ImGui::Button("Load Data")) {
			folder = ui::GetFolder();
			if (!folder.empty()) {
				stop = false;
				break;
			}
		}
		ImGui::End();
	}
	if (stop) {
		return 0;
	}
	ImGui::End();
	window->Show();

	std::vector<std::vector<ui::Trace>> roots;
	for (const auto& rootPath : ui::GetDirs(folder)) {
		std::vector<ui::Trace> traces;
		for (const auto& path : ui::GetDirs(rootPath)) {
			traces.emplace_back(ui::Trace(path / (path.filename().u8string() + ".trace")));
		}
		roots.emplace_back(std::move(traces));
	}

	for (; window->Update(); window->Show()) {
		SetupFrame();
		ImGui::BeginTabBar("Runs");
		for (size_t i = 0; i < roots.size(); i++) {
			if (ImGui::BeginTabItem(std::to_string(i).c_str())) {
				ImGui::BeginTabBar("Root Traces");
				for (const auto& root : roots[i]) {
					ImGui::PushID(&root);
					if (ImGui::BeginTabItem(root.GetName().c_str())) {
						root.Show();
						ImGui::EndTabItem();
					}
					ImGui::PopID();
				}
				ImGui::EndTabBar();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
		ImGui::End();
	}

	window->Shutdown();
	return 0;
}

#ifdef _WIN32
// NOTE: this is here sothat no extra console window is opened.
#include <windows.h>
int APIENTRY
WinMain(HINSTANCE /*unused*/, HINSTANCE /*unused*/, PSTR /*unused*/, int /*unused*/) {
	return main();
}
#endif
