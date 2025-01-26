#include "windows_adapter.hpp"

LRESULT WINAPI WndProc(HWND aHwnd, UINT aMsg, WPARAM aWparam, LPARAM aLparam);

static constexpr auto theColorBits = 32;
static auto theWidth = 1280; // NOLINT
static auto theHeight = 720; // NOLINT

std::unique_ptr<ImGuiAdapter>
ImGuiAdapter::CreateWindows() { // NOLINT
	return std::make_unique<WindowsAdapter>();
}

void
WindowsAdapter::Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) {
	theWidth = aWidth;
	theHeight = aHight;
	myClearColor = aClearColor;

	int nameLengthAsWChar = MultiByteToWideChar(
		CP_UTF8,
		MB_USEGLYPHCHARS,
		aName.c_str(),
		static_cast<int>(aName.size()),
		nullptr,
		0);
	if (nameLengthAsWChar == 0) {
		return;
	}
	std::wstring convertedName{};
	convertedName.resize(nameLengthAsWChar);
	MultiByteToWideChar(
		CP_UTF8,
		MB_USEGLYPHCHARS,
		aName.c_str(),
		static_cast<int>(aName.size()),
		convertedName.data(),
		nameLengthAsWChar);

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
		convertedName.c_str(),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		aWidth,
		aHight,
		nullptr,
		nullptr,
		myWc.hInstance,
		nullptr);

	// Initialize OpenGL
	if (!PrivCreateDeviceWGL(myHwnd, &myMainWindow)) {
		PrivCleanupDeviceWGL(myHwnd, &myMainWindow);
		::DestroyWindow(myHwnd);
		::UnregisterClassW(myWc.lpszClassName, myWc.hInstance);
		return;
	}
	wglMakeCurrent(myMainWindow.hDC, myHrc);

	// Show the window
	::ShowWindow(myHwnd, SW_SHOWDEFAULT);
	::UpdateWindow(myHwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& theIo = ImGui::GetIO();
	(void)theIo;
	// NOLINTNEXTLINE
	theIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// NOLINTNEXTLINE
	theIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	// NOLINTNEXTLINE
	theIo.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multiple Viewports

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_InitForOpenGL(myHwnd);
	ImGui_ImplOpenGL3_Init();

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
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE) == TRUE) {
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
	// Rendering
	ImGui::Render();

	glViewport(0, 0, theWidth, theHeight);
	glClearColor(myClearColor.x, myClearColor.y, myClearColor.z, myClearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Present
	::SwapBuffers(myMainWindow.hDC);

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void
WindowsAdapter::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	PrivCleanupDeviceWGL(myHwnd, &myMainWindow);
	wglDeleteContext(myHrc);
	::DestroyWindow(myHwnd);
	::UnregisterClassW(myWc.lpszClassName, myWc.hInstance);
}

// Helper functions
bool
WindowsAdapter::PrivCreateDeviceWGL(HWND aHwnd, WglWindowData* aData) {
	HDC hDc = ::GetDC(aHwnd);
	PIXELFORMATDESCRIPTOR pfd = {0};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = theColorBits;

	const int pixelFormat = ::ChoosePixelFormat(hDc, &pfd);
	if (pixelFormat == 0) {
		return false;
	}
	if (::SetPixelFormat(hDc, pixelFormat, &pfd) == FALSE) {
		return false;
	}
	::ReleaseDC(aHwnd, hDc);

	aData->hDC = ::GetDC(aHwnd);
	if (myHrc == nullptr) {
		myHrc = wglCreateContext(aData->hDC);
	}
	return true;
}

void
WindowsAdapter::PrivCleanupDeviceWGL(HWND aHwnd, WglWindowData* aData) {
	wglMakeCurrent(nullptr, nullptr);
	::ReleaseDC(aHwnd, aData->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT // NOLINTNEXTLINE
ImGui_ImplWin32_WndProcHandler(HWND aHwnd, UINT aMsg, WPARAM aWparam, LPARAM aLparam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to
// use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
// clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
// application, or clear/overwrite your copy of the keyboard data. Generally you may always pass all
// inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI
WndProc(HWND aHwnd, UINT aMsg, WPARAM aWparam, LPARAM aLparam) {
	// NOLINTNEXTLINE
	if (ImGui_ImplWin32_WndProcHandler(aHwnd, aMsg, aWparam, aLparam)) {
		return TRUE;
	}

	// NOLINTNEXTLINE
	switch (aMsg) {
	case WM_SIZE:
		if (aWparam != SIZE_MINIMIZED) {
			theWidth = LOWORD(aLparam);
			theHeight = HIWORD(aLparam);
		}
		return 0;
	case WM_SYSCOMMAND:
		// NOLINTNEXTLINE
		if ((aWparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
		{
			return 0;
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(aHwnd, aMsg, aWparam, aLparam);
}
