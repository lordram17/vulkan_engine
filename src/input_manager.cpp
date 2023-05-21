#include "input_manager.h"

//allocating memory for the static variables
std::unordered_map<IVRKey, bool> IVRKeyStatus::KeyStatus_;
int IVRMouseStatus::MouseX_;
int IVRMouseStatus::MouseY_;
bool IVRMouseStatus::LeftButtonPressed_;
bool IVRMouseStatus::RightButtonPressed_;
bool IVRMouseStatus::MiddleButtonPressed_;

void IVRKeyStatus::Init()
{
	for (int i = 0; i < 1024; i++)
		KeyStatus_[(IVRKey)i] = false;
}

void IVRKeyStatus::SetKeyStatus(IVRKey key, bool status)
{
	KeyStatus_[key] = status;
}

bool IVRKeyStatus::GetKeyStatus(IVRKey key)
{
	return KeyStatus_[key];
}

IVRInputManager::IVRInputManager(std::shared_ptr<IVRWindow> window) :
	Window_(window)
{
	IVRKeyStatus::Init();
}

IVRInputManager::~IVRInputManager()
{
}

void IVRInputManager::PollInputs()
{
	glfwPollEvents();
	CheckForMouseInputs();
	CheckForKeyboardInputs();
}

void IVRInputManager::CheckForMouseInputs()
{
	//check for mouse button presses
	glfwGetMouseButton(Window_->GetGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? IVRMouseStatus::LeftButtonPressed_ = true : IVRMouseStatus::LeftButtonPressed_ = false;
	glfwGetMouseButton(Window_->GetGLFWWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? IVRMouseStatus::RightButtonPressed_ = true : IVRMouseStatus::RightButtonPressed_ = false;
	glfwGetMouseButton(Window_->GetGLFWWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS ? IVRMouseStatus::MiddleButtonPressed_ = true : IVRMouseStatus::MiddleButtonPressed_ = false;
	//check for mouse position
	double x, y;
	glfwGetCursorPos(Window_->GetGLFWWindow(), &x, &y);
	int width, height;
	glfwGetWindowSize(Window_->GetGLFWWindow(), &width, &height);
	IVRMouseStatus::MouseX_ =  (int)x;
	IVRMouseStatus::MouseY_ =  (int)y;

	//glfwSetCursorPos(Window_->GetGLFWWindow(), width / 2, height / 2);
}

void IVRInputManager::CheckForKeyboardInputs()
{
	//check for key presses for all values in the enum using 
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::A, true) : IVRKeyStatus::SetKeyStatus(IVRKey::A, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_B) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::B, true) : IVRKeyStatus::SetKeyStatus(IVRKey::B, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_C) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::C, true) : IVRKeyStatus::SetKeyStatus(IVRKey::C, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::D, true) : IVRKeyStatus::SetKeyStatus(IVRKey::D, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_E) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::E, true) : IVRKeyStatus::SetKeyStatus(IVRKey::E, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_G) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::G, true) : IVRKeyStatus::SetKeyStatus(IVRKey::G, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_H) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::H, true) : IVRKeyStatus::SetKeyStatus(IVRKey::H, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_I) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::I, true) : IVRKeyStatus::SetKeyStatus(IVRKey::I, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_J) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::J, true) : IVRKeyStatus::SetKeyStatus(IVRKey::J, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_K) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::K, true) : IVRKeyStatus::SetKeyStatus(IVRKey::K, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_L) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::L, true) : IVRKeyStatus::SetKeyStatus(IVRKey::L, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_M) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::M, true) : IVRKeyStatus::SetKeyStatus(IVRKey::M, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_N) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::N, true) : IVRKeyStatus::SetKeyStatus(IVRKey::N, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_O) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::O, true) : IVRKeyStatus::SetKeyStatus(IVRKey::O, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_P) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::P, true) : IVRKeyStatus::SetKeyStatus(IVRKey::P, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_Q) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::Q, true) : IVRKeyStatus::SetKeyStatus(IVRKey::Q, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_R) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::R, true) : IVRKeyStatus::SetKeyStatus(IVRKey::R, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::S, true) : IVRKeyStatus::SetKeyStatus(IVRKey::S, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_T) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::T, true) : IVRKeyStatus::SetKeyStatus(IVRKey::T, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_U) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::U, true) : IVRKeyStatus::SetKeyStatus(IVRKey::U, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_V) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::V, true) : IVRKeyStatus::SetKeyStatus(IVRKey::V, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::W, true) : IVRKeyStatus::SetKeyStatus(IVRKey::W, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_X) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::X, true) : IVRKeyStatus::SetKeyStatus(IVRKey::X, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_Y) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::Y, true) : IVRKeyStatus::SetKeyStatus(IVRKey::Y, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_Z) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::Z, true) : IVRKeyStatus::SetKeyStatus(IVRKey::Z, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_0) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM0, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM0, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_1) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM1, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM1, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_2) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM2, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM2, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_3) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM3, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM3, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_4) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM4, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM4, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_5) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM5, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM5, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_6) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM6, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM6, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_7) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM7, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM7, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_8) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM8, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM8, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_9) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::NUM9, true) : IVRKeyStatus::SetKeyStatus(IVRKey::NUM9, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_SPACE) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::SPACE, true) : IVRKeyStatus::SetKeyStatus(IVRKey::SPACE, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::LEFTSHIFT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::LEFTSHIFT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTSHIFT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTSHIFT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::LEFTCTRL, true) : IVRKeyStatus::SetKeyStatus(IVRKey::LEFTCTRL, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTCTRL, true) : IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTCTRL, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::LEFTALT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::LEFTALT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTALT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::RIGHTALT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::ESCAPE, true) : IVRKeyStatus::SetKeyStatus(IVRKey::ESCAPE, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_ENTER) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::ENTER, true) : IVRKeyStatus::SetKeyStatus(IVRKey::ENTER, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_BACKSPACE) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::BACKSPACE, true) : IVRKeyStatus::SetKeyStatus(IVRKey::BACKSPACE, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_TAB) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::TAB, true) : IVRKeyStatus::SetKeyStatus(IVRKey::TAB, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_LEFT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::LEFT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::LEFT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::RIGHT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::RIGHT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_UP) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::UP, true) : IVRKeyStatus::SetKeyStatus(IVRKey::UP, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_DOWN) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::DOWN, true) : IVRKeyStatus::SetKeyStatus(IVRKey::DOWN, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_PAGE_UP) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::PAGEUP, true) : IVRKeyStatus::SetKeyStatus(IVRKey::PAGEUP, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_PAGE_DOWN) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::PAGEDOWN, true) : IVRKeyStatus::SetKeyStatus(IVRKey::PAGEDOWN, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_HOME) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::HOME, true) : IVRKeyStatus::SetKeyStatus(IVRKey::HOME, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_END) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::END, true) : IVRKeyStatus::SetKeyStatus(IVRKey::END, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_INSERT) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::INSERT, true) : IVRKeyStatus::SetKeyStatus(IVRKey::INSERT, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_DELETE) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::DEL, true) : IVRKeyStatus::SetKeyStatus(IVRKey::DEL, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F1, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F1, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F2) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F2, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F2, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F3) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F3, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F3, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F4) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F4, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F4, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F5) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F5, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F5, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F6) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F6, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F6, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F7) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F7, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F7, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F8) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F8, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F8, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F9) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F9, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F9, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F10) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F10, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F10, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F11) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F11, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F11, false);
	glfwGetKey(Window_->GetGLFWWindow(), GLFW_KEY_F12) == GLFW_PRESS ? IVRKeyStatus::SetKeyStatus(IVRKey::F12, true) : IVRKeyStatus::SetKeyStatus(IVRKey::F12, false);
}
