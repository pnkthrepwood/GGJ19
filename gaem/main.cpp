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

const int MAP_HEIGHT = 20;
const int MAP_WIDTH =  39;

const int START_DINEROS = 1000;

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
	std::array<float,CURSOR_AMOUNT> anim_elapsed;
	std::array<unsigned short, CURSOR_AMOUNT> anim_state;
	std::array<sf::Color, CURSOR_AMOUNT> color;
	std::array<sf::Vector2f, CURSOR_AMOUNT> joy_left_before;
	std::array<float, CURSOR_AMOUNT> timer_joy_dir;
} cursor;

int dineros[CURSOR_AMOUNT];

enum class SpriteType
{
	IMPOSSIBLE,
	EMPTY,
	
	FARM,
	TREE_1,
	TREE_2,

	HOUSE_BEING_BUILT,
	HOUSE,

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

		case SpriteType::TREE_1:
		{
			spr.setTextureRect(sf::IntRect(0, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		} break;
		case SpriteType::TREE_2:
		{
			spr.setTextureRect(sf::IntRect(TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE));
		} break;

	}
}

void selectSprite(SpriteType type)
{
	selectSprite(type, spr_stamp);
}

SpriteType tileMap[MAP_WIDTH][MAP_HEIGHT];
int tileOwner[MAP_WIDTH][MAP_HEIGHT];
bool tilePassable[MAP_WIDTH][MAP_HEIGHT];
float tileBuildingTime[MAP_WIDTH][MAP_HEIGHT];

void InitTilePassable()
{

	auto image = background->copyToImage();
	sf::Color colorBase = image.getPixel(TILE_SIZE/2, TILE_SIZE/2);
	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			tilePassable[x][y] = colorBase == image.getPixel(x * TILE_SIZE + TILE_SIZE/2, y * TILE_SIZE + TILE_SIZE/2);
		}
	}
}

bool IsInsideMap(int x, int y) {
	return x >= 0 && x <= MAP_WIDTH && y >= 0 && y <= MAP_HEIGHT;
}

bool CanBeBuilt(SpriteType type)
{
	return
		type == SpriteType::EMPTY  ||
		type == SpriteType::TREE_1 ||
		type == SpriteType::TREE_2;
}

void draw_dineros(sf::RenderWindow& window, sf::Font &font, int dineros, sf::Color color, int x, int y) {

	sf::Text txt_money;
	txt_money.setFont(font);
	sf::String str;
	str = "$";
	int mm = dineros;
	for (int i = 0; i < 5; ++i)
	{
		str += std::to_string(mm % 10);
		mm = mm / 10;
	}
	std::reverse(str.begin(), str.end());
	txt_money.setString(str);
	txt_money.setPosition(x, y);
	txt_money.setFillColor(color);
	txt_money.setCharacterSize(38);
	window.draw(txt_money);
}

float RES_X = 1280.0f;
float RES_Y = 720.0f;

enum class EDirection {LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3, COUNT = 4};
const std::array<sf::Vector2i, 4> DIRECTION_OFFSETS = {{ {-1, 0}, {0, -1}, {1, 0}, {0, 1} }};


void InitJuego()
{
	InitTilePassable();
	
	for (int x = 0; x < MAP_WIDTH; ++x)
	{
		for (int y = 0; y < MAP_HEIGHT; ++y)
		{
			tileBuildingTime[x][y] = 0;
		}
	}
}

int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	ImGui::SFML::Init(window);

	sf::View view(sf::FloatRect(0.0f, -TILE_SIZE*2.5f, RES_X*0.5f, RES_Y*0.5f));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

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
		dineros[i] = START_DINEROS;
	}

	cursor.x[0] = 2;
	cursor.y[0] = 2;
	cursor.x[1] = 2;
	cursor.y[1] = MAP_HEIGHT - 2;
	cursor.x[2] = MAP_WIDTH - 2;
	cursor.y[2] = 2;
	cursor.x[3] = MAP_WIDTH -2;
	cursor.y[3] = MAP_HEIGHT-2;

	sf::Font font;
	font.loadFromFile("8bitmadness.ttf");

	InitJuego();




	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			tileOwner[x][y] = -1;
		}
	}

	for (int x = 0; x < MAP_WIDTH; x++)
	{
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			if (tilePassable[x][y])
			{
				SpriteType spriteType = SpriteType::EMPTY;
				if (std::rand() % 32 == 0) {
					spriteType = std::rand() % 3 == 0 ? SpriteType::TREE_2 : SpriteType::TREE_1;
				}

				tileMap[x][y] = spriteType;
			}
			else
			{
				tileMap[x][y] = SpriteType::IMPOSSIBLE;
			}
		}
	}

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

		//ImGui gives the position in window positions. (0, 0) is left corner.
		ImVec2 mpos = ImGui::GetMousePos();



		GamePad::UpdateInputState();

		//Update cursorsito en modo clack
		for (int i = 0; i < CURSOR_AMOUNT; ++i)
		{
			float& timer = cursor.timer_joy_dir[i];

			const float clack_first_timer = -0.1f;

			sf::Vector2f& joy_left_before = cursor.joy_left_before[i];
			sf::Vector2f joy_left = GamePad::AnalogStick::Left.get(i);
			if (joy_left.x < -50 && joy_left_before.x > -50)
			{
				timer = clack_first_timer;
				cursor.x[i]--;
			}
			if (joy_left.x > 50 && joy_left_before.x < 50)
			{
				timer = clack_first_timer;
				cursor.x[i]++;
			}
			if (joy_left.y < -50 && joy_left_before.y > -50)
			{
				timer = clack_first_timer;
				cursor.y[i]--;
			}
			if (joy_left.y > 50 && joy_left_before.y < 50)
			{
				timer = clack_first_timer;
				cursor.y[i]++;
			}
		}

		//Update cursorsito en modo dandole
		for (int i = 0; i < CURSOR_AMOUNT; ++i)
		{
			sf::Vector2f& joy_left_before = cursor.joy_left_before[i];
			sf::Vector2f joy_left = GamePad::AnalogStick::Left.get(i);

			float& timer = cursor.timer_joy_dir[i];
			const float walking_cooldown = 0.15f;

			if (joy_left.x < -50)
			{
				timer += dt_time.asSeconds();
				if (timer > walking_cooldown)
				{
					cursor.x[i]--;
					timer -= walking_cooldown;
					cursor.anim_elapsed[i] = 0;
					cursor.anim_elapsed[i] = 0;
				}
			}
			else if (joy_left.x > 50)
			{
				timer += dt_time.asSeconds();
				if (timer > walking_cooldown)
				{
					cursor.x[i]++;
					timer -= walking_cooldown;
					cursor.anim_elapsed[i] = 0;
				}
			}
			else if (joy_left.y < -50)
			{
				timer += dt_time.asSeconds();
				if (timer > walking_cooldown)
				{
					cursor.y[i]--;
					timer -= walking_cooldown;
					cursor.anim_elapsed[i] = 0;
				}
			}
			else if (joy_left.y > 50)
			{
				timer += dt_time.asSeconds();
				if (timer > walking_cooldown)
				{
					cursor.y[i]++;
					timer -= walking_cooldown;
					cursor.anim_elapsed[i] = 0;
				}
			}

			joy_left_before = joy_left;
		}


		// Comprar
		for (int i = 0; i < CURSOR_AMOUNT; ++i)
		{
			int x = cursor.x[i];
			int y = cursor.y[i];

			/*
			if (GamePad::IsButtonJustPressed(i, GamePad::Button::A)) 
			{
				if (tileMap[x][y] == SpriteType::HOUSE && tileOwner[x][y] == -1 && dineros[i] >= 250) 
				{
					tileOwner[x][y] = i;
					dineros[i] -= 250;
				}
			}
			*/

			if (GamePad::IsButtonJustPressed(i, GamePad::Button::X) && CanBeBuilt(tileMap[x][y]))
			{
				tileBuildingTime[x][y] = 0.0f;
			}

			if (GamePad::IsButtonPressed(i, GamePad::Button::X) && CanBeBuilt(tileMap[x][y]))
			{
				tileBuildingTime[x][y] += dt_time.asSeconds();
				if (tileBuildingTime[x][y] > 1.0f)
				{
					tileMap[x][y] = SpriteType::HOUSE;
					dineros[i] -= 250;
				}
			}

		}

		// Update bosquecico
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			for (int y = 0; y < MAP_HEIGHT; y++)
			{
				auto& tile = tileMap[x][y];
				const float FACTOR_SPEED = 1.f;
				const int FRAMES_TO_EVOLVE = 50000;
				const int FRAMES_TO_EXPLAND = 50000;
				if (tile == SpriteType::TREE_1)
				{ // Crecer el arbol
					if (std::rand()%static_cast<int>(FRAMES_TO_EVOLVE*FACTOR_SPEED) == 0)
					{
						tile = SpriteType::TREE_2;
					}
				}
				if (tile == SpriteType::TREE_2)
				{
					for (int i = 0; i < static_cast<int>(EDirection::COUNT); ++i)
					{
						auto offset = DIRECTION_OFFSETS[i];
						auto posOffset = sf::Vector2i(x,y) + offset;
						if (IsInsideMap(posOffset.x, posOffset.y))
						{
							auto& newTile = tileMap[posOffset.x][posOffset.y];
							if (newTile == SpriteType::EMPTY && std::rand() % static_cast<int>(FRAMES_TO_EXPLAND*FACTOR_SPEED) == 0)
							{
								newTile = SpriteType::TREE_1;
							}
						}
					}
				}
			}
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
				int owner = tileOwner[x][y];
				if (owner == -1) {
					spr_stamp.setColor(sf::Color::White);
				}
				else {
					spr_stamp.setColor(cursor.color[owner]);
				}
				window.draw(spr_stamp);
			}
		}


		for (int i = 0; i < CURSOR_AMOUNT; ++i)
		{
			{ // Cursorsito animation
				cursor.anim_elapsed[i] -= dt_time.asSeconds();
				if (cursor.anim_elapsed[i] < 0.f)
				{
					cursor.anim_elapsed[i] = 0.3f;
					cursor.anim_state[i] = (cursor.anim_state[i] + 1) % 2;
				}
				float scale = 1.f + 0.3f * cursor.anim_state[i];
				spr_cursor.setScale(scale, scale);
			}

			//Draw el cursorsito
			spr_cursor.setColor(cursor.color[i]);
			spr_cursor.setPosition(cursor.x[i] * TILE_SIZE + TILE_SIZE * 0.5f, cursor.y[i] * TILE_SIZE + TILE_SIZE * 0.5f);
			window.draw(spr_cursor);
		}

		window.setView(ui_view);
		draw_dineros(window, font, dineros[0], cursor.color[0], 10, 10);
		draw_dineros(window, font, dineros[1], cursor.color[1], 210, 10);
		draw_dineros(window, font, dineros[2], cursor.color[2], 410, 10);
		draw_dineros(window, font, dineros[3], cursor.color[3], 610, 10);
		window.setView(view);

		//ImGui::ShowDemoWindow();
		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}
