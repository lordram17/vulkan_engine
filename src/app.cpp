#include "app.h"

IVRApp::IVRApp()
{
	Engine_ = std::make_shared<IVREngine>();
	InputManager_ = std::make_shared<IVRInputManager>(Engine_->GetWindow());
	World_ = std::make_shared<IVRWorld>(Engine_->GetDeviceManager());
	World_->Init(); //setting the world contents
	Engine_->SetWorld(World_);
	Engine_->PostWorldInit();
}

void IVRApp::Mainloop()
{
	while (!glfwWindowShouldClose(Engine_->GetWindow()->GetGLFWWindow()))
	{
		CurrentTime_ = std::chrono::high_resolution_clock::now();
		FrameTime_ = std::chrono::duration_cast<std::chrono::duration<float>>(CurrentTime_ - PreviousTime_).count();
		PreviousTime_ = CurrentTime_;

		InputManager_->PollInputs();
		World_->Update(FrameTime_);
		Engine_->DrawFrame();
	}
}


int main()
{
	IVRApp app;
	app.Mainloop();
	return 0;
}