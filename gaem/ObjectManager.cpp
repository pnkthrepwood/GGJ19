#include "ObjectManager.h"
#include <list>
#include <iostream>
#include <cmath>
#include <queue>
#include <SFML/Graphics.hpp>
#include <unordered_set>
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )

GameObject go_collection[MAX_OBJ_SIZE];

sf::IntRect SelectSprite(GameObjectType type)
{
	switch (type)
	{
	case GameObjectType::NONE:
		return sf::IntRect(0, 0, 0, 0);
	case GameObjectType::CASA:
		return sf::IntRect(0, 0, 16, 16);
	case GameObjectType::TREE:
		return sf::IntRect(0, 2 * 16, 16, 16);
	case GameObjectType::WATER:
		return sf::IntRect(3 * 16, 0 * 16, 16, 16);
	default:
		std::cout << "Sprite type not handled madafaca" << std::endl;
		return sf::IntRect(0, 0, 0, 0);
	}
}

sf::Vector2f getObjSize(GameObjectType type)
{
	switch (type)
	{
		case GameObjectType::NONE:
		{
			return sf::Vector2f(0, 0);
		} break;

		case GameObjectType::CASA:
		{
			return sf::Vector2f(16, 16);
		} break;
	}

	return sf::Vector2f(-1, -1);
}

sf::FloatRect getBoundBox(GameObject* obj)
{
	sf::Vector2f size = getObjSize(obj->type);
	sf::FloatRect fr;

	fr.left = obj->x - size.x*0.5f;
	fr.width = size.x;
	fr.top = obj->y - size.y*0.5f;
	fr.height = size.y;

	return fr;
}


ObjManager::ObjManager()
{
	quadTree = new quadNode;
	quadTree->isLeaf = true;
	quadTree->NE = NULL;
	quadTree->NW = NULL;
	quadTree->SE = NULL;
	quadTree->SW = NULL;

	lastObjCollided = NULL;

	nObjects = 0;
}

void ObjManager::setBounds(int _height, int _width)
{
	width = _width;
	height = _height;
	min_width = width;
	min_height = height;
}

void ObjManager::AddObject(GameObject* obj)
{
	quadNode* node = quadTree;

	sf::FloatRect bounds;
	bounds.top = 0;
	bounds.left = 0;
	bounds.width = width;
	bounds.height = height;

	while(!node->isLeaf) 
	{	           bool north = false;
	bool west = false;            bool east = false;
		           bool south = false;

		if (int(obj->x) <= bounds.left + bounds.width*0.5f) 
		{ 
			bounds.width = bounds.width*0.5f;
			west = true;
		}
		else 
		{ 
			bounds.left = bounds.left + bounds.width*0.5f;
			bounds.width = bounds.width*0.5f;
			east = true;
		}

		if (int(obj->y) <= bounds.top + bounds.height*0.5f) 
		{ 
			bounds.height = bounds.height*0.5f;
			north = true;
		}
		else 
		{
			bounds.top = bounds.top + bounds.height*0.5f;
			bounds.height = bounds.height*0.5f;
			south = true;
		}

		if (north && west) node = node->NW;
		else if (north && east) node = node->NE;
		else if (south && west) node = node->SW;
		else if (south && east) node = node->SE;
		else std::cerr << "!!! ERROR: Search in QTree failed! (ADDING)" << std::endl;
	}

	node->bucket.push_back(obj);

	if (node->bucket.size() > MAX_OBJ_IN_QTREE)
	{
		node->isLeaf = false;
		node->NE = new quadNode;
		node->NE->isLeaf = true;
		node->NW = new quadNode;
		node->NW->isLeaf = true;
		node->SE = new quadNode;
		node->SE->isLeaf = true;
		node->SW = new quadNode;
		node->SW->isLeaf = true;

		if (width < min_width) min_width = width;
		if (height < min_height) min_height = height;
		
		while(node->bucket.size() > 0)
		{          bool north = false;
	bool west = false;            bool east = false;
		           bool south = false;

		GameObject* target = (node->bucket.front());

		if (int(target->x) <= bounds.left + bounds.width*0.5f) west = true;
		else east = true;

		if (int(target->y) <= bounds.top + bounds.height*0.5f) north = true;
		else south = true;
		
		if (north && west) node->NW->bucket.push_back(target);
		else if (north && east) node->NE->bucket.push_back(target);
		else if (south && west) node->SW->bucket.push_back(target);
		else if (south && east) node->SE->bucket.push_back(target);
		else std::cerr << "!!! ERROR: Search in QTree failed! (RESIZING)" << std::endl;

		node->bucket.pop_front();
		//std::cerr << "Objects: " << objects << std::endl;
		}
	}
	
	++nObjects;
}


void ObjManager::DestroyObject(GameObject* obj) 
{
	DestroyObject(obj, true);
}

void ObjManager::DestroyObject(GameObject* obj, bool canDelete) 
{
	quadNode* node = quadTree;
	quadNode* parent = NULL;

	sf::FloatRect bounds;
	bounds.top = 0;
	bounds.left = 0;
	bounds.width = width;
	bounds.height = height;

	while(!node->isLeaf) 
	{	           bool north = false;
	bool west = false;            bool east = false;
		           bool south = false;

		if (int(obj->x) <= bounds.left + bounds.width*0.5f) 
		{ 
			bounds.width = bounds.width*0.5f;
			west = true;
		}
		else 
		{ 
			bounds.left = bounds.left + bounds.width*0.5f;
			bounds.width = bounds.width*0.5f;
			east = true;
		}

		if (int(obj->y) <= bounds.top + bounds.height*0.5f) 
		{ 
			bounds.height = bounds.height*0.5f;
			north = true;
		}
		else 
		{
			bounds.top = bounds.top + bounds.height*0.5f;
			bounds.height = bounds.height*0.5f;
			south = true;
		}

		parent = node;

		if (north && west) node = node->NW;
		else if (north && east) node = node->NE;
		else if (south && west) node = node->SW;
		else if (south && east) node = node->SE;
		else std::cerr << "! ERROR: Search in QTree failed! (DELETING)" << std::endl;
	}

	for (std::list<GameObject*>::iterator it = node->bucket.begin(); it != node->bucket.end(); ++it) 
	{
		if ((*it) == obj) 
		{
			it = node->bucket.erase(it);
			if (canDelete)
			{
				obj->type = GameObjectType::NONE;
			}
			break;
		}
	}

	if (parent != NULL     && !parent->isLeaf && 
		parent->NE != NULL && parent->NE->isLeaf &&
		parent->NW != NULL && parent->NW->isLeaf &&
		parent->SE != NULL && parent->SE->isLeaf &&
		parent->SW != NULL && parent->SW->isLeaf) {

		unsigned int total = 0;
		total += parent->NE->bucket.size();
		total += parent->NW->bucket.size();
		total += parent->SE->bucket.size();
		total += parent->SW->bucket.size();

		if (total < MAX_OBJ_IN_QTREE) 
		{
			std::cerr << "! Resizing OBJECT MANAGER" << std::endl;

			parent->bucket.splice(parent->bucket.end(), parent->NE->bucket);
			parent->bucket.splice(parent->bucket.end(), parent->NW->bucket);
			parent->bucket.splice(parent->bucket.end(), parent->SE->bucket);
			parent->bucket.splice(parent->bucket.end(), parent->SW->bucket);

			delete parent->NE;
			parent->NE = NULL;
			delete parent->NW;
			parent->NW = NULL;
			delete parent->SE;
			parent->SE = NULL;
			delete parent->SW;
			parent->SW = NULL;
			parent->isLeaf = true;
		}
	}
	
	--nObjects;
}

void ObjManager::Draw(const sf::View& Camera, std::vector<sf::Sprite>& toDraw, sf::Sprite& spr)
{
	float size_x, size_y;
	size_x = Camera.getSize().x*0.5f;
	size_y = Camera.getSize().y*0.5f;
	sf::Vector2f center = Camera.getCenter();

	camDraw(toDraw, center + sf::Vector2f(-size_x*0.5f*2.f, -size_y*0.5f*2.f), spr);
	camDraw(toDraw, center + sf::Vector2f(0, -size_y*0.5f*1.5f), spr);
	camDraw(toDraw, center + sf::Vector2f(+size_x*0.5f*2.f, -size_y*0.5f*2.f), spr);

	camDraw(toDraw, center + sf::Vector2f(-size_x*0.5f*2.f, 0), spr);
	camDraw(toDraw, center, spr);
	camDraw(toDraw, center + sf::Vector2f(+size_x*0.5f*2.f, 0), spr);
	
	camDraw(toDraw, center + sf::Vector2f(-size_x*0.5f*2.f, +size_y*0.5f*2.f), spr);
	camDraw(toDraw, center + sf::Vector2f(0, +size_y*0.5f*1.5f), spr);
	camDraw(toDraw, center + sf::Vector2f(+size_x*0.5f*2.f, +size_y*0.5f*2.f), spr);
}

void ObjManager::camDraw(std::vector<sf::Sprite>& toDraw, const sf::Vector2f& Position, sf::Sprite& spr)
{
	quadNode* node = searchLeaf(Position);
	std::list<GameObject*>::iterator it;
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it)
	{
		spr.setTextureRect( SelectSprite((*it)->type));

		spr.setPosition((*it)->x, (*it)->y);
		
		toDraw.push_back(spr); //Makes a copy which is actually needed
		//(*it)->Draw();
	}
}

bool ObjManager::isColliding(const sf::Vector2f& point) 
{
	
	quadNode* node;
	std::list<GameObject*>::iterator it;

	node = searchLeaf(point);
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it) {	
		sf::FloatRect objRect = getBoundBox((*it)); //(*it)->getBoundBox();
		if (objRect.contains(point)) return true;	
	}

	return false;
	
}

GameObject* ObjManager::getColliding(const sf::Vector2f& point)
{
	
	quadNode* node;
	std::list<GameObject*>::iterator it;

	node = searchLeaf(point);
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it) 
	{	
		sf::FloatRect objRect = getBoundBox((*it));
		if (objRect.contains(point)) return (*it);
	}
	
	return NULL;
}

bool ObjManager::isColliding(const sf::FloatRect& rect)
{
	
	quadNode* node;
	std::list<GameObject*>::iterator it;

	node = searchLeaf(sf::Vector2f((float)rect.left + rect.width/2, (float)rect.top + rect.height/2));
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it)
	{
		sf::FloatRect objRect = getBoundBox((*it));
		if (rect.intersects(objRect)) 
		{
			lastObjCollided = (*it);
			return true;
		}
	}

	node = searchLeaf(sf::Vector2f((float)rect.left - rect.width*2, (float)rect.top + rect.height/2));
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it)
	{
		sf::FloatRect objRect = getBoundBox((*it));
		if (rect.intersects(objRect)) 
		{
			lastObjCollided = (*it);
			return true;
		}
	}

	node = searchLeaf(sf::Vector2f((float)rect.left + rect.width/2, (float)rect.top - rect.height*2));
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it)
	{
		sf::FloatRect objRect = getBoundBox((*it));
		if (rect.intersects(objRect)) 
		{
			lastObjCollided = (*it);
			return true;
		}
	}

	node = searchLeaf(sf::Vector2f((float)rect.left - rect.width/2, (float)rect.top - rect.height*2));
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it)
	{
		sf::FloatRect objRect = getBoundBox((*it));
		if (rect.intersects(objRect)) 
		{
			lastObjCollided = (*it);
			return true;
		}
	}

	return false;
}

GameObject* ObjManager::getLastObjCollided()
{
	return lastObjCollided;
}

void ObjManager::getAllObjects(std::vector<GameObject*>& vec, quadNode* node) 
{
	std::list<GameObject*>::iterator it;
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it) 
	{
		vec.push_back(*it);
	}

}



static bool rectTouchesCircle(const sf::FloatRect& r,
	const sf::Vector2f& c, const float& rad) {
	sf::Vector2f cDist;
	cDist.x = abs(c.x - r.left);
	cDist.y = abs(c.y - r.top);

	if (cDist.x > (r.width / 2 + rad)) return false;
	if (cDist.y > (r.height / 2 + rad)) return false;

	if (cDist.x <= (r.width) / 2) return true;
	if (cDist.y <= (r.height) / 2) return true;

	float cornerDist_sq = (cDist.x - r.width / 2)*(cDist.x - r.width / 2) +
		(cDist.y - r.height / 2)*(cDist.y - r.height / 2);

	return (cornerDist_sq <= (rad*rad));

}

void ObjManager::getObjectsNear(const sf::Vector2f& pos, const float& threshold, 
	std::vector<GameObject*>& vec, quadNode* node, sf::FloatRect& bounds)
{
	sf::FloatRect fr(pos.x - threshold/2, pos.y - threshold/2, 
		threshold, threshold);

	if (!rectTouchesCircle(bounds, pos, threshold) &&
		!fr.intersects(bounds) && !bounds.intersects(fr) &&
		!fr.contains(sf::Vector2f(bounds.left, bounds.top)) && 
		!fr.contains(sf::Vector2f(bounds.left, bounds.top+bounds.height)) && 
		!fr.contains(sf::Vector2f(bounds.left+bounds.width, bounds.top)) && 
		!fr.contains(sf::Vector2f(bounds.left+bounds.width, bounds.top+bounds.height)) && 
		!bounds.contains(sf::Vector2f(fr.left, fr.top)) && 
		!bounds.contains(sf::Vector2f(fr.left, fr.top+bounds.height)) && 
		!bounds.contains(sf::Vector2f(fr.left+bounds.width, fr.top)) && 
		!bounds.contains(sf::Vector2f(fr.left+bounds.width, fr.top+bounds.height))) 
	{
		/*
			InputEng* input = InputEng::getInstance();
			
			if (input->getKeyDown(InputEng::F1)) {
				std::cerr << "GETOBJECTSNEAR NOT ON SITIO" << std::endl;
				std::cerr << "Bounds: " << bounds.left << " "<< bounds.top << " " <<
					bounds.width << " " << bounds.height << std::endl
					<< "Fr: " << fr.left << " "<< fr.top << " " <<
					fr.width << " " << fr.height << std::endl;
			}
			*/
			return;
	}

	if (node->isLeaf) 
	{
		/*
		//Create boxpoints
		sf::Vector2f center(bounds.left+bounds.width/2,		
					    bounds.top+bounds.height/2);
		std::vector<sf::Vector2f> boxPoints;
		boxPoints.push_back(center);
		boxPoints.push_back(sf::Vector2f(bounds.left, bounds.top));
		boxPoints.push_back(sf::Vector2f(bounds.left+bounds.width, bounds.top));
		boxPoints.push_back(sf::Vector2f(bounds.left, bounds.top+bounds.height));
		boxPoints.push_back(sf::Vector2f(bounds.left+bounds.width, bounds.top+bounds.height));

		//Is touching?
		bool isTouching = false;
		for (int i = 0; i < boxPoints.size(); ++i) {
			if (Utils::distance(boxPoints[i], pos) <= threshold) {
				isTouching = true;
			}
		}
		if (!isTouching) return;
		*/
		//Push'em'all
		std::list<GameObject*>::iterator it;
		for (it = node->bucket.begin(); it != node->bucket.end(); ++it) 
		{
			vec.push_back((*it));
		}

		return;
	}

	else 
	{
		bounds.width *= 0.5f;
		bounds.height *= 0.5f;

		getObjectsNear(pos, threshold, vec, node->NW, bounds);
			//sf::FloatRect(bounds.left, bounds.top, 
			//bounds.width*0.5f, bounds.height*0.5f));

		bounds.left += bounds.width;

		getObjectsNear(pos, threshold, vec, node->NE, bounds);
			//sf::FloatRect(bounds.left+bounds.width*0.5f, bounds.top, 
			//bounds.width*0.5f, bounds.height*0.5f));

		bounds.left -= bounds.width;
		bounds.top += bounds.height;

		getObjectsNear(pos, threshold, vec, node->SW, bounds);
			//sf::FloatRect(bounds.left, bounds.top+bounds.height*0.5f, 
			//bounds.width*0.5f, bounds.height*0.5f));

		bounds.left += bounds.width;

		getObjectsNear(pos, threshold, vec, node->SE, bounds);
			//sf::FloatRect(bounds.left+bounds.width*0.5f, 
			//bounds.top+bounds.height*0.5f,
			//bounds.width*0.5f, bounds.height*0.5f));

		bounds.left -= bounds.width;
		bounds.width *= 2;
		bounds.top -= bounds.height;
		bounds.height *= 2;
	}
	


}

void ObjManager::getObjectsNear(const sf::Vector2f& pos, const float& threshold, std::vector<GameObject*>& vec)
{
	//Threshold is in World Distance System units
	//This function puts all the objects of the QTree inside the circle
	//defined by 'pos' and 'threshold'


	sf::FloatRect fr(0, 0, width, height);

	getObjectsNear(pos, threshold, vec, quadTree, fr);// sf::FloatRect(0, 0, width, height));

}

/*
void ObjManager::DrawBoundaries() 
{
	DrawBoundaries(quadTree, sf::FloatRect(0,0,width,height));
}

void ObjManager::DrawBoundaries(quadNode* node, sf::FloatRect bounds) 
{
	if (node->isLeaf) 
	{
		sf::RectangleShape rect(sf::Vector2f(bounds.width, bounds.height));
		rect.setPosition(sf::Vector2f(bounds.left, bounds.top));
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::Green);
		rect.setOutlineThickness(20);
		App->draw(rect);
		return;
	}
	sf::RectangleShape rect(sf::Vector2f(bounds.width, bounds.height));
		rect.setPosition(sf::Vector2f(bounds.left, bounds.top));
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::Red);
		rect.setOutlineThickness(50);
		App->draw(rect);
	

	
		DrawBoundaries(
			node->NE, sf::FloatRect(bounds.left, bounds.top, 
			bounds.width*0.5f, bounds.height*0.5f));

		DrawBoundaries(
			node->NW, sf::FloatRect((bounds.left+bounds.width)*0.5f, bounds.top, 
			bounds.width, bounds.height*0.5f));

		DrawBoundaries(
			node->SE, sf::FloatRect(bounds.left, (bounds.top+bounds.height)*0.5f, 
			bounds.width*0.5f, bounds.height));

		DrawBoundaries(
			node->SW, sf::FloatRect((bounds.left+bounds.width)*0.5f, (bounds.top+bounds.height)*0.5f,
			bounds.width, bounds.height));
	
}
*/

quadNode* ObjManager::searchLeaf(const sf::Vector2f& p)
{
	quadNode* node = quadTree;

	sf::FloatRect bounds;

	bounds.height  = height;
	bounds.top = 0;
	bounds.left = 0;
	bounds.width = width;

	while(!node->isLeaf)
	{	          bool north = false;
		bool west = false;    bool east = false;
                 bool south = false;
		
		if (int(p.x) <= bounds.left + bounds.width*0.5f) { 
			bounds.width = bounds.width*0.5f;
			west = true;
		}
		else { 
			bounds.left = bounds.left + bounds.width*0.5f;
			bounds.width = bounds.width*0.5f;
			east = true;
		}

		if (int(p.y) <= bounds.top + bounds.height*0.5f) { 
			bounds.height = bounds.height*0.5f;
			north = true;
		}
		else {
			bounds.top = bounds.top + bounds.height*0.5f;
			bounds.height = bounds.height*0.5f;
			south = true;
		}

		if (north && west) node = node->NW;
		else if (north && east) node = node->NE;
		else if (south && west) node = node->SW;
		else if (south && east) node = node->SE;
		else std::cerr << "ERROR: Search in QTree fucked up!" << std::endl;
	}

	return node;
}

int ObjManager::Count() 
{
	return nObjects;
}


void ObjManager::getObjects(std::vector<GameObject*>& vec, const sf::View& camera)
{
	//Asumimos cosas de la camara

	quadNode* node = searchLeaf(camera.getCenter());
	std::list<GameObject*>::iterator it;
	for (it = node->bucket.begin(); it != node->bucket.end(); ++it) 
	{
		vec.push_back(*it);
	}
	


	/*
	std::unordered_set<quadNode*> set;

	for (int i = camera.getCenter().x - camera.getSize().x*2.0f;
		i < camera.getCenter().x + camera.getSize().x*2.0f;
		i += 16*2) 
	{
			for (int j = camera.getCenter().y - camera.getSize().y*2.0f;
				j < camera.getCenter().y + camera.getSize().y*2.0f;
				j += 16 *2)
			{
					//std::cerr << i << ",,," << j << std::endl;

					quadNode* node = searchLeaf(sf::Vector2f(i, j));

					if (set.find(node) == set.end()) 
					{
						set.insert(node);

						std::list<GameObject*>::iterator it;
						for (it = node->bucket.begin(); 
							it != node->bucket.end(); ++it){
								vec.push_back(*it);
						}
					}
			}
	}
*/

}


void ObjManager::Spawn(GameObjectType type, int x, int y)
{
	for (int i = 0; i < MAX_OBJ_SIZE; ++i)
	{
		if (go_collection[i].type == GameObjectType::NONE)
		{
			go_collection[i].x = x;
			go_collection[i].y = y;
			go_collection[i].type = type;

			AddObject(&go_collection[i]);

			return;
		}
	}

	//Error: si llega aqui, todo mal
}