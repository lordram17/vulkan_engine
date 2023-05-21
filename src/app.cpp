#include "app.h"

IVRApp::IVRApp()
{
	Engine_ = std::make_shared<IVREngine>();
	InputManager_ = std::make_shared<IVRInputManager>(Engine_->GetWindow());
	World_ = std::make_shared<IVRWorld>(Engine_->GetDeviceManager(), Engine_->GetSwapchainManager()->GetImageViewCount());
	World_->Init(); //setting the world contents
	Engine_->SetWorld(World_);
	Engine_->PostWorldInit();
	PreviousTime_ = std::chrono::high_resolution_clock::now();
}

void IVRApp::Mainloop()
{
	while (!glfwWindowShouldClose(Engine_->GetWindow()->GetGLFWWindow()))
	{
		CurrentTime_ = std::chrono::high_resolution_clock::now();
		FrameTime_ = std::chrono::duration<float>(CurrentTime_ - PreviousTime_).count();
		PreviousTime_ = CurrentTime_;

		uint32_t current_swapchain_index = Engine_->QueryForSwapchainIndex();
		InputManager_->PollInputs();
		World_->Update(FrameTime_, current_swapchain_index);
		Engine_->DrawFrame();
	}
}


int main()
{
	IVRApp app;
	app.Mainloop();
	return 0;
}