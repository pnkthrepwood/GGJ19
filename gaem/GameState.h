#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

class DayManager;
class ObjManager;
class GameObject;
class ObjManager;

class GameState {
public:
  GameState(DayManager& dayManager, ObjManager& obj_manager);

  void Update(float dt, std::function<bool()> AreAllPlayersInHaima);

  void PlaceHaimaIfPosible(sf::Vector2f pos, int& woodAmount, GameObject* oasis);

  void Render(sf::RenderWindow& window);

  bool IsPlaying() const;


private:
  enum class EState {
    PLAYING,
    DEAD,
    NEXT_DAY_ING,
    SHOWING_NEXT_DAY
  };

  EState mState;

  DayManager& mDayManager;
  ObjManager& mObjManager;
  GameObject* mHaima;

  float mTimer = 0.f;
};
