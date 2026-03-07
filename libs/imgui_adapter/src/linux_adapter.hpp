#include "imgui_adapter/imgui_adapter.hpp"

#include <SDL2/SDL.h>

// NOLINTBEGIN
class LinuxAdapter : public ImGuiAdapter {
public:
	void Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) override;
	bool Update() override;
	void Show() override;
	void Shutdown() override;

private:
	SDL_Window* window;
	ImVec4 clear_color;
};
// NOLINTEND