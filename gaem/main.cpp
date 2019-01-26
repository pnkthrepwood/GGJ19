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
#include "mates.h"
#include <array>

using namespace std;

float RES_X = 1280.0f;
float RES_Y = 720.0f;

const int TILE_SIZE = 16;

const int NUM_PLAYERS = 4;
const float PLAYER_SPEED = 500;
const float BULLET_SPEED = 700;

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
	sf::Vector2f facing_vector;
};

struct Bullet {
	float x, y;
	float vel_x, vel_y;
	int player;

	Bullet(float px, float py, sf::Vector2f facing_vector, int num_player)
		: x(px)
		, y(py)
		, vel_x(facing_vector.x*BULLET_SPEED) 
		, vel_y(facing_vector.y*BULLET_SPEED)
		, player(num_player)
	{ }
};


sf::Texture* tex_spritesheet;
sf::Sprite spr_tile_dessert;


sf::Shader* nightLight;
sf::VertexArray quad(sf::Quads, 4);
sf::Clock clockDay;

std::array<Player, NUM_PLAYERS> players;
std::vector<Bullet*> bullets;

void InitPlayers() {
	for (int i = 0; i < NUM_PLAYERS; i++) {
		memset(&(players[i]), 0, sizeof(Player));
		players[i].hp = 100;
		players[i].x = RES_X / 2;
		players[i].y = RES_Y / 2;
	}
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

bool UpdateBullet(Bullet *b, float dt, sf::View& cam)
{

	b->x += b->vel_x * dt;
	b->y += b->vel_y * dt;

	//Out of view
	if (b->x > cam.getCenter().x + cam.getSize().x / 2 ||
		b->x < cam.getCenter().x - cam.getSize().x / 2 ||
		b->y > cam.getCenter().y + cam.getSize().y / 2 ||
		b->y < cam.getCenter().y - cam.getSize().y / 2) {
		return true;
	}

	//TODO: Collisions with enemies


	return false;
	
}

void UpdatePlayer(float dt, int num_player, sf::View& cam)
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
	sf::Vector2f direction = Mates::Normalize(sf::Vector2f(stick_L.x, stick_L.y));
	p->vel_x = direction.x * PLAYER_SPEED;
	p->vel_y = direction.y * PLAYER_SPEED;

	// Update pos
	p->x = p->x + p->vel_x * dt;
	p->y = p->y + p->vel_y * dt;

	// Keep inside view
	p->x = Mates::Clamp(p->x, cam.getCenter().x - cam.getSize().x / 2, cam.getCenter().x + cam.getSize().x / 2);
	p->y = Mates::Clamp(p->y, cam.getCenter().y - cam.getSize().y / 2, cam.getCenter().y + cam.getSize().y / 2);

	// Update facing vector
	sf::Vector2f stick_R = GamePad::AnalogStick::Left.get(num_player);
	float length_R = Mates::Length(stick_L);
	if (length_R > 30)
	{
		p->facing_vector = stick_R;
	}
	else if (length_L > 30)
	{
		p->facing_vector = stick_R;
	}
	p->facing_vector = Mates::Normalize(sf::Vector2f(p->facing_vector.x, p->facing_vector.y));

	//Shot
	if (GamePad::IsButtonJustPressed(num_player, GamePad::Button::A)) {
		bullets.push_back(new Bullet(p->x, p->y, p->facing_vector, num_player));
	}


	// Gather wood
	if (GamePad::IsButtonPressed(num_player, GamePad::Button::B)) {

	}
}

void SpriteCenterOrigin(sf::Sprite& spr)
{
	spr.setOrigin(spr.getTexture()->getSize().x / 2.f, spr.getTexture()->getSize().y / 2.f);
}


void InitNightShader(sf::RenderTarget& renderTarget) {
	nightLight = new sf::Shader();
	nightLight->loadFromFile("nightLight.frag", sf::Shader::Type::Fragment);

	auto size = renderTarget.getSize();
	quad[0].position = sf::Vector2f(0,0);
	quad[1].position = sf::Vector2f(0,size.y);
	quad[2].position = sf::Vector2f(size.x,size.y);
	quad[3].position = sf::Vector2f(size.x,0);

	quad[0].texCoords = sf::Vector2f(0,0);
	quad[1].texCoords = sf::Vector2f(0,size.y);
	quad[2].texCoords = sf::Vector2f(size.x,size.y);
	quad[3].texCoords = sf::Vector2f(size.x,0);
}

void RenderWithShader(sf::RenderWindow& window, const sf::RenderTexture& renderTexture) {
	sf::RenderStates states;
	window.clear();
	nightLight->setUniform("texture", sf::Shader::CurrentTexture);
	nightLight->setUniform("dayTime", (clockDay.getElapsedTime().asSeconds())/60.f);
	nightLight->setUniform("texture", sf::Shader::CurrentTexture);
	states.shader = nightLight;
	states.texture = &renderTexture.getTexture();
	window.draw(quad, states);
	window.display();
}

int main()
{
	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!");
	sf::RenderTexture renderTexture;
	renderTexture.create(RES_X, RES_Y);

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	sf::View cam(sf::FloatRect(0.0f, 0.0f, RES_X, RES_Y));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	sf::Font font;
	font.loadFromFile("8bitmadness.ttf");

	sf::Texture player_texture;
	player_texture.loadFromFile("player.png");
	sf::Sprite spr_player[NUM_PLAYERS];
	for (int i = 0; i < NUM_PLAYERS; i++) {
		spr_player[i].setTexture(player_texture);
		SpriteCenterOrigin(spr_player[i]);
		//spr_player[i].setColor(playerColors[i]);
	}

	sf::Texture bullet_texture;
	bullet_texture.loadFromFile("bullet.png");
	sf::Sprite spr_bullet;
	spr_bullet.setTexture(bullet_texture);
	SpriteCenterOrigin(spr_bullet);

	tex_spritesheet = new sf::Texture();
	tex_spritesheet->loadFromFile("sprite_sheet.png");
	spr_tile_dessert.setTexture(*tex_spritesheet);
	spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));

	InitNightShader(window);

	InitPlayers();

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

		GamePad::UpdateInputState();


		//UPDATE
		for (int i = 0; i < NUM_PLAYERS; i++) {
			UpdatePlayer(dt_time.asSeconds(), i, cam);
		}
		for (int i = 0; i < bullets.size();) {
			bool cale_destruir = UpdateBullet(bullets[i], dt_time.asSeconds(), cam);
			if (cale_destruir) {
				cout << "DESTRUR" << endl;
				bullets.erase(bullets.begin() + i);
			} else {
				i++;
			}
		}




		//DRAW
		//window.clear();
		renderTexture.setView(cam);
		renderTexture.clear(sf::Color(255, 216, 0));


		static sf::Vector2f joy = GamePad::AnalogStick::Left.get(0);
		joy = GamePad::AnalogStick::Left.get(0);
		sf::Vector2f cam_offset(0, 0);
		if (joy.x < -50) cam_offset.x = -100 * dt_time.asSeconds();
		else if (joy.x > 50) cam_offset.x = 100 * dt_time.asSeconds();
		if (joy.y < -50) cam_offset.y = -100 * dt_time.asSeconds();
		else if (joy.y > 50) cam_offset.y = 100 * dt_time.asSeconds();
		cam.move(cam_offset);


		ImGui::Begin("finester");

		ImGui::Text("Joy: %f, %f", joy.x, joy.y);
		ImGui::Text("Cam offset: %f, %f", cam_offset.x, cam_offset.y);

		ImGui::End();

		renderTexture.setView(cam);

		//Draw fondo del Mapita
		int start_x = cam.getCenter().x - cam.getSize().x*0.5f - TILE_SIZE;
		start_x = (start_x / TILE_SIZE) * TILE_SIZE;
		int start_y = cam.getCenter().y - cam.getSize().y*0.5f - TILE_SIZE;
		start_y = (start_y / TILE_SIZE) * TILE_SIZE;

		int TILES_CAM_WIDTH = 100;
		int TILES_CAM_HEIGHT = 100;

		for (int x = start_x; x < start_x + TILES_CAM_WIDTH*TILE_SIZE; x += TILE_SIZE)
		{
			for (int y = start_y; y < start_y + TILES_CAM_HEIGHT * TILE_SIZE; y += TILE_SIZE)
			{
				spr_tile_dessert.setPosition(x, y);
				renderTexture.draw(spr_tile_dessert);
			}
		}

		for (int i = 0; i < NUM_PLAYERS; i++) {
			spr_player[i].setPosition(players[i].x, players[i].y);
			renderTexture.draw(spr_player[i]);
		}

		for (int i = 0; i < bullets.size(); i++) {
			spr_bullet.setPosition(bullets[i]->x, bullets[i]->y);
			//spr_bullet.setColor(playerColors[bullets[i]->player]);
			renderTexture.draw(spr_bullet);
		}

		renderTexture.display();

		ImGui::SFML::Render(window);

		RenderWithShader(window, renderTexture);

	}

	return 0;
}
