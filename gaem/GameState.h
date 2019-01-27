#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

class DayManager;
class ObjManager;
class GameObject;

class GameState {
public:
  GameState(DayManager& dayManager);

  void Update(float dt, std::function<bool()> AreAllPlayersInHaima);

  void PlaceHaimaIfPosible(ObjManager& obj_manager, sf::Vector2f pos, int& woodAmount);

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
  GameObject* mHaima;
};
