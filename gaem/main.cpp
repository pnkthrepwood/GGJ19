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
#include "math.h"
#include <array>

using namespace std;

float RES_X = 1280.0f;
float RES_Y = 720.0f;

const int NUM_PLAYERS = 4;
const float PLAYER_SPEED = 500;

sf::Color playerColors[] = {
	sf::Color::Cyan,
	sf::Color::Magenta,
	sf::Color::Red,
	sf::Color::Blue,
};

enum PlayerState {
	IDLE = 0,
	WALKING,
	GATHERING,
};

struct Player {
	float x, y;
	float vel_x, vel_y;
	int hp;
	PlayerState state;
};


std::array<Player, NUM_PLAYERS> players;

void InitPlayers() {
	for (int i = 0; i < NUM_PLAYERS; i++) {
		memset(&(players[i]), 0, sizeof(Player));
		players[i].hp = 100;
		players[i].x = RES_X / 2;
		players[i].y = RES_Y / 2;
	}
	std::cout << players[0].x << std::endl;
	const int PLAYER_INITIAL_POS_OFFSET = 90;
	players[0].x -= PLAYER_INITIAL_POS_OFFSET / 2;
	players[0].y -= PLAYER_INITIAL_POS_OFFSET;
	players[1].x -= PLAYER_INITIAL_POS_OFFSET;
	players[1].y += PLAYER_INITIAL_POS_OFFSET / 2;
	players[2].x += PLAYER_INITIAL_POS_OFFSET;
	players[2].y -= PLAYER_INITIAL_POS_OFFSET / 2;
	players[3].x += PLAYER_INITIAL_POS_OFFSET / 2;
	players[3].y += PLAYER_INITIAL_POS_OFFSET;
}

void UpdatePlayer(float dt, int num_player)
{
	Player* p = &(players[num_player]);

	sf::Vector2f stick_L = GamePad::AnalogStick::Left.get(num_player);
	float length_L = Mates::Length(stick_L);

	// Dead zone
	if (length_L < 30)
	{
		stick_L = sf::Vector2f(0, 0);
	}

	// Update speed
	sf::Vector2f stick = Mates::Normalize(sf::Vector2f(stick_L.x, stick_L.y));
	p->vel_x = stick.x * PLAYER_SPEED;
	p->vel_y = stick.y * PLAYER_SPEED;

	// Update pos
	p->x = p->x + p->vel_x * dt;
	p->y = p->y + p->vel_y * dt;

	//Out of camera (TODO: Use camera instead of res)
	p->x = Mates::Clamp(p->x, 0, RES_X);
	p->y = Mates::Clamp(p->y, 0, RES_Y);

}

void SpriteCenterOrigin(sf::Sprite& spr)
{
	spr.setOrigin(spr.getTexture()->getSize().x / 2.f, spr.getTexture()->getSize().y / 2.f);
}

int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	ImGui::SFML::Init(window);

	sf::View view(sf::FloatRect(0.0f, 0.0f, RES_X, RES_Y));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	sf::Font font;
	font.loadFromFile("8bitmadness.ttf");

	sf::Texture texture;
	texture.loadFromFile("player.png");
	sf::Sprite spr_player[NUM_PLAYERS];
	for (int i = 0; i < NUM_PLAYERS; i++) {
		spr_player[i].setTexture(texture);
		SpriteCenterOrigin(spr_player[i]);
		//spr_player[i].setColor(playerColors[i]);
	}

	InitPlayers();
	
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



		//UPDATE
		for (int i = 0; i < NUM_PLAYERS; i++) {
			UpdatePlayer(dt_time.asSeconds(), i);
		}

		
		
		
		//DRAW
		window.clear();

		for (int i = 0; i < NUM_PLAYERS; i++) {
			spr_player[i].setPosition(players[i].x, players[i].y);
			window.draw(spr_player[i]);
		}


		ImGui::Begin("ASDASD");
		ImGui::Text("%f", players[0].y);
		ImGui::End();

		//window.setView(ui_view);
		//Todo UI

		//window.setView(view);

		ImGui::SFML::Render(window);

		window.display();
	}

	return 0;
}
