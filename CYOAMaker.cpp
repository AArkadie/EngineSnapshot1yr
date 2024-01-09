#include "CYOAMaker.hpp"

namespace SAGE {

	TextModule::TextModule(std::string sHand, std::string dat) {

		data = dat;
		shorthand = sHand;
		gameOver = true;
	}

	void TextModule::addModule(TextModule& texMod) {
		gameOver = false;
		choices.push_back(&texMod);
	}

	bool TextModule::display() {
		std::cout << data << '\n';
		if (!gameOver) {
			for (int i = 1; i <= choices.size(); i++) {
				std::cout << i << ". " << choices[i - 1]->shorthand << '\n';
			}
		}
		return !gameOver;
	}

	TextModule* TextModule::choose(short choice) {
		if (choice <= 0 || choice > choices.size()) return /*this;/*/new TextModule("Game Over", "You tried defying the hand of fate and it slapped you into oblivion.  Good Job.");
		return choices[choice - 1];
		//for (TextModule t : choices) if (&t != currentModule) delete &t;
		//delete this;
	}

	void CYOAload(std::string file, std::vector<TextModule>& modules) {
		std::ifstream adventure(file);
		std::string currentLine, currentLens;
		short index;

		while (std::getline(adventure, currentLine)) {
			if (currentLine[0] == '{') {
				index = currentLine.find('}');
				currentLens = currentLine.substr(1, index - 1);
				currentLine = currentLine.substr(index + 1);
				TextModule t = TextModule(currentLens, currentLine);
				modules.push_back(t);
			}
			else {
				bool done = false;
				while (!done) {
					switch (currentLine[0]) {
					case '[':
						currentLens.clear();
						index = std::stoi(currentLine.substr(1, currentLine.find(',')));
						currentLine = currentLine.substr(currentLine.find(','));
						break;
					case ',':
						modules[index].addModule(modules[std::stoi(currentLens)]);
						currentLens.clear();
						break;
					case ';':
						modules[index].addModule(modules[std::stoi(currentLens)]);
						currentLens.clear();
						index = std::stoi(currentLine.substr(1, currentLine.find(',')));
						currentLine = currentLine.substr(currentLine.find(','));
						break;
					case ']':
						modules[index].addModule(modules[std::stoi(currentLens)]);
						done = true;
						break;
					default:
						currentLens += currentLine[0];
					}
					currentLine = currentLine.substr(1);
				}
			}
		}
		adventure.close();
	}
}