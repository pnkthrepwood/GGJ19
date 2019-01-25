// Haxoria.cpp : Defines the entry point for the console application.
//

#include "imgui.h"
#include "imgui-SFML.h"

#include "FUN_Random.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <SFML/Graphics.hpp>

using namespace std;


int main()
{
	sf::RenderWindow window(sf::VideoMode(1024, 720), "SFML works!");
	ImGui::SFML::Init(window);

	Fun::Random.init();

	sf::Clock clk_running;
	sf::Clock clk_delta;
	while (window.isOpen())
	{
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

		//ImGui gives the position in window positions. (0, 0) is left corner.
		ImVec2 mpos = ImGui::GetMousePos();

		window.clear();
		ImGui::ShowDemoWindow();
		ImGui::SFML::Render(window);
		
		window.display();
	}

    return 0;
}

