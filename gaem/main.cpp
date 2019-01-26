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
#include <algorithm>

#include <unordered_set>
#include <unordered_map>
#include "rand.h"
#include "mates.h"
#include <array>

#include "ObjectManager.h"

#pragma warning( disable : 4244 )

using namespace std;

float RES_X = 1280.0f;
float RES_Y = 720.0f;

const int TILE_SIZE = 16;

const int NUM_PLAYERS = 4;
const float PLAYER_SPEED = 400;
const float BULLET_SPEED = 700;
const float BULLET_COOLDOWN = 0.5; //seconds
const float MADERA_GATHER_TIME = 3; //seconds

struct Player {
	float x, y;
	float vel_x, vel_y;
	int hp;
	sf::Vector2f facing_vector;
	float bullet_cooldown;
	float madera_progress;
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


void SpriteCenterOrigin(sf::Sprite& spr)
{
	spr.setOrigin(spr.getTexture()->getSize().x / 2.f, spr.getTexture()->getSize().y / 2.f);
}

struct Particle {
	float x, y;
	float vel_x, vel_y;
	sf::Sprite sprite;
	float life;

	Particle(sf::Texture& texture, float px,float py, float vx, float vy, float mlife)
		: x(px)
		, y(py)
		, vel_x(vx)
		, vel_y(vy)
		, life(mlife)
	{
		sprite.setTexture(texture);
		SpriteCenterOrigin(sprite);
	}

	bool Update(float dt)
	{
		x += vel_x * dt;
		y += vel_y * dt;
		life -= dt;
		if (life < 0) {
			return true;
		}
		sprite.setPosition(x, y);
		return false;
	}

};


struct ProgressShape : public sf::CircleShape
{
	int progress;

	ProgressShape() : sf::CircleShape(46, 120) { };
	sf::Vector2f getPoint(std::size_t index) const
	{
		if (index > progress) index = 0;
		if (index > progress / 2) index = progress - index;

		static const float pi = 3.141592654f;

		float angle = index * 2 * pi / this->getPointCount() * 2 - pi / 2;
		float x = std::cos(angle) * getRadius();
		float y = std::sin(angle) * getRadius();

		return sf::Vector2f(getRadius() + x, getRadius() + y);
	}

};

int madera = 0;

sf::Texture* tex_spritesheet;
sf::Sprite spr_tile_dessert;
sf::Sprite* spr_player[NUM_PLAYERS];

sf::Shader* nightLight;
sf::VertexArray quad(sf::Quads, 4);
sf::Clock clockDay;

std::array<Player, NUM_PLAYERS> players;
std::vector<Bullet*> bullets;
std::vector<Particle*> particles;

void InitPlayers() {
	for (int i = 0; i < NUM_PLAYERS; i++) {
		memset(&(players[i]), 0, sizeof(Player));
		players[i].hp = 100;
		players[i].x = RES_X / 2;
		players[i].y = RES_Y / 2;
		players[i].facing_vector = sf::Vector2f(1, 0);
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

sf::Texture* madera_texture;

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
	if (p->madera_progress > 0) {
		p->vel_x = p->vel_y = 0;
	} else {
		p->vel_x = direction.x * PLAYER_SPEED;
		p->vel_y = direction.y * PLAYER_SPEED;
	}
	// Update pos
	p->x = p->x + p->vel_x * dt;
	p->y = p->y + p->vel_y * dt;

	// Keep inside view
	p->x = Mates::Clamp(p->x, cam.getCenter().x - cam.getSize().x / 2 + 50, cam.getCenter().x + cam.getSize().x / 2 - 50);
	p->y = Mates::Clamp(p->y, cam.getCenter().y - cam.getSize().y / 2 + 50, cam.getCenter().y + cam.getSize().y / 2 - 50);

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
	if (p->bullet_cooldown > 0) {
		p->bullet_cooldown -= dt;
	}
	if (GamePad::IsButtonJustPressed(num_player, GamePad::Button::A) && p->bullet_cooldown <= 0 && p->madera_progress <= 0) {
		bullets.push_back(new Bullet(p->x, p->y, p->facing_vector, num_player));
		p->bullet_cooldown = BULLET_COOLDOWN;
	}


	// Gather wood
	// TODO: CHECK THERE IS A TREE
	if (GamePad::IsButtonJustReleased(num_player, GamePad::Button::B)) {
		p->madera_progress = 0;
	}
	if (GamePad::IsButtonPressed(num_player, GamePad::Button::B)) {
		p->madera_progress += dt;
		if (p->madera_progress >= MADERA_GATHER_TIME) {
			madera += 1;
			p->madera_progress = 0;
			particles.push_back(new Particle(*madera_texture, p->x, p->y - 50, 0, -200, 0.2f));
		}
	}

	if (p->hp < 100) {
		p->hp += dt;
		if (p->hp > 100) {
			p->hp = 100;
		}
	}
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

sf::Glsl::Vec3 pSetHSV(float h, float s, float v ) {
    	// H [0, 360] S and V [0.0, 1.0].
    	int i = (int)floor(h/60.0f) % 6;
    	float f = h/60.0f - floor(h/60.0f);
    	float p = v * (1.f - s);
    	float q = v * (1.f - s * f);
    	float t = v * (1.f - (1.f - f) * s);

    	switch (i) {
    		case 0: return sf::Glsl::Vec3(v, t, p);
    		break;
    		case 1: return sf::Glsl::Vec3(q, v, p);
    		break;
    		case 2: return sf::Glsl::Vec3(p, v, t);
    		break;
    		case 3: return sf::Glsl::Vec3(p, q, v);
    		break;
    		case 4: return sf::Glsl::Vec3(t, p, v);
    		break;
    		case 5: return sf::Glsl::Vec3(v, p, q);
    	}
        return sf::Glsl::Vec3(0.2f, 0.2f, 0.2f);
    }

void RenderWithShader(sf::RenderWindow& window, const sf::RenderTexture& renderTexture) {
	sf::RenderStates states;

	nightLight->setUniform("texture", sf::Shader::CurrentTexture);
	nightLight->setUniform("dayTime", (clockDay.getElapsedTime().asSeconds())/60);
	nightLight->setUniform("day_color", sf::Glsl::Vec3(1,1,1));
	nightLight->setUniform("sun_set_color", sf::Glsl::Vec3(1,0.5f,0));
	nightLight->setUniform("night_color", sf::Glsl::Vec3(0.2f,0,1));

	states.shader = nightLight;
	states.texture = &renderTexture.getTexture();
	window.draw(quad, states);
}

ObjManager obj_manager;


void DrawPlayer(int num_player, sf::RenderTarget& renderTexture)
{

	spr_player[num_player]->setPosition(players[num_player].x, players[num_player].y);
	renderTexture.draw(*spr_player[num_player]);

	Player* p = &players[num_player];
	static ProgressShape sprite;
	sprite.progress = sprite.getPointCount() - (sprite.getPointCount()*(p->madera_progress / MADERA_GATHER_TIME));
	if (sprite.progress < 120 && sprite.progress > 3) {
		sprite.setFillColor(sf::Color::Transparent);
		sprite.setOrigin(47, 47);
		sprite.setPosition(p->x, p->y);
		sprite.setScale(-1, 1);

		sprite.setOutlineThickness(11);
		sprite.setOutlineColor(sf::Color(20, 250, 20, 90));
		renderTexture.draw(sprite);
	}

	static sf::RectangleShape lifeBar;
	const float health_bar_width = 50;
	if (p->hp < 100)
	{
		lifeBar.setFillColor(sf::Color(250, 20, 20));
		lifeBar.setPosition(p->x - health_bar_width/2, p->y + 40);
		lifeBar.setSize(sf::Vector2f(p->hp / 100.f * health_bar_width, 5));
		renderTexture.draw(lifeBar);
	}

}

int main()
{
	srand(time(NULL));

	sf::ContextSettings settings;
	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!", sf::Style::Default, settings);
	sf::RenderTexture renderTexture;
	renderTexture.create(RES_X, RES_Y);

	sf::Color playerColors[] = {
		sf::Color::Cyan,
		sf::Color::Magenta,
		sf::Color::Red,
		sf::Color::Blue,
	};

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	sf::View cam(sf::FloatRect(0.0f, 0.0f, RES_X, RES_Y));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	sf::Font font;
	font.loadFromFile("8bitwonder.ttf");

	sf::Texture player_texture;
	player_texture.loadFromFile("player.png");
	for (int i = 0; i < NUM_PLAYERS; i++) {
		spr_player[i] = new sf::Sprite();
		spr_player[i]->setTexture(player_texture);
		SpriteCenterOrigin(*(spr_player[i]));
		spr_player[i]->setColor(playerColors[i]);
	}

	sf::Texture bullet_texture;
	bullet_texture.loadFromFile("bullet.png");
	sf::Sprite spr_bullet;
	spr_bullet.setTexture(bullet_texture);
	SpriteCenterOrigin(spr_bullet);

	madera_texture = new sf::Texture();
	madera_texture->loadFromFile("madera.png");
	sf::Sprite spr_madera;
	spr_madera.setTexture(*madera_texture);
	SpriteCenterOrigin(spr_madera);

	tex_spritesheet = new sf::Texture();
	tex_spritesheet->loadFromFile("sprite_sheet.png");
	spr_tile_dessert.setTexture(*tex_spritesheet);
	spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));

	InitNightShader(window);

	InitPlayers();

	obj_manager.Spawn(GameObjectType::CASA, 0, 0);

	obj_manager.Spawn(GameObjectType::TREE, 50, 50);

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
				bullets.erase(bullets.begin() + i);
			} else {
				i++;
			}
		}
		for (int i = 0; i < particles.size();) {
			bool cale_destruir = particles[i]->Update(dt_time.asSeconds());
			if (cale_destruir) {
				particles.erase(particles.begin() + i);
			}
			else {
				i++;
			}
		}

		//DRAW
		renderTexture.clear(sf::Color(255, 216, 0));


		{ // UpdateCamera(cam);
			sf::Vector2f centroid;
			for (int i = 0; i < NUM_PLAYERS; ++i) {
					centroid += sf::Vector2f(players[i].x, players[i].y);
			}
			centroid = centroid / static_cast<float>(NUM_PLAYERS);
			cam.setCenter(centroid);
		}


		ImGui::Begin("finester");
		ImGui::Text("Joy: %f, %f", 1337.f, 42.f);
		ImGui::End();

		renderTexture.setView(cam);

		//Draw fondo del Mapita
		float start_x = cam.getCenter().x - cam.getSize().x*0.5f - TILE_SIZE;
		start_x = (start_x / TILE_SIZE) * TILE_SIZE;
		float start_y = cam.getCenter().y - cam.getSize().y*0.5f - TILE_SIZE;
		start_y = (start_y / TILE_SIZE) * TILE_SIZE;

		int TILES_CAM_WIDTH = 100;
		int TILES_CAM_HEIGHT = 100;

		//Draw tilesitos del suelo
		for (int x = start_x; x < start_x + TILES_CAM_WIDTH*TILE_SIZE; x += TILE_SIZE)
		{
			for (int y = start_y; y < start_y + TILES_CAM_HEIGHT * TILE_SIZE; y += TILE_SIZE)
			{
				spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));
				spr_tile_dessert.setPosition(x, y);
				renderTexture.draw(spr_tile_dessert);
			}
		}

		//Draw objetesitos
		obj_manager.Draw(cam, renderTexture, spr_tile_dessert);

		
		//Draw Playersitos
		std::vector<pair<int, int> > draw_order;
		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			draw_order.push_back(make_pair(players[i].y, i));
		}
		std::sort(draw_order.begin(), draw_order.end());

		for (int j = 0; j < NUM_PLAYERS; j++) {
			int num_player = draw_order[j].second;
			DrawPlayer(num_player, renderTexture);
		}

		for (int i = 0; i < bullets.size(); i++) {
			spr_bullet.setPosition(bullets[i]->x, bullets[i]->y);
			spr_bullet.setColor(playerColors[bullets[i]->player]);
			renderTexture.draw(spr_bullet);
		}

		for (Particle* particle : particles) {

			renderTexture.draw(particle->sprite);
		}


		renderTexture.display();



		window.clear();
		RenderWithShader(window, renderTexture);
		ImGui::SFML::Render(window);

		sf::Text txt_money;
		txt_money.setFont(font);
		sf::String str = std::to_string((int)madera);
		txt_money.setString(str);
		txt_money.setFillColor(sf::Color::White);
		txt_money.setOutlineColor(sf::Color::Black);
		txt_money.setOutlineThickness(2);
		txt_money.setCharacterSize(32);
		txt_money.setPosition(40.f + spr_madera.getTexture()->getSize().x, 40.f);
		sf::FloatRect textRect = txt_money.getLocalBounds();
		txt_money.setOrigin(0, textRect.top + textRect.height / 2.0f);
		window.draw(txt_money);
		spr_madera.setPosition(40, 40);
		window.draw(spr_madera);

		window.display();

	}

	return 0;
}
