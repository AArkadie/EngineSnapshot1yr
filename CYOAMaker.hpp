#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace SAGE {

	class TextModule {
		bool gameOver;
		std::string data, shorthand;
		std::vector<TextModule*> choices{};

	public:
		TextModule(std::string sHand, std::string dat);
		//~TextModule() = default;

		void addModule(TextModule& texMod);

		bool display();

		TextModule* choose(short choice);
	};

	void CYOAload(std::string file, std::vector<TextModule>& modules);
}