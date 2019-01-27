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
const float PLAYER_SPEED = 200;

const float BULLET_SPEED = 700;
const float ENEMY_TRIGGER_DISTANCE = 180;
const float ENEMY_MAX_SPEED = 250;
const float BULLET_COOLDOWN = 0.1f; //seconds
const float MADERA_GATHER_TIME = 3.5; //seconds

ObjManager obj_manager;
int madera;

sf::Texture* tex_spritesheet;
sf::Texture* player_texture;
sf::Texture* madera_texture;
sf::Texture* bullet_texture;

sf::Sound* groar1; //done
sf::Sound* groar2; //done
sf::Sound* grills;
sf::Sound* shot; //Done
sf::Sound* mussol;
sf::Sound* hammer;
sf::SoundBuffer* bchopwood; //done

sf::Font* font;

sf::Music* music; //Done
sf::Music* musicdanger; //done

sf::Shader* nightLight;
sf::VertexArray quad(sf::Quads, 4);
sf::Clock clockDay;

sf::Color playerColors[4];

void SpriteCenterOrigin(sf::Sprite& spr)
{
	spr.setOrigin(spr.getTextureRect().width / 2.f, spr.getTextureRect().height / 2.f);
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
	SHOOTING,
	PREPARING_LANCE,
	THROWING_LANCE
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
	bool will_shot = false;
	sf::Sound chopwood;
	float inmune;

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
		, chopwood(*bchopwood)
		, inmune(-1)
	{
		sprite.setTexture(*player_texture);
		sprite.setOrigin(5, 8);
		//sprite.setColor(playerColors[n_player]);

		progress.setFillColor(sf::Color::Transparent);
		progress.setOrigin(47, 47);
		progress.setScale(-0.3f, 0.3f);
		progress.setOutlineColor(sf::Color(0, 200, 0, 100));
		lifeBar.setFillColor(sf::Color(250, 20, 20));

		const int PLAYER_INITIAL_POS_OFFSET = 90;
		x = RES_X / 2;
		y = RES_Y / 2;
		switch (n_player) 
		{
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
		sf::FloatRect int_rect = 
		sf::FloatRect(
			x - sprite.getTextureRect().width / 2,
			y - sprite.getTextureRect().height / 2,
			sprite.getTextureRect().width,
			sprite.getTextureRect().height
		);

		return int_rect;
	}

	sf::Vector2f centerPos() {
		 const auto box = boundBox();
		 return {box.left + box.width/2.f, box.top + box.height/2.f};
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
			texrect.left += (static_cast<int>(anim_timer / 0.1f) % 3) * 11;
		}
		else if (state == PlayerState::GATHERING)
		{
			texrect.left = 5*11 + (static_cast<int>(anim_timer / 0.4f) % 2) * 11;
		}
		else if (state == PlayerState::PREPARING_LANCE)
		{
			texrect.left = 3 * 11;
		}
		else if (state == PlayerState::THROWING_LANCE)
		{
			texrect.left = 4 * 11;
		}

		sprite.setTextureRect(texrect);
		toDraw.push_back(sprite);
	}

	void DrawUI(sf::RenderTarget& rt)
	{
		progress.setOutlineThickness(12); //Force redraw shape
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
		sprite.setScale(0.6f, 0.6f);
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

	float last_hit_timer = 0.0f;

	sf::Sprite* sprite;

	~Enemy() {
		delete sprite;
	}

	Enemy(float px, float py) : x(px), y(py), vel_x(0), vel_y(0), state(IDLE), anim_timer(0.f)
	{
		sprite = new sf::Sprite();
		sprite->setTexture(*tex_spritesheet);
		sprite->setOrigin(8, 8);
	}

	bool Update(float dt)
	{

		Player* closestPlayer = nullptr;
		float closestDist = 9999999999.f;
		for (Player *p : players)
		{
			float d = Mates::Distance(sf::Vector2f(p->x, p->y), sf::Vector2f(x, y));
			if (d < closestDist)
			{
				closestDist = d;
				closestPlayer = p;
			}
		}
		if (closestDist < ENEMY_TRIGGER_DISTANCE)
		{
			anim_timer += dt;
			if (state == IDLE) 
			{
				if (std::rand() % 2) 
				{
					groar2->play();
				}
				else 
				{
					groar1->play();
				}

				state = WALKING;
				sf::Vector2f dir(closestPlayer->x - x, closestPlayer->y - y);
				sf::Vector2f dir_bona = Mates::Normalize(dir);

				vel_x = dir_bona.x * ENEMY_MAX_SPEED;
				vel_y = dir_bona.y * ENEMY_MAX_SPEED;

			}
			x += vel_x * dt;
			y += vel_y * dt;
		}
		else
		{
			state = IDLE;
			anim_timer = 0;
			vel_x = 0;
			vel_y = 0;
		}

		//Collisions with enemies
		sf::FloatRect bounding(x - 4, y - 4, 8, 8);
		for (Player* player : players) 
		{
			if (player->inmune > 0) continue;
			if (bounding.intersects(player->boundBox())) 
			{
				player->inmune = 2.f;
				//player->hp -= 50;
				if (madera > 0) 
				{
					madera -= 1;
					particles.push_back(new Particle(*madera_texture, player->x, player->y + 25, (rand() % 2) ? 200 : -200, 200, 0.2f));
				}
				shot->play();
			}
		}

		return (hp <= 0);
	}
};

std::vector<Enemy*> enemies;

bool chunksSpawned[4000][4000] = { 0 };

void SpawnCosasEnChunk(int casilla_x, int casilla_y, bool first_tile = false)
{
	if (chunksSpawned[casilla_x][casilla_y])
	{
		return;
	}

	cout << "Spawning chunk " << casilla_x << "," << casilla_y << " " << first_tile << endl;

	chunksSpawned[casilla_x][casilla_y] = true;

	casilla_x -= 2000;
	casilla_y -= 2000;
	int area_left = casilla_x * RES_X;
	int area_right = (casilla_x + 1) * RES_X;
	int area_top = (casilla_y) * RES_Y;
	int area_bottom = (casilla_y + 1) * RES_Y;;

	//Trees
	for (int i = 0; i < 8; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::TREE, x, y);
	}

	//Decor
	for (int i = 0; i < 8; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::DECOR_1, x, y);
	}
	for (int i = 0; i < 8; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::DECOR_2, x, y);
	}
	for (int i = 0; i < 8; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::DECOR_3, x, y);
	}
	for (int i = 0; i < 8; ++i)
	{
		int x = std::rand() % (area_right - area_left) + area_left;
		int y = std::rand() % (area_bottom - area_top) + area_top;

		obj_manager.Spawn(GameObjectType::DECOR_4, x, y);
	}

	if (!first_tile) {

		//Enemies
		for (int i = 0; i < 5; ++i)
		{
			int x = std::rand() % (area_right - area_left) + area_left;
			int y = std::rand() % (area_bottom - area_top) + area_top;

			enemies.push_back(new Enemy(x, y));
		}

		//Water
		if (std::rand()%2) {
			int x = std::rand() % (area_right - area_left) + area_left;
			int y = std::rand() % (area_bottom - area_top) + area_top;

			obj_manager.Spawn(GameObjectType::WATER, x, y);
		}

	}
}


int current_casilla_x = -500, current_casilla_y = -500;

pair<int, int> GetCasillaFromCam(sf::View& cam) {
	int x = int(cam.getCenter().x - cam.getSize().x / 2) / RES_X;
	int y = int(cam.getCenter().y - cam.getSize().y / 2) / RES_Y;
	return make_pair(x+2000, y + 2000);
}

void InitPlayers() 
{
	for (int i = 0; i < NUM_PLAYERS; i++) 
	{
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
		b->y < cam.getCenter().y - cam.getSize().y / 2) 
	{
		return true;
	}

	//Collisions with enemies
	sf::FloatRect bounding(b->x, b->y, bullet_texture->getSize().x, bullet_texture->getSize().y);
	for (Enemy* e : enemies) 
	{
		if (bounding.intersects(getBoundBoxSprite(e->sprite))) 
		{
			e->last_hit_timer = 0.2f;
			e->hp -= 50;
			shot->play();
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

	if (p->inmune > 0) 
	{
		p->inmune -= dt;
	}


	if (p->state != PlayerState::PREPARING_LANCE &&
		p->state != PlayerState::THROWING_LANCE)
	{
		if (p->madera_progress > 0) //gathering -> quieto
		{
			stick_L = sf::Vector2f(0, 0);
			if (p->state != PlayerState::GATHERING)
			{
				p->anim_timer = 0;
				p->chopwood.play();
			}
			p->state = PlayerState::GATHERING;
			p->anim_timer += dt;
		}
		else if (length_L < 30) // Dead zone -> quieto
		{
			stick_L = sf::Vector2f(0, 0);
			p->state = PlayerState::IDLE;
			p->anim_timer = 0;
		}
		else // else -> walking
		{
			p->state = PlayerState::WALKING;
			p->anim_timer += dt;
		}
	}
	else
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
	p->x = Mates::Clamp(p->x, cam.getCenter().x - cam.getSize().x / 2 + 50, cam.getCenter().x + cam.getSize().x / 2 - 50);
	p->y = Mates::Clamp(p->y, cam.getCenter().y - cam.getSize().y / 2 + 50, cam.getCenter().y + cam.getSize().y / 2 - 50);

	// Update facing vector
	sf::Vector2f facing_vector(0, 0);
	if (length_R > 30)
	{
		facing_vector = stick_R;
	}
	else if (length_L > 30)
	{
		facing_vector = stick_L;
	}
	if (Mates::Length(facing_vector) > 0.01f) 
	{
		p->facing_vector = Mates::Normalize(facing_vector);
	}

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
	if (p->state == PlayerState::THROWING_LANCE && p->bullet_cooldown > 0)
	{
		p->bullet_cooldown -= dt;
		if (p->bullet_cooldown < 0)
		{
			p->state = PlayerState::IDLE;
		}
	}
	else if (p->will_shot) 
	{
		p->state = PlayerState::THROWING_LANCE;
		p->bullet_cooldown = BULLET_COOLDOWN;
		bullets.push_back(new Bullet(p->x, p->y, p->facing_vector, num_player));
		p->will_shot = false;
	}
	else if (p->state == PlayerState::PREPARING_LANCE && p->bullet_cooldown > 0)
	{
		p->bullet_cooldown -= dt;
		
		if (p->bullet_cooldown < 0) 
		{
			p->will_shot = true;
		}
	}
	else if (GamePad::Trigger::Right.IsJustPressed(num_player) && p->bullet_cooldown <= 0 && p->madera_progress <= 0) 
	{
		p->bullet_cooldown = BULLET_COOLDOWN;
		p->state = PlayerState::PREPARING_LANCE;
	}
	

	// Gather wood
	static std::vector<GameObject*> objs_near;
	objs_near.clear();
	obj_manager.getObjects(objs_near, cam);

	GameObject* touching_arbol = NULL;
	for (GameObject* obj : objs_near)
	{
		if (obj->type == GameObjectType::TREE)
		{
			if (getBoundBox(obj).intersects(p->boundBox()))
			{

				touching_arbol = obj;
			}
		}
	}

	if (GamePad::IsButtonJustReleased(num_player, GamePad::Button::X))
	{
		p->madera_progress = 0;
		p->chopwood.stop();
	}
	if (touching_arbol && GamePad::IsButtonPressed(num_player, GamePad::Button::X))
	{
		p->madera_progress += dt;
		if (p->madera_progress >= MADERA_GATHER_TIME)
		{
			madera += 1;
			p->madera_progress = 0;
			particles.push_back(new Particle(*madera_texture, p->x, p->y - 25, 0, -200, 0.2f));
			obj_manager.DestroyObject(touching_arbol);;
		}
	}


	// Plant Haimas
	objs_near.clear();
	obj_manager.getObjects(objs_near, cam);

	GameObject* oasis_near = NULL;
	sf::Vector2f playerPos(p->centerPos());
	for (GameObject* obj : objs_near)
	{
		if (obj->type == GameObjectType::HAIMA)
		{
			if (getBoundBox(obj).intersects(p->boundBox()))
			{
				touching_arbol = obj;
			}
		}
	}


	if (p->hp < 100) 
	{
		p->hp += dt;
		if (p->hp > 100) 
		{
			p->hp = 100;
		}
	}
}

//	3hola qye ts c-dcD
//		hola mamonn bbbbastant er


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

	font = new sf::Font();
	font->loadFromFile("8bitwonder.ttf");

	player_texture = new sf::Texture();
	player_texture->loadFromFile("desertman_sheet.png");

	bullet_texture = new sf::Texture();
	bullet_texture->loadFromFile("desertman_sheet.png");
	sf::Sprite spr_bullet;
	spr_bullet.setTexture(*bullet_texture);
	SpriteCenterOrigin(spr_bullet);

	madera_texture = new sf::Texture();
	madera_texture->loadFromFile("madera.png");
	sf::Sprite spr_madera;
	spr_madera.setTexture(*madera_texture);
	SpriteCenterOrigin(spr_madera);

	tex_spritesheet = new sf::Texture();
	tex_spritesheet->loadFromFile("sprite_sheet.png");
	sf::Sprite spr_tile_dessert;
	spr_tile_dessert.setTexture(*tex_spritesheet);
	spr_tile_dessert.setTextureRect(sf::IntRect(1 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE));


	sf::SoundBuffer b_groar1; b_groar1.loadFromFile("groar1.ogg");
	groar1 = new sf::Sound(b_groar1);
	sf::SoundBuffer bgroar2; bgroar2.loadFromFile("groar2.ogg");
	groar2 = new sf::Sound(bgroar2);
	sf::SoundBuffer bgrills; bgrills.loadFromFile("grills.ogg");
	grills = new sf::Sound(bgrills);
	bchopwood = new sf::SoundBuffer();
	bchopwood->loadFromFile("chopwood.ogg"); //chainsaw?

	sf::SoundBuffer bshot; bshot.loadFromFile("shot.ogg");
	shot = new sf::Sound(bshot);
	sf::SoundBuffer bmussol; bmussol.loadFromFile("mussol.ogg");
	mussol = new sf::Sound(bmussol);
	sf::SoundBuffer bhammer; bhammer.loadFromFile("hammer.ogg");
	hammer = new sf::Sound(bhammer);

	music = new sf::Music();
	music->openFromFile("theme.ogg");
	musicdanger = new sf::Music();
	musicdanger->openFromFile("dangerhyena.ogg");

	InitPlayers();

	music->play();
	music->setLoop(true);
	musicdanger->play();
	musicdanger->setVolume(0);
	musicdanger->setLoop(true);

	obj_manager.Spawn(GameObjectType::WATER, RES_X/2, RES_Y / 2);

	auto p = GetCasillaFromCam(cam);
	current_casilla_x = p.first;
	current_casilla_y = p.second;
	cout << "first chunks " << current_casilla_x << "," << current_casilla_y << endl;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			SpawnCosasEnChunk(current_casilla_x + i, current_casilla_y + j, (i == 0 && j == 0));
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

		GamePad::UpdateInputState();
		dayManager.Update(dt_time.asSeconds());


		//Spawn chunks
		auto p = GetCasillaFromCam(cam);
		if (p.first != current_casilla_x ||
			p.second != current_casilla_y)
		{

			current_casilla_x = p.first;
			current_casilla_y = p.second;

			cout << "entering " << current_casilla_x << "," << current_casilla_y << endl;

			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					SpawnCosasEnChunk(current_casilla_x + i, current_casilla_y + j);
				}
			}
		}


		//UPDATE
		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			UpdatePlayer(dt_time.asSeconds(), i, cam);
		}

		//Enemies
		bool in_danger = false;
		for (int i = 0; i < enemies.size();)
		{
			bool cale_destruir = enemies[i]->Update(dt_time.asSeconds());
			if (enemies[i]->state == PlayerState::WALKING) in_danger = true;
			if (cale_destruir)
			{
				delete enemies[i];
				enemies.erase(enemies.begin() + i);
			}
			else
			{
				i++;
			}
		}
		if (in_danger) {
			musicdanger->setVolume(100);
		}
		else {
			musicdanger->setVolume(0);
		}
		//Bullets
		for (int i = 0; i < bullets.size();) {
			bool cale_destruir = UpdateBullet(bullets[i], dt_time.asSeconds(), cam);
			if (cale_destruir) {
				delete bullets[i];
				bullets.erase(bullets.begin() + i);
			} else {
				i++;
			}
		}
		//Particles
		for (int i = 0; i < particles.size();) {
			bool cale_destruir = particles[i]->Update(dt_time.asSeconds());
			if (cale_destruir) {
				delete particles[i];
				particles.erase(particles.begin() + i);
			}
			else {
				i++;
			}
		}



		//DRAW
		//renderTexture.clear(sf::Color(255, 216, 0)); //Hack to hide black lines between tiles

		{ // UpdateCamera(cam);
			sf::Vector2f centroid;
			for (int i = 0; i < NUM_PLAYERS; ++i) {
					centroid += sf::Vector2f(players[i]->x, players[i]->y);
			}
			centroid = centroid / static_cast<float>(NUM_PLAYERS);
			cam.setCenter(centroid);

			sf::Vector2f camcenter = cam.getCenter();
			cam.setCenter(static_cast<int>(camcenter.x) + 0.01f, static_cast<int>(camcenter.y) + 0.01f);

		}


		ImGui::Begin("finester");
		ImGui::Text("Pos: %f, %f", players[0]->x, players[0]->y);
		ImGui::End();


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
			int frame = static_cast<int>(enemies[i]->anim_timer / 0.1f) % 2;

			sf::Sprite * sprite = enemies[i]->sprite;

			sprite->setPosition(enemies[i]->x, enemies[i]->y);
			sprite->setTextureRect(sf::IntRect((1 + frame + ((enemies[i]->vel_x > 0) ? 2 : 0)) * TILE_SIZE, 1*TILE_SIZE, 16, 16));

			toDraw.push_back(*sprite);
		}
		//Bulletitas
		for (int i = 0; i < bullets.size(); i++)
		{
			
			spr_bullet.setPosition(bullets[i]->x, bullets[i]->y);
			spr_bullet.setOrigin( 5, 8 );
			spr_bullet.setTextureRect(sf::IntRect(0, 4*16, 11, 16));

			spr_bullet.setRotation(Mates::RadsToDegs(atan2(bullets[i]->vel_y, bullets[i]->vel_x)));

			//spr_bullet.setColor(playerColors[bullets[i]->player]);
			toDraw.push_back(spr_bullet);
		}
		//Ordering madafaca
		sf::IntRect rectWater = SelectSprite(GameObjectType::WATER);;
		sort(toDraw.begin(), toDraw.end(), [rectWater](sf::Sprite& a, sf::Sprite& b) {
			return a.getPosition().y + a.getTextureRect().height / 2 < b.getPosition().y + b.getTextureRect().height / 2;
		});
		//Dale draw de verdad
		for (sf::Sprite& d : toDraw)
		{

			renderTexture.draw(d);
		}


		for (Particle* particle : particles) {

			renderTexture.draw(particle->sprite);
		}


		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			players[i]->DrawUI(renderTexture);
		}



		/*
		static std::vector<GameObject*> objs_near;
		objs_near.clear();
		obj_manager.getObjects(objs_near, cam);

		for (GameObject* go : objs_near) {
			sf::RectangleShape rs;
			sf::FloatRect bb = getBoundBox(go);
			rs.setPosition(bb.left, bb.top);
			rs.setSize(sf::Vector2f(bb.width, bb.height));
			renderTexture.draw(rs);
		}
		*/
		renderTexture.display();



		window.clear();
		dayManager.RenderWithShader(window, renderTexture);
		dayManager.ImGuiRender();
		ImGui::SFML::Render(window);

		sf::Text txt_money;
		txt_money.setFont(*font);
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

		dayManager.RenderGui(window);

		window.display();

	}

	return 0;
}
