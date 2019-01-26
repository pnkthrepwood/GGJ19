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

sf::Texture* tex_spritesheet;

sf::Sprite spr_tile_dessert;
const int TILE_SIZE = 16;

int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	ImGui::SFML::Init(window);

	sf::View cam(sf::FloatRect(0.0f, 0.0f, RES_X*0.5f, RES_Y*0.5f));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	sf::Font font;
	font.loadFromFile("8bitmadness.ttf");

	tex_spritesheet = new sf::Texture();
	tex_spritesheet->loadFromFile("sprite_sheet.png");
	spr_tile_dessert.setTexture(*tex_spritesheet);
	spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));

	sf::Clock clk_running;
	sf::Clock clk_delta;
	while (window.isOpen())
	{
		window.setView(cam);


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

		window.setView(cam);

		//Draw fondo del Mapita
		int start_x = cam.getCenter().x - cam.getSize().x*0.5f - TILE_SIZE;
		int start_y = cam.getCenter().y - cam.getSize().y*0.5f - TILE_SIZE;

		int TILES_CAM_WIDTH = 100;
		int TILES_CAM_HEIGHT = 100;

		for (int x = start_x; x < start_x + TILES_CAM_WIDTH; ++x)
		{
			for (int y = start_y; y < start_y + TILES_CAM_HEIGHT; ++y)
			{
				spr_tile_dessert.setPosition(x*TILE_SIZE, y*TILE_SIZE);
				window.draw(spr_tile_dessert);
			}
		}



		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}
