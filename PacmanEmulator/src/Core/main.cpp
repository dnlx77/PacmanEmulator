#include "Core/PacmanEmulator.h"
#include "Video/VideoController.h"
#include <iostream>

int main() {
	PacmanEmulator emulator;

	if (!emulator.Initialize()) {
		std::cerr << "Failed to initialize\n";
		return 1;
	}

	if (!emulator.LoadRomSet("assets")) {
		std::cerr << "Failed to load romset\n";
		return 1;
	}

	VideoController video(emulator.GetMemory());
	video.RenderFrame();
	video.SaveFramebufferPPM("framebuffer.ppm");

	std::cout << "Done! Check framebuffer.ppm\n";

	emulator.Run();
	return 0;
}