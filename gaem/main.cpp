#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "imgui.h"
#include "imgui-SFML.h"
#include <string>
#include "InputManager.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "rand.h"
#include <array>

using namespace std;


float RES_X = 1280.0f;
float RES_Y = 720.0f;



int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	ImGui::SFML::Init(window);

	sf::View view(sf::FloatRect(0.0f, 0.0f, RES_X*0.5f, RES_Y*0.5f));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	sf::Font font;
	font.loadFromFile("8bitmadness.ttf");


	sf::Clock clk_running;
	sf::Clock clk_delta;
	while (window.isOpen())
	{
		window.setView(view);


		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

		}

		sf::Time dt_time = clk_delta.restart();
		ImGui::SFML::Update(window, dt_time);


		GamePad::UpdateInputState();


		window.clear();


		window.setView(ui_view);
		//Todo UI

		window.setView(view);

		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}
