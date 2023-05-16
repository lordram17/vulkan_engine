#pragma once

#include <chrono>

#include "world.h"
#include "ivr_engine.h"
#include "input_manager.h"

class IVRApp {
private:

	std::shared_ptr<IVRWorld> World_;
	std::shared_ptr<IVREngine> Engine_;
	std::shared_ptr<IVRInputManager> InputManager_;

	std::chrono::high_resolution_clock::time_point CurrentTime_;
	std::chrono::high_resolution_clock::time_point PreviousTime_;
	float FrameTime_;

public:

	IVRApp();
	~IVRApp() {};
	void Mainloop();
};