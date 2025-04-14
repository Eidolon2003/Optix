#include "game.h"
#include "ini.h"

int main() {
	Options config;
	if (read_ini(&config, "config.ini")) {
		load_defaults(&config);
	}
	else {
		check_bounds(&config);
	}
	write_ini(&config, "config.ini");

	jaw::EngineProperties ep;
	ep.showCMD = false;
	jaw::AppProperties ap;
	ap.size = jaw::Point(config["width"], config["height"]);
	ap.framerate = (float)config["framerate"];
	ap.mode = config["fullscreen"] ?
		jaw::AppProperties::WINDOWED_FULLSCREEN :
		jaw::AppProperties::WINDOWED;
	ap.layerCount = 3;
	ap.backgroundCount = 1;
	ap.enableSubpixelTextRendering = true;
	ap.title = "Optix";

	jaw::StartEngine(new Game, ap, ep);
}