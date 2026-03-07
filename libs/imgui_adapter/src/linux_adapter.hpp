#pragma once

#include "imgui_adapter/imgui_adapter.hpp"
#include "imgui_impl_vulkan.h"

#include <SDL2/SDL.h>

class LinuxAdapter : public ImGuiAdapter {
public:
	void Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) override;
	bool Update() override;
	void Show() override;
	void Shutdown() override;

private:
	SDL_Window* myWindow = nullptr;
	ImVec4 myClearColor;

	VkAllocationCallbacks* myAllocator = nullptr;
	VkInstance myInstance = VK_NULL_HANDLE;
	VkPhysicalDevice myPhysicalDevice = VK_NULL_HANDLE;
	VkDevice myDevice = VK_NULL_HANDLE;
	uint32_t myQueueFamily = (uint32_t)-1;
	VkQueue myQueue = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT myDebugReport = VK_NULL_HANDLE;
	VkPipelineCache myPipelineCache = VK_NULL_HANDLE;
	VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

	static ImGui_ImplVulkanH_Window theMainWindowData;
	uint32_t myMinImageCount = 2;
	bool mySwapChainRebuild = false;

	static bool PrivIsExtensionAvailable(
		const ImVector<VkExtensionProperties>& aProperties,
		const char* aExtension);
	void PrivSetupVulkan(ImVector<const char*> aInstanceExtensions);
	void
	PrivSetupVulkanWindow(ImGui_ImplVulkanH_Window* aWindow, VkSurfaceKHR aSurface, int aWidth, int aHeight);
	void PrivCleanupVulkan();
	void PrivCleanupVulkanWindow();
	void PrivFrameRender(ImGui_ImplVulkanH_Window* aWindow, ImDrawData* aDrawData);
	void PrivFramePresent(ImGui_ImplVulkanH_Window* aWindow);
};