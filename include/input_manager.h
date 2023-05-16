#pragma once

#include <unordered_map>
#include <memory>

#include "ivr_window.h"

enum IVRKey 
{
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	SPACE, ESCAPE, ENTER, UP, DOWN, LEFT, RIGHT, LEFTSHIFT, RIGHTSHIFT, LEFTCTRL, RIGHTCTRL, LEFTALT, RIGHTALT, TAB, BACKSPACE, INSERT, HOME, END, PAGEUP, PAGEDOWN, DEL,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

class IVRKeyStatus {
	static std::unordered_map<IVRKey, bool> KeyStatus_;

public:
	IVRKeyStatus() {};

	static void Init();

	static void SetKeyStatus(IVRKey key, bool status);

	static bool GetKeyStatus(IVRKey key);
};

struct IVRMouseStatus {
	static int MouseX_;
	static int MouseY_;

	static bool LeftButtonPressed_;
	static bool RightButtonPressed_;
	static bool MiddleButtonPressed_;
};


class IVRInputManager
{
private:

	std::shared_ptr<IVRWindow> Window_;

	void CheckForKeyboardInputs();
	void CheckForMouseInputs();

public:
	IVRInputManager(std::shared_ptr<IVRWindow> window);

	~IVRInputManager();

	void PollInputs();

};

