// Dear ImGui: standalone example application for Win32 + OpenGL 3

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// This is provided for completeness, however it is strongly recommended you use OpenGL with SDL or
// GLFW.

#include "windows_adapter.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <GL/GL.h>
#include <tchar.h>
#include <windows.h>

std::unique_ptr<ImGuiAdapter>
ImGuiAdapter::CreateWindows() { // NOLINT
	return std::make_unique<WindowsAdapter>();
}

// Data stored per platform window
struct WglWindowData {
	HDC hDC;
};

// Data
static HGLRC theHRC;
static WglWindowData theMainWindow;
static int theWidth;
static int theHeight;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND aHwnd, WglWindowData* aData);
void CleanupDeviceWGL(HWND aHwnd, WglWindowData* aData);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND aHwnd, UINT aMsg, WPARAM aWParam, LPARAM aLParam);

// Support function for multi-viewports
// Unlike most other backend combination, we need specific hooks to combine Win32+OpenGL.
// We could in theory decide to support Win32-specific code in OpenGL backend via e.g. an
// hypothetical ImGui_ImplOpenGL3_InitForRawWin32().
static void
HookRendererCreateWindow(ImGuiViewport* aViewport) {
	assert(aViewport->RendererUserData == nullptr);

	auto* data = IM_NEW(WglWindowData);
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	CreateDeviceWGL((HWND)aViewport->PlatformHandle, data);
	aViewport->RendererUserData = data;
}

static void
HookRendererDestroyWindow(ImGuiViewport* aViewport) {
	if (aViewport->RendererUserData != nullptr) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
		auto* data = (WglWindowData*)aViewport->RendererUserData;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
		CleanupDeviceWGL((HWND)aViewport->PlatformHandle, data);
		IM_DELETE(data);
		aViewport->RendererUserData = nullptr;
	}
}

static void
HookPlatformRenderWindow(ImGuiViewport* aViewport, void* /*unused*/) {
	// Activate the platform window DC in the OpenGL rendering context
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	if (auto* data = (WglWindowData*)aViewport->RendererUserData) {
		wglMakeCurrent(data->hDC, theHRC);
	}
}

static void
HookRendererSwapBuffers(ImGuiViewport* aViewport, void* /*unused*/) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	if (auto* data = (WglWindowData*)aViewport->RendererUserData) {
		::SwapBuffers(data->hDC);
	}
}

void
WindowsAdapter::Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) {
	// Create application window
	// ImGui_ImplWin32_EnableDpiAwareness();
	myWc = {
		sizeof(myWc),
		CS_OWNDC,
		WndProc,
		0L,
		0L,
		GetModuleHandle(nullptr),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		L"ImGui Example",
		nullptr};
	::RegisterClassExW(&myWc);
	myHwnd = ::CreateWindowW(
		myWc.lpszClassName,
		L"Dear ImGui Win32+OpenGL3 Example",
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		1280,
		800,
		nullptr,
		nullptr,
		myWc.hInstance,
		nullptr);

	// Initialize OpenGL
	if (!CreateDeviceWGL(myHwnd, &theMainWindow)) {
		CleanupDeviceWGL(myHwnd, &theMainWindow);
		::DestroyWindow(myHwnd);
		::UnregisterClassW(myWc.lpszClassName, myWc.hInstance);
		return;
	}
	wglMakeCurrent(theMainWindow.hDC, theHRC);

	// Show the window
	::ShowWindow(myHwnd, SW_SHOWDEFAULT);
	::UpdateWindow(myHwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& locIo = ImGui::GetIO();
	(void)locIo;
	// NOLINTBEGIN
	locIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	locIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	 // Enable Gamepad Controls
	locIo.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	 // Enable Multi-Viewport / Platform
															 // Windows

	// NOLINTEND

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
	// identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0F;
	style.Colors[ImGuiCol_WindowBg].w = 1.0F;

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_InitForOpenGL(myHwnd);
	ImGui_ImplOpenGL3_Init();

	// Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32
	// and GL api.
	ImGuiPlatformIO& platformIo = ImGui::GetPlatformIO();
	IM_ASSERT(platformIo.Renderer_CreateWindow == nullptr);
	IM_ASSERT(platformIo.Renderer_DestroyWindow == nullptr);
	IM_ASSERT(platformIo.Renderer_SwapBuffers == nullptr);
	IM_ASSERT(platformIo.Platform_RenderWindow == nullptr);
	platformIo.Renderer_CreateWindow = HookRendererCreateWindow;
	platformIo.Renderer_DestroyWindow = HookRendererDestroyWindow;
	platformIo.Renderer_SwapBuffers = HookRendererSwapBuffers;
	platformIo.Platform_RenderWindow = HookPlatformRenderWindow;

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

	myIsValid = true;
}

bool
WindowsAdapter::Update() {
	if (!myIsValid) {
		return false;
	}

	// Poll and handle messages (inputs, window resize, etc.)
	// See the WndProc() function below for our to dispatch events to the Win32 backend.
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT) {
			return false;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	return true;
}

void
WindowsAdapter::Show() {
	ImVec4 constexpr clearColor = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);
	// Rendering
	ImGui::Render();
	glViewport(0, 0, theWidth, theHeight);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	ImGuiIO& locIo = ImGui::GetIO();
	(void)locIo;
	// NOLINTNEXTLINE(hicpp-signed-bitwise)
	if ((locIo.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		// Restore the OpenGL rendering context to the main window DC, since platform windows might
		// have changed it.
		wglMakeCurrent(theMainWindow.hDC, theHRC);
	}

	// Present
	::SwapBuffers(theMainWindow.hDC);
}

void
WindowsAdapter::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceWGL(myHwnd, &theMainWindow);
	wglDeleteContext(theHRC);
	::DestroyWindow(myHwnd);
	::UnregisterClassW(myWc.lpszClassName, myWc.hInstance);
}

// Helper functions
bool
CreateDeviceWGL(HWND aHwnd, WglWindowData* aData) {
	HDC hDc = ::GetDC(aHwnd);
	PIXELFORMATDESCRIPTOR pfd = {0};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32; // NOLINT

	const int locpf = ::ChoosePixelFormat(hDc, &pfd);
	if (locpf == 0) {
		return false;
	}
	if (::SetPixelFormat(hDc, locpf, &pfd) == FALSE) {
		return false;
	}
	::ReleaseDC(aHwnd, hDc);

	aData->hDC = ::GetDC(aHwnd);
	if (theHRC == nullptr) {
		theHRC = wglCreateContext(aData->hDC);
	}
	return true;
}

void
CleanupDeviceWGL(HWND aHwnd, WglWindowData* aData) {
	wglMakeCurrent(nullptr, nullptr);
	::ReleaseDC(aHwnd, aData->hDC);
}

// NOLINTBEGIN
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// NOLINTEND

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to
// use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
// clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
// application, or clear/overwrite your copy of the keyboard data. Generally you may always pass all
// inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI
WndProc(HWND aHwnd, UINT aMsg, WPARAM aWParam, LPARAM aLParam) {
	if (ImGui_ImplWin32_WndProcHandler(aHwnd, aMsg, aWParam, aLParam) != 0) {
		return 1;
	}

	switch (aMsg) {
	case WM_SIZE:
		if (aWParam != SIZE_MINIMIZED) {
			theWidth = LOWORD(aLParam);
			theHeight = HIWORD(aLParam);
		}
		return 0;
	case WM_SYSCOMMAND:
		// NOLINTNEXTLINE
		if ((aWParam & 0xfff0) == SC_KEYMENU) { // Disable ALT application menu
			return 0;
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return ::DefWindowProcW(aHwnd, aMsg, aWParam, aLParam);
}