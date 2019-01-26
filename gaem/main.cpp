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

const int MAP_HEIGHT = 64;
const int MAP_WIDTH = 64;

const int TILE_SIZE = 16;

sf::Texture* spriteSheet;
sf::Texture* background;

sf::Sprite spr_background;
sf::Sprite spr_stamp;
sf::Sprite spr_cursor;


const int CURSOR_AMOUNT = 4;
struct {
	std::array<int,CURSOR_AMOUNT> x;
	std::array<int,CURSOR_AMOUNT> y;
	std::array<float,CURSOR_AMOUNT> elapsed;
	std::array<unsigned short, CURSOR_AMOUNT> icon_id;
	std::array<sf::Color, CURSOR_AMOUNT> color;
	std::array<sf::Vector2f, CURSOR_AMOUNT> joy_left_before;
} cursor;

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
	selectSprite(SpriteType::CURSOR, spr_cursor);
	spr_cursor.setOrigin(TILE_SIZE/2.f, TILE_SIZE/2.f);

	cursor.color[0] = sf::Color::Cyan;
	cursor.color[1] = sf::Color::Magenta;
	cursor.color[2] = sf::Color::Red;
	cursor.color[3] = sf::Color::Blue;

	for (int i = 0; i < CURSOR_AMOUNT; ++i) {
		cursor.x[i] = cursor.y[i] = i * 2;
	}

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



		GamePad::UpdateInputState();


		for (int i = 0; i < CURSOR_AMOUNT; ++i)
		{
			sf::Vector2f& joy_left_before = cursor.joy_left_before[i];
			sf::Vector2f joy_left = GamePad::AnalogStick::Left.get(i);
			if (joy_left.x < -50 && joy_left_before.x > -50)
			{
				cursor.x[i]--;
			}
			if (joy_left.x > 50 && joy_left_before.x < 50)
			{
				cursor.x[i]++;
			}
			if (joy_left.y < -50 && joy_left_before.y > -50)
			{
				cursor.y[i]--;
			}
			if (joy_left.y > 50 && joy_left_before.y < 50)
			{
				cursor.y[i]++;
			}
			joy_left_before = joy_left;
		}

		

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




		for (int i = 0; i < CURSOR_AMOUNT; ++i) {
			{ // Cursorsito animation
				cursor.elapsed[i] -= dt_time.asSeconds();
				if (cursor.elapsed[i] < 0.f)
				{
					cursor.elapsed[i] = 0.3f;
					cursor.icon_id[i] = (cursor.icon_id[i] + 1) % 2;
				}
				float scale = 1.f + 0.3f * cursor.icon_id[i];
				spr_cursor.setScale(scale, scale);
			}

			//Draw el cursorsito
			spr_cursor.setColor(cursor.color[i]);
			spr_cursor.setPosition(cursor.x[i] * TILE_SIZE + TILE_SIZE * 0.5f, cursor.y[i] * TILE_SIZE + TILE_SIZE * 0.5f);
			window.draw(spr_cursor);
		}



		//ImGui::ShowDemoWindow();
		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}
