#include "linux_adapter.hpp"

#include <array>
#include <stdexcept>
#include <tuple>

std::unique_ptr<ImGuiAdapter>
ImGuiAdapter::CreateWindows() { // NOLINT
	return std::make_unique<LinuxAdapter>();
}

// Dear ImGui: standalone example application for SDL2 + Vulkan

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own
// engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with
// imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering backend in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp)
// and by
//   the backend itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own
//   engine/app code.
// Read comments in imgui_impl_vulkan.h.

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <cstdio>  // printf, fprintf
#include <cstdlib> // abort

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

// #define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// NOTE: this is here because otherwise clang complains about the default constructur of this type.
ImGui_ImplVulkanH_Window LinuxAdapter::theMainWindowData = {};

static void
CheckVkResult(VkResult aErr) {
	if (aErr == VK_SUCCESS) {
		return;
	}
	// NOLINTNEXTLINE
	std::ignore = fprintf(stderr, "[vulkan] Error: VkResult = %d\n", aErr);
	if (aErr < 0) {
		abort();
	}
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_report(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData) {
	(void)flags;
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pUserData;
	(void)pLayerPrefix; // Unused arguments
	fprintf(
		stderr,
		"[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n",
		objectType,
		pMessage);
	return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

bool
LinuxAdapter::PrivIsExtensionAvailable(
	const ImVector<VkExtensionProperties>& aProperties,
	const char* aExtension) {
	for (const VkExtensionProperties& property : aProperties) {
		// NOLINTNEXTLINE
		if (strcmp(property.extensionName, aExtension) == 0) {
			return true;
		}
	}
	return false;
}

void
LinuxAdapter::PrivSetupVulkan(ImVector<const char*> aInstanceExtensions) {
	VkResult err{};
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
	volkInitialize();
#endif

	// Create Vulkan Instance
	{
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		// Enumerate available extensions
		uint32_t propertiesCount = 0;
		ImVector<VkExtensionProperties> aProperties;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, nullptr);
		// NOLINTNEXTLINE
		aProperties.resize(propertiesCount);
		err = vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, aProperties.Data);
		CheckVkResult(err);

		// Enable required extensions
		if (PrivIsExtensionAvailable(
				aProperties,
				VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
			aInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
		if (PrivIsExtensionAvailable(aProperties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
			aInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

		// Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
		const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = layers;
		aInstanceExtensions.push_back("VK_EXT_debug_report");
#endif

		// Create Vulkan Instance
		createInfo.enabledExtensionCount = (uint32_t)aInstanceExtensions.Size;
		createInfo.ppEnabledExtensionNames = aInstanceExtensions.Data;
		err = vkCreateInstance(&createInfo, myAllocator, &myInstance);
		CheckVkResult(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
		volkLoadInstance(myInstance);
#endif

		// Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
		auto f_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr(myInstance, "vkCreateDebugReportCallbackEXT");
		IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
		VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
		debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
								| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debug_report_ci.pfnCallback = debug_report;
		debug_report_ci.pUserData = nullptr;
		err = f_vkCreateDebugReportCallbackEXT(
			myInstance,
			&debug_report_ci,
			myAllocator,
			&myDebugReport);
		CheckVkResult(err);
#endif
	}

	// Select Physical Device (GPU)
	myPhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(myInstance);
	IM_ASSERT(myPhysicalDevice != VK_NULL_HANDLE);

	// Select graphics queue family
	myQueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(myPhysicalDevice);
	IM_ASSERT(myQueueFamily != (uint32_t)-1);

	// Create Logical Device (with 1 queue)
	{
		ImVector<const char*> deviceExtensions;
		deviceExtensions.push_back("VK_KHR_swapchain");

		// Enumerate physical device aExtension
		uint32_t propertiesCount = 0;
		ImVector<VkExtensionProperties> aProperties;
		vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &propertiesCount, nullptr);
		// NOLINTNEXTLINE
		aProperties.resize(propertiesCount);
		vkEnumerateDeviceExtensionProperties(
			myPhysicalDevice,
			nullptr,
			&propertiesCount,
			aProperties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
		if (PrivIsExtensionAvailable(aProperties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
			deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
		const std::array queuePriority = {1.0F};
		std::array<VkDeviceQueueCreateInfo, 1> queueInfo = {};
		queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[0].queueFamilyIndex = myQueueFamily;
		queueInfo[0].queueCount = 1;
		queueInfo[0].pQueuePriorities = queuePriority.data();
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
		createInfo.pQueueCreateInfos = queueInfo.data();
		createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.Size;
		createInfo.ppEnabledExtensionNames = deviceExtensions.Data;
		err = vkCreateDevice(myPhysicalDevice, &createInfo, myAllocator, &myDevice);
		CheckVkResult(err);
		vkGetDeviceQueue(myDevice, myQueueFamily, 0, &myQueue);
	}

	// Create Descriptor Pool
	// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
	{
		std::array poolSizes = {
			VkDescriptorPoolSize{
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
		};
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 0;
		for (VkDescriptorPoolSize& poolSize : poolSizes) {
			poolInfo.maxSets += poolSize.descriptorCount;
		}
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		err = vkCreateDescriptorPool(myDevice, &poolInfo, myAllocator, &myDescriptorPool);
		CheckVkResult(err);
	}
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
void
LinuxAdapter::PrivSetupVulkanWindow(
	ImGui_ImplVulkanH_Window* aWindow,
	VkSurfaceKHR aSurface,
	int aWidth,
	int aHeight) {
	aWindow->Surface = aSurface;

	// Check for WSI support
	VkBool32 res{};
	vkGetPhysicalDeviceSurfaceSupportKHR(myPhysicalDevice, myQueueFamily, aWindow->Surface, &res);
	if (res != VK_TRUE) {
		// NOLINTNEXTLINE
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const std::array requestSurfaceImageFormat = {
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8_UNORM,
		VK_FORMAT_R8G8B8_UNORM};
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	aWindow->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
		myPhysicalDevice,
		aWindow->Surface,
		requestSurfaceImageFormat.data(),
		requestSurfaceImageFormat.size(),
		requestSurfaceColorSpace);

	// Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
	VkPresentModeKHR presentModes[] = {
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_FIFO_KHR};
#else
	std::array presentModes = {VK_PRESENT_MODE_FIFO_KHR};
#endif
	aWindow->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
		myPhysicalDevice,
		aWindow->Surface,
		presentModes.data(),
		presentModes.size());
	// printf("[vulkan] Selected PresentMode = %d\n", aWindow->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(myMinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(
		myInstance,
		myPhysicalDevice,
		myDevice,
		aWindow,
		myQueueFamily,
		myAllocator,
		aWidth,
		aHeight,
		myMinImageCount);
}

void
LinuxAdapter::PrivCleanupVulkan() {
	vkDestroyDescriptorPool(myDevice, myDescriptorPool, myAllocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
	// Remove the debug report callback
	auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(myInstance, "vkDestroyDebugReportCallbackEXT");
	f_vkDestroyDebugReportCallbackEXT(myInstance, myDebugReport, myAllocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

	vkDestroyDevice(myDevice, myAllocator);
	vkDestroyInstance(myInstance, myAllocator);
}

void
LinuxAdapter::PrivCleanupVulkanWindow() {
	ImGui_ImplVulkanH_DestroyWindow(myInstance, myDevice, &theMainWindowData, myAllocator);
}

void
LinuxAdapter::PrivFrameRender(ImGui_ImplVulkanH_Window* aWindow, ImDrawData* aDrawData) {
	VkSemaphore imageAcquiredSemaphore =
		aWindow->FrameSemaphores[aWindow->SemaphoreIndex].ImageAcquiredSemaphore; // NOLINT
	VkSemaphore renderCompleteSemaphore =
		aWindow->FrameSemaphores[aWindow->SemaphoreIndex].RenderCompleteSemaphore; // NOLINT
	VkResult err = vkAcquireNextImageKHR(
		myDevice,
		aWindow->Swapchain,
		UINT64_MAX,
		imageAcquiredSemaphore,
		VK_NULL_HANDLE,
		&aWindow->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		mySwapChainRebuild = true;
	}
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		return;
	}
	if (err != VK_SUBOPTIMAL_KHR) {
		CheckVkResult(err);
	}

	// NOLINTNEXTLINE
	ImGui_ImplVulkanH_Frame* frameData = &aWindow->Frames[aWindow->FrameIndex];
	{
		err = vkWaitForFences(myDevice, 1, &frameData->Fence, VK_TRUE, UINT64_MAX); // wait
																					// indefinitely
																					// instead of
																					// periodically
																					// checking
		CheckVkResult(err);

		err = vkResetFences(myDevice, 1, &frameData->Fence);
		CheckVkResult(err);
	}
	{
		err = vkResetCommandPool(myDevice, frameData->CommandPool, 0);
		CheckVkResult(err);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(frameData->CommandBuffer, &info);
		CheckVkResult(err);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = aWindow->RenderPass;
		info.framebuffer = frameData->Framebuffer;
		info.renderArea.extent.width = aWindow->Width;
		info.renderArea.extent.height = aWindow->Height;
		info.clearValueCount = 1;
		info.pClearValues = &aWindow->ClearValue;
		vkCmdBeginRenderPass(frameData->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(aDrawData, frameData->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(frameData->CommandBuffer);
	{
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &imageAcquiredSemaphore;
		info.pWaitDstStageMask = &waitStage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &frameData->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &renderCompleteSemaphore;

		err = vkEndCommandBuffer(frameData->CommandBuffer);
		CheckVkResult(err);
		err = vkQueueSubmit(myQueue, 1, &info, frameData->Fence);
		CheckVkResult(err);
	}
}

void
LinuxAdapter::PrivFramePresent(ImGui_ImplVulkanH_Window* aWindow) {
	if (mySwapChainRebuild) {
		return;
	}
	VkSemaphore renderCompleteSemaphore =
		aWindow->FrameSemaphores[aWindow->SemaphoreIndex].RenderCompleteSemaphore; // NOLINT
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &renderCompleteSemaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &aWindow->Swapchain;
	info.pImageIndices = &aWindow->FrameIndex;
	VkResult err = vkQueuePresentKHR(myQueue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		mySwapChainRebuild = true;
	}
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		return;
	}
	if (err != VK_SUBOPTIMAL_KHR) {
		CheckVkResult(err);
	}

	// Now we can use the next set of semaphores
	aWindow->SemaphoreIndex = (aWindow->SemaphoreIndex + 1) % aWindow->SemaphoreCount;
}

// Main code
void
LinuxAdapter::Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) {
	ImVec4 myClearColor = aClearColor;

	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		// NOLINTNEXTLINE
		printf("Error: %s\n", SDL_GetError());
		throw std::runtime_error(SDL_GetError());
	}

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with Vulkan graphics context
	auto windowFlags =
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI; // NOLINT
	myWindow = SDL_CreateWindow(
		aName.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		aWidth,
		aHight,
		windowFlags);
	if (myWindow == nullptr) {
		// NOLINTNEXTLINE
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		throw std::runtime_error(SDL_GetError());
	}

	ImVector<const char*> extensions;
	uint32_t extensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(myWindow, &extensionsCount, nullptr);
	extensions.resize(extensionsCount); // NOLINT
	SDL_Vulkan_GetInstanceExtensions(myWindow, &extensionsCount, extensions.Data);
	PrivSetupVulkan(extensions);

	// Create Window Surface
	VkSurfaceKHR aSurface{};
	VkResult err{};
	if (SDL_Vulkan_CreateSurface(myWindow, myInstance, &aSurface) == 0) {
		// NOLINTNEXTLINE
		printf("Failed to create Vulkan aSurface.\n");
		throw std::runtime_error("Vulkan");
	}

	// Create Framebuffers
	int width{};
	int height{};
	SDL_GetWindowSize(myWindow, &width, &height);
	ImGui_ImplVulkanH_Window* aWindow = &theMainWindowData;
	PrivSetupVulkanWindow(aWindow, aSurface, width, height);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); // NOLINT
	(void)io;
	// NOLINTBEGIN
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
	// NOLINTEND

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
	// identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	// NOLINTNEXTLINE
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0F;
		style.Colors[ImGuiCol_WindowBg].w = 1.0F;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForVulkan(myWindow);
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = myInstance;
	initInfo.PhysicalDevice = myPhysicalDevice;
	initInfo.Device = myDevice;
	initInfo.QueueFamily = myQueueFamily;
	initInfo.Queue = myQueue;
	initInfo.PipelineCache = myPipelineCache;
	initInfo.DescriptorPool = myDescriptorPool;
	initInfo.RenderPass = aWindow->RenderPass;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = myMinImageCount;
	initInfo.ImageCount = aWindow->ImageCount;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = myAllocator;
	initInfo.CheckVkResultFn = CheckVkResult;
	ImGui_ImplVulkan_Init(&initInfo);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple
	// fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the
	// font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those
	// errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture
	// when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below
	// will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher
	// quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to
	// write a double backslash \\ !
	// io.Fonts->AddFontDefault();
	// io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	// ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
	// nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
}

bool
LinuxAdapter::Update() {
	// Poll and handle events (inputs, window resize, etc.)
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui
	// wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main
	// application, or clear/overwrite your copy of the mouse data.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
	// application, or clear/overwrite your copy of the keyboard data. Generally you may always
	// pass all inputs to dear imgui, and hide them from your application based on those two
	// flags.
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT) {
			return false;
		}
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE
			&& event.window.windowID == SDL_GetWindowID(myWindow)) {
			return false;
		}
	}
	// NOLINTNEXTLINE
	if (SDL_GetWindowFlags(myWindow) & SDL_WINDOW_MINIMIZED) {
		SDL_Delay(10); // NOLINT
		return true;
	}

	// Resize swap chain?
	int fbWidth{};
	int fbHeight{};
	SDL_GetWindowSize(myWindow, &fbWidth, &fbHeight);
	if (fbWidth > 0 && fbHeight > 0
		&& (mySwapChainRebuild || theMainWindowData.Width != fbWidth
			|| theMainWindowData.Height != fbHeight)) {
		ImGui_ImplVulkan_SetMinImageCount(myMinImageCount);
		ImGui_ImplVulkanH_CreateOrResizeWindow(
			myInstance,
			myPhysicalDevice,
			myDevice,
			&theMainWindowData,
			myQueueFamily,
			myAllocator,
			fbWidth,
			fbHeight,
			myMinImageCount);
		theMainWindowData.FrameIndex = 0;
		mySwapChainRebuild = false;
	}

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	return true;
}

void
LinuxAdapter::Show() {
	ImGui_ImplVulkanH_Window* aWindow = &theMainWindowData;
	ImGuiIO& io = ImGui::GetIO(); // NOLINT

	// Rendering
	ImGui::Render();
	ImDrawData* mainDrawData = ImGui::GetDrawData();
	const bool mainIsMinimized =
		(mainDrawData->DisplaySize.x <= 0.0F || mainDrawData->DisplaySize.y <= 0.0F);
	aWindow->ClearValue.color.float32[0] = myClearColor.x * myClearColor.w;
	aWindow->ClearValue.color.float32[1] = myClearColor.y * myClearColor.w;
	aWindow->ClearValue.color.float32[2] = myClearColor.z * myClearColor.w;
	aWindow->ClearValue.color.float32[3] = myClearColor.w;
	if (!mainIsMinimized) {
		PrivFrameRender(aWindow, mainDrawData);
	}

	// Update and Render additional Platform Windows
	// NOLINTNEXTLINE
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Present Main Platform Window
	if (!mainIsMinimized) {
		PrivFramePresent(aWindow);
	}
}

void
LinuxAdapter::Shutdown() {
	VkResult err{};

	// Cleanup
	err = vkDeviceWaitIdle(myDevice);
	CheckVkResult(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	PrivCleanupVulkanWindow();
	PrivCleanupVulkan();

	SDL_DestroyWindow(myWindow);
	SDL_Quit();
}
