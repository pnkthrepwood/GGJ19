#pragma once
#include <list>
#include <set>
#include <SFML/Graphics.hpp>

#define MAX_OBJ_IN_QTREE 50

const int MAX_OBJ_SIZE = 5000;


enum class GameObjectType
{
	NONE,
	CASA,
	DECOR_CACTUS,
	DECOR_SKELETON,
	DECOR_ONE_ROCK,
	DECOR_TWO_ROCKS,
	TREE,
	WATER,
	WATER_BROKEN,
	HAIMA,
	HAIMA_BROKEN
	//...
};
struct GameObject
{
	float x;
	float y;
	GameObjectType type;
};


sf::IntRect SelectSprite(GameObjectType type);

extern GameObject go_collection[MAX_OBJ_SIZE];
sf::Vector2f getObjSize(GameObjectType type);
sf::FloatRect getBoundBox(GameObject* obj);

struct quadNode
{
	quadNode* NW;
	quadNode* NE;
	quadNode* SW;
	quadNode* SE;
	std::list<GameObject*> bucket;
	bool isLeaf;
};

class ObjManager
{
public:
	ObjManager();
	void setBounds(int _height, int _width);
	void AddObject(GameObject* obj);
	void DestroyObject(GameObject* obj);
	void DestroyObject(GameObject* obj, bool canDelete);
	void Draw(const sf::View& Camera, std::vector<sf::Sprite>& toDraw, sf::Sprite& spr);
	void camDraw(std::vector<sf::Sprite>& toDraw, const sf::Vector2f& Position, sf::Sprite& spr, std::vector<quadNode*>& visited);

	bool isColliding(const sf::FloatRect& rect);
	bool isColliding(const sf::Vector2f& point);
	GameObject* getColliding(const sf::Vector2f& point);

	GameObject* getLastObjCollided();
	void getObjectsNear(const sf::Vector2f& pos, const float& threshold, std::vector<GameObject*>& vec);
	int Count();

	void getObjects(std::vector<GameObject*>& vec, const sf::View& camera);

	void DrawBoundaries();
	void DrawBoundaries(quadNode* node, sf::FloatRect bounds);

	int width, height; //World System coordinates (not tiles)
	int min_width, min_height;

	quadNode* quadTree;
	void getAllObjects(std::vector<GameObject*>& vec, quadNode* node);
	void getObjectsNear(const sf::Vector2f& pos, const float& threshold,
						std::vector<GameObject*>& vec, quadNode* node, sf::FloatRect& bounds);
	void getObjectsAux(std::set<GameObject*>& set, sf::Vector2f pos);


	quadNode* searchLeaf(const sf::Vector2f& p);


	GameObject* lastObjCollided;

	int nObjects;

	GameObject* Spawn(GameObjectType type, float x, float y);
};
