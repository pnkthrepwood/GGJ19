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
#include "DayManager.h"

#pragma warning( disable : 4244 )

using namespace std;

float RES_X = 1280.0f;
float RES_Y = 720.0f;

const int TILE_SIZE = 16;

const int NUM_PLAYERS = 1;
const float PLAYER_SPEED = 300;

const float BULLET_SPEED = 700;
const float ENEMY_TRIGGER_DISTANCE = 400;
const float ENEMY_ACCEL = 2000;
const float ENEMY_MAX_SPEED = 700;
const float BULLET_COOLDOWN = 0.3; //seconds
const float MADERA_GATHER_TIME = 3; //seconds

ObjManager obj_manager;
int madera;

sf::Texture* tex_spritesheet;
sf::Texture* player_texture;
sf::Texture* madera_texture;
sf::Texture* enemy_texture;
sf::Texture* bullet_texture;

sf::Shader* nightLight;
sf::VertexArray quad(sf::Quads, 4);
sf::Clock clockDay;

sf::Color playerColors[4];

void SpriteCenterOrigin(sf::Sprite& spr)
{
	spr.setOrigin(spr.getTexture()->getSize().x / 2.f, spr.getTexture()->getSize().y / 2.f);
}

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

enum FacingDirection
{
	UP, DOWN, LEFT, RIGHT
};

enum PlayerState
{
	IDLE,
	WALKING,
	GATHERING,
};

struct Player
{
	float x, y;
	float vel_x, vel_y;
	int hp;
	sf::Vector2f facing_vector;
	float bullet_cooldown;
	float madera_progress;
	int num_player;

	//Anim stuff
	FacingDirection facing;
	PlayerState state;
	float anim_timer;

	sf::Sprite sprite;
	ProgressShape progress;
	sf::RectangleShape lifeBar;

	Player(int n_player) 
		: num_player(n_player) 
		, hp(100)
		, vel_x(0)
		, vel_y(0)
		, madera_progress(0)
		, bullet_cooldown(0)
		, anim_timer(0.f)
		, facing_vector(1, 0)
	{
		sprite.setTexture(*player_texture);
		sprite.setOrigin(16, 32);
		//sprite.setColor(playerColors[n_player]);

		progress.setFillColor(sf::Color::Transparent);
		progress.setOrigin(47, 47);
		progress.setScale(-1, 1);
		progress.setOutlineThickness(11);
		progress.setOutlineColor(sf::Color(20, 250, 20, 90));
		lifeBar.setFillColor(sf::Color(250, 20, 20));

		const int PLAYER_INITIAL_POS_OFFSET = 90;
		x = RES_X / 2;
		y = RES_Y / 2;
		switch (n_player) {
			case 0:
				x -= PLAYER_INITIAL_POS_OFFSET / 2;
				y -= PLAYER_INITIAL_POS_OFFSET;
				break;
			case 1:
				x -= PLAYER_INITIAL_POS_OFFSET;
				y += PLAYER_INITIAL_POS_OFFSET / 2;
				break;
			case 2:
				x += PLAYER_INITIAL_POS_OFFSET;
				y -= PLAYER_INITIAL_POS_OFFSET / 2;
				break;
			case 3:
				x += PLAYER_INITIAL_POS_OFFSET / 2;
				y += PLAYER_INITIAL_POS_OFFSET;
		}
	}

	sf::FloatRect boundBox()
	{
		return sf::FloatRect(x - sprite.getTexture()->getSize().x/2, 
			y - sprite.getTexture()->getSize().y/2,
			sprite.getTexture()->getSize().x,
			sprite.getTexture()->getSize().y);
	}

	void Draw(std::vector<sf::Sprite>& toDraw)
	{
		sprite.setPosition(x, y);

		sf::IntRect texrect;

		if (facing == FacingDirection::DOWN)
		{
			texrect = (sf::IntRect(0, 0, 11, 16));
		}
		if (facing == FacingDirection::RIGHT)
		{
			texrect = (sf::IntRect(0, 16*1, 11, 16));
		}
		if (facing == FacingDirection::LEFT)
		{
			texrect = (sf::IntRect(0, 16*2, 11, 16));
		}
		if (facing == FacingDirection::UP)
		{
			texrect = (sf::IntRect(0, 16*3, 11, 16));
		}

		if (state == PlayerState::IDLE)
		{
			texrect.left = 0;
		}
		else if (state == PlayerState::WALKING)
		{
			texrect.left += (static_cast<int>(anim_timer / 0.1f) % 3) * 10;
		}

		sprite.setTextureRect(texrect);
		toDraw.push_back(sprite);
	}

	void DrawUI(sf::RenderTarget& rt)
	{
		progress.setOutlineThickness(11); //Force redraw shape
		progress.progress = progress.getPointCount() - (progress.getPointCount()*(madera_progress / MADERA_GATHER_TIME));
		if (progress.progress < 120 && progress.progress > 3) 
		{
			progress.setPosition(x, y);
			rt.draw(progress);
		}

		const float health_bar_width = 50;
		if (hp < 100)
		{
			lifeBar.setPosition(x - health_bar_width / 2, y + 40);
			lifeBar.setSize(sf::Vector2f(hp / 100.f * health_bar_width, 5));
			rt.draw(lifeBar);
		}

	}


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

std::array<Player*, NUM_PLAYERS> players;
std::vector<Bullet*> bullets;
std::vector<Particle*> particles;



sf::FloatRect getBoundBoxSprite(sf::Sprite* sprite)
{
	sf::IntRect size = sprite->getTextureRect();
	sf::FloatRect fr;

	fr.left = sprite->getPosition().x - size.left*0.5f;
	fr.width = size.width;
	fr.top = sprite->getPosition().y - size.top*0.5f;
	fr.height = size.width;

	return fr;
}


struct Enemy 
{
	float x, y;
	float vel_x, vel_y;
	int hp = 100;
	PlayerState state; //IDLE OR WALKING ONLY
	float anim_timer;

	sf::Sprite sprite;

	Enemy(float px, float py) : x(px), y(py), vel_x(0), vel_y(0), state(IDLE), anim_timer(0.f) {
		sprite.setTexture(*enemy_texture);
		//TODO: CENTER texture rect and stuff
	}

	bool Update(float dt) {
		Player* closestPlayer = nullptr;
		float closestDist = 9999999999.f;
		for (Player *p : players) {
			float d = Mates::Distance(sf::Vector2f(p->x, p->y), sf::Vector2f(x, y));
			if (d < closestDist) {
				closestDist = d;
				closestPlayer = p;
			}
		}
		if (closestDist < ENEMY_TRIGGER_DISTANCE) {
			state = WALKING;
			anim_timer += dt;;
			sf::Vector2f dir(closestPlayer->x - x, closestPlayer->y - y);
			sf::Vector2f dir_bona = Mates::Normalize(dir);
			
			vel_x += dir_bona.x * ENEMY_ACCEL * dt;
			vel_y += dir_bona.y * ENEMY_ACCEL * dt;
			
			float lenght = Mates::Length(sf::Vector2f(vel_x, vel_y));
			if (lenght > ENEMY_MAX_SPEED) {
				vel_x = dir_bona.x * ENEMY_MAX_SPEED;
				vel_y = dir_bona.y * ENEMY_MAX_SPEED;
			}

			x += vel_x * dt;
			y += vel_y * dt;

			sprite.setPosition(x, y);
		}
		else {
			state = IDLE;
			anim_timer = 0;
			vel_x = 0;
			vel_y = 0;
		}
		return (hp <= 0);
	}
};

std::vector<Enemy*> enemies;

void SpawnCosasScenario()
{

	int area_left = -4000;
	int area_right = 4000;
	int area_top = -4000;
	int area_bottom = 4000;

	for (int i = 0; i < 100; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::TREE, x, y);
	}

}



void InitPlayers() {
	int madera = 0;
	for (int i = 0; i < NUM_PLAYERS; i++) {
		players[i] = new Player(i);

	}
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

	//Collsions with enemies
	sf::FloatRect bounding(b->x, b->y, bullet_texture->getSize().x, bullet_texture->getSize().y);
	for (Enemy* e : enemies) {
		if (bounding.intersects(getBoundBoxSprite(&(e->sprite)))) {
			e->hp -= 50;
			return true;
		}
	}

	return false;

}


void UpdatePlayer(float dt, int num_player, sf::View& cam)
{
	Player* p = players[num_player];

	sf::Vector2f stick_L = GamePad::AnalogStick::Left.get(num_player);
	sf::Vector2f stick_R = GamePad::AnalogStick::Right.get(num_player);
	float length_L = Mates::Length(stick_L);
	float length_R = Mates::Length(stick_R);

	if (length_L < 30) // Dead zone -> quieto
	{
		stick_L = sf::Vector2f(0, 0);
		p->state = PlayerState::IDLE;
		p->anim_timer = 0;
	}
	else if (p->madera_progress > 0) //gathering -> quieto
	{
		stick_L = sf::Vector2f(0, 0);
		p->state = PlayerState::GATHERING;
		p->anim_timer += dt;
	}
	else // else -> walking
	{
		p->state = PlayerState::WALKING;
		p->anim_timer += dt;
	}
	
	// Update speed
	sf::Vector2f direction = Mates::Normalize(sf::Vector2f(stick_L.x, stick_L.y));
	p->vel_x = direction.x * PLAYER_SPEED;
	p->vel_y = direction.y * PLAYER_SPEED;

	// Update pos
	p->x = p->x + p->vel_x * dt;
	p->y = p->y + p->vel_y * dt;

	// Keep inside view
	p->x = Mates::Clamp(p->x, cam.getCenter().x - cam.getSize().x / 2 + 50, cam.getCenter().x + cam.getSize().x / 2 - 50);
	p->y = Mates::Clamp(p->y, cam.getCenter().y - cam.getSize().y / 2 + 50, cam.getCenter().y + cam.getSize().y / 2 - 50);

	// Update facing vector
	if (length_R > 30)
	{
		p->facing_vector = stick_R;
	}
	else if (length_L > 30)
	{
		p->facing_vector = stick_L;
	}
	p->facing_vector = Mates::Normalize(sf::Vector2f(p->facing_vector.x, p->facing_vector.y));

	float angle = 180+Mates::RadsToDegs(atan2(p->facing_vector.y, p->facing_vector.x));
	if (angle > 270+45) {
		p->facing = FacingDirection::LEFT;
	} else if (angle > 225) {
		p->facing = FacingDirection::DOWN;
	} else if (angle > 45+90) {
		p->facing = FacingDirection::RIGHT;
	} else {
		p->facing = FacingDirection::UP;
	}

	//Shot
	if (p->bullet_cooldown > 0) {
		p->bullet_cooldown -= dt;
	}
	if (GamePad::Trigger::Right.IsJustPressed(num_player) && p->bullet_cooldown <= 0 && p->madera_progress <= 0) {
		bullets.push_back(new Bullet(p->x, p->y, p->facing_vector, num_player));
		p->bullet_cooldown = BULLET_COOLDOWN;
	}


	// Gather wood
	static std::vector<GameObject*> objs_near;
	objs_near.clear();
	obj_manager.getObjects(objs_near, cam);

	GameObject* touching_arbol = NULL;
	for (GameObject* obj : objs_near)
	{
		if (getBoundBox(obj).intersects(p->boundBox()))
		{
			touching_arbol = obj;
		}
	}

	if (GamePad::IsButtonJustReleased(num_player, GamePad::Button::B)) {
		p->madera_progress = 0;
	}
	if (touching_arbol && GamePad::IsButtonPressed(num_player, GamePad::Button::B)) {
		p->madera_progress += dt;
		if (p->madera_progress >= MADERA_GATHER_TIME) {
			madera += 1;
			p->madera_progress = 0;
			particles.push_back(new Particle(*madera_texture, p->x, p->y - 50, 0, -200, 0.2f));
			obj_manager.DestroyObject(touching_arbol);;
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

void SpawnAndUnspawnEnemies(sf::Time dt_time, sf::View& cam) 
{

	static float timer_spawn_next = 0.0f;
	timer_spawn_next += dt_time.asSeconds();

	if (timer_spawn_next > 5.0f)
	{
		int x = cam.getCenter().x;
		int y = cam.getCenter().y;

		float x_enemy = x + std::rand() % static_cast<int>(cam.getSize().x * 3) - cam.getSize().x;
		float y_enemy = y + std::rand() % static_cast<int>(cam.getSize().y * 3) - cam.getSize().y;

		if (x_enemy > (cam.getCenter().x - cam.getSize().x*0.5f) &&
			x_enemy < (cam.getCenter().x + cam.getSize().x*0.5f) &&
			y_enemy >(cam.getCenter().y - cam.getSize().y*0.5f) &&
			y_enemy < (cam.getCenter().y + cam.getSize().y*0.5f))
		{

		}
		else
		{
			enemies.push_back(new Enemy(x_enemy, y_enemy));
		}

		timer_spawn_next = 0.0f;

	}

	


}

void SpawnOasis(int x, int y)
{

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			obj_manager.Spawn(GameObjectType::WATER, x + i*16, y + j*16);
		}
	}
		


}
int main()
{
	srand(time(NULL));

	sf::ContextSettings settings;
	sf::RenderWindow window(sf::VideoMode(RES_X, RES_Y), "SFML works!", sf::Style::Default, settings);
	sf::RenderTexture renderTexture;
	renderTexture.create(RES_X, RES_Y);

	playerColors[0] = sf::Color::Cyan;
	playerColors[1] = sf::Color::Magenta;
	playerColors[2] = sf::Color::Red;
	playerColors[3] = sf::Color::Blue;

	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	sf::View cam(sf::FloatRect(0.0f, 0.0f, RES_X, RES_Y));
	sf::View ui_view(sf::FloatRect(0.0f, 0.f, RES_X, RES_Y));

	cam.zoom(0.4f);

	DayManager dayManager;
	dayManager.InitNightShader(window);

	sf::Font font;
	font.loadFromFile("8bitwonder.ttf");

	player_texture = new sf::Texture();
	player_texture->loadFromFile("desertman_sheet.png");

	bullet_texture = new sf::Texture();
	bullet_texture->loadFromFile("bullet.png");
	sf::Sprite spr_bullet;
	spr_bullet.setTexture(*bullet_texture);
	SpriteCenterOrigin(spr_bullet);

	madera_texture = new sf::Texture();
	madera_texture->loadFromFile("madera.png");
	sf::Sprite spr_madera;
	spr_madera.setTexture(*madera_texture);
	SpriteCenterOrigin(spr_madera);

	enemy_texture = new sf::Texture();
	enemy_texture->loadFromFile("madera.png");

	tex_spritesheet = new sf::Texture();
	tex_spritesheet->loadFromFile("sprite_sheet.png");
	sf::Sprite spr_tile_dessert;
	spr_tile_dessert.setTexture(*tex_spritesheet);
	spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));

	InitPlayers();

	obj_manager.Spawn(GameObjectType::CASA, 0, 0);
	obj_manager.Spawn(GameObjectType::TREE, 50, 50);

	enemies.push_back(new Enemy(600, 400));

	SpawnCosasScenario();

	SpawnOasis(0, 0);

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
		dayManager.Update(dt_time.asSeconds());

		//UPDATE
		for (int i = 0; i < NUM_PLAYERS; i++) {
			UpdatePlayer(dt_time.asSeconds(), i, cam);
		}
		//Enemies
		SpawnAndUnspawnEnemies(dt_time, cam);
		for (int i = 0; i < enemies.size(); i++)
		{
			bool cale_destruir = enemies[i]->Update(dt_time.asSeconds());
			bool esta_lejos = (Mates::Distance(cam.getCenter(), sf::Vector2f(enemies[i]->x, enemies[i]->y))) > cam.getSize().x*8;

			if (cale_destruir || esta_lejos) 
			{
				enemies.erase(enemies.begin() + i);
			}
			else 
			{
				i++;
			}
		}
		//Bullets
		for (int i = 0; i < bullets.size();) {
			bool cale_destruir = UpdateBullet(bullets[i], dt_time.asSeconds(), cam);
			if (cale_destruir) {
				bullets.erase(bullets.begin() + i);
			} else {
				i++;
			}
		}
		//Particles
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
		renderTexture.clear(sf::Color(255, 216, 0)); //Hack to hide black lines between tiles

		{ // UpdateCamera(cam);
			sf::Vector2f centroid;
			for (int i = 0; i < NUM_PLAYERS; ++i) {
					centroid += sf::Vector2f(players[i]->x, players[i]->y);
			}
			centroid = centroid / static_cast<float>(NUM_PLAYERS);
			cam.setCenter(centroid);
		}


//		ImGui::Begin("finester");
//		ImGui::Text("Joy: %f, %f", cam.getCenter().x, cam.getCenter().y);
//		ImGui::End();

		renderTexture.setView(cam);

		//Draw fondo del Mapita
		int start_x = cam.getCenter().x - cam.getSize().x*0.5f - TILE_SIZE;
		start_x = (start_x / TILE_SIZE) * TILE_SIZE;
		int start_y = cam.getCenter().y - cam.getSize().y*0.5f - TILE_SIZE;
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

		//Draw cosas que se ordenan
		static std::vector<sf::Sprite> toDraw;
		toDraw.clear();
		//Objetesitos
		obj_manager.Draw(cam, toDraw, spr_tile_dessert);
		//Playersitos
		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			players[i]->Draw(toDraw);
		}
		//Enemies
		for (int i = 0; i < enemies.size(); i++)
		{
			toDraw.push_back(enemies[i]->sprite);
		}
		//Bulletitas
		for (int i = 0; i < bullets.size(); i++) {
			spr_bullet.setPosition(bullets[i]->x, bullets[i]->y);
			spr_bullet.setColor(playerColors[bullets[i]->player]);
			toDraw.push_back(spr_bullet);
		}
		//Ordering madafaca
		sort(toDraw.begin(), toDraw.end(), [](sf::Sprite& a, sf::Sprite& b) {
			return a.getPosition().y + a.getTextureRect().height/2 < b.getPosition().y + b.getTextureRect().height / 2;
		});
		//Dale draw de verdad
		for (sf::Sprite& d : toDraw) {
			renderTexture.draw(d);
		}


		for (Particle* particle : particles) {

			renderTexture.draw(particle->sprite);
		}


		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			players[i]->DrawUI(renderTexture);
		}

		renderTexture.display();



		window.clear();
		dayManager.RenderWithShader(window, renderTexture);
		dayManager.ImGuiRender();
		ImGui::SFML::Render(window);

		sf::Text txt_money;
		txt_money.setFont(font);
		sf::String str = std::to_string(madera);
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
