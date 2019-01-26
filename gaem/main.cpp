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

using namespace std;

const int MAP_HEIGHT = 64;
const int MAP_WIDTH = 64;

const int TILE_SIZE = 16;

sf::Texture* spriteSheet;
sf::Texture* background;

sf::Sprite spr_background;
sf::Sprite spr_stamp;
sf::Sprite spr_cursor;


enum SpriteType
{
	IMPOSSIBLE,
	EMPTY,
	HOUSE,
	FARM,

	CURSOR
};

void selectSprite(SpriteType type, sf::Sprite& spr)
{
	switch (type)
	{
		case SpriteType::HOUSE:
		{
			spr.setTextureRect(sf::IntRect(0, 0, TILE_SIZE, TILE_SIZE));
		} break;

		case SpriteType::FARM:
		{
			//spr_stamp.setTextureRect(sf::IntRect(0, 0, 16, 16));
		} break;

		case SpriteType::EMPTY:
		{
			spr.setTextureRect(sf::IntRect(0, 1* TILE_SIZE, TILE_SIZE, TILE_SIZE));
		} break;

		case SpriteType::IMPOSSIBLE:
		{
			spr.setTextureRect(sf::IntRect(0, 1 * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		} break;

		case SpriteType::CURSOR:
		{
			spr.setTextureRect(sf::IntRect(0, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		} break;

	}
}

void selectSprite(SpriteType type)
{
	selectSprite(type, spr_stamp);
}

SpriteType tileMap[MAP_WIDTH][MAP_HEIGHT];
bool tilePassable[MAP_WIDTH][MAP_HEIGHT];

float RES_X = 1280.0f;
float RES_Y = 720.0f;

int main()
{
	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	ImGui::SFML::Init(window);

	sf::View view(sf::FloatRect(0.0f, 0.0f, RES_X*0.5f, RES_Y*0.5f));
	window.setView(view);

	spriteSheet = new sf::Texture();
	spriteSheet->loadFromFile("sprite_sheet.png");
	spr_stamp.setTexture(*spriteSheet);

	background = new sf::Texture();
	background->loadFromFile("background.png");
	spr_background.setTexture(*background);

	spr_cursor.setTexture(*spriteSheet);

	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			//if (tileMap[x][y] != TileType::IMPOSSIBLE)
			{
				tileMap[x][y] = (std::rand() % 12 == 0) ? SpriteType::HOUSE : SpriteType::EMPTY;
			}
		}
	}

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

		

		//Draw el fondo
		spr_background.setTexture(*background);
		spr_background.setPosition(0, 0);
		window.draw(spr_background);


		//Draw el mapita
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			for (int y = 0; y < MAP_HEIGHT; y++)
			{
				selectSprite(tileMap[x][y]);
				spr_stamp.setPosition(x*TILE_SIZE, y*TILE_SIZE);
				window.draw(spr_stamp);
			}
		}


		static int cursor_x = 0;
		static int cursor_y = 0;
		//Draw el cursorsito
		selectSprite(SpriteType::CURSOR, spr_cursor);
		spr_cursor.setColor(sf::Color(255, 0, 0, 255));
		spr_cursor.setPosition(cursor_x * TILE_SIZE, cursor_y * TILE_SIZE);
		window.draw(spr_cursor);
		

		//ImGui::ShowDemoWindow();
		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}

