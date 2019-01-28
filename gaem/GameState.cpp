#include "GameState.h"

#include "ObjectManager.h"
#include "DayManager.h"
#include <SFML/Audio.hpp>

extern sf::Font* font;
extern float RES_Y;
extern float RES_X;

namespace 
{
  int WOOD_FOR_HAIMA = 1;

  void DrawText(sf::RenderWindow& window, const std::string& str, sf::Vector2f pos, int size) {
    sf::Text txt_day;
    txt_day.setFont(*font);
    txt_day.setString(str);
    txt_day.setFillColor(sf::Color::White);
    txt_day.setOutlineColor(sf::Color::Black);
    txt_day.setOutlineThickness(2);
    txt_day.setCharacterSize(size);
    sf::FloatRect textRect = txt_day.getLocalBounds();
    txt_day.setPosition(pos.x, pos.y - textRect.height);
    txt_day.setOrigin(textRect.width/2, 0);
    window.draw(txt_day);
  }
}

GameState::GameState(DayManager& dayManager, ObjManager& obj_manager)
: mState(EState::PLAYING)
, mDayManager(dayManager)
, mObjManager(obj_manager)
, mHaima(nullptr)
{

}
#include <iostream>
extern sf::Sound* mussol;
extern sf::Sound* grills;
void GameState::Update(float dt, std::function<bool()> AreAllPlayersInHaima) {
  if (mDayManager.GetDayTime() > 0.72f && mDayManager.GetDayTime() < 0.77f && grills->getStatus() != sf::Sound::Status::Playing) {
	  grills->play();
  }

  switch (mState) {
    case EState::PLAYING:
    {
      if (AreAllPlayersInHaima()) {
        mState = EState::NEXT_DAY_ING;
		mussol->play();
        mDayManager.FastForwardUntilNextMorning();
      }
      else if (mDayManager.GetDayTime() < 0.1f) {
        mState = EState::DEAD;
		mussol->play();
      }
    } break;
    case EState::NEXT_DAY_ING:
    {
      if (!mDayManager.IsFastForwarding()) {
        mState = EState::SHOWING_NEXT_DAY;
        mTimer = 1.f;
      }
    } break;
    case EState::SHOWING_NEXT_DAY:
    {
      mTimer -= dt;
      if (mTimer < 0) {
        mState = EState::PLAYING;
        mHaima->type = GameObjectType::HAIMA_BROKEN;
        mHaima = nullptr;
      }
    } break;
    case EState::DEAD:
    {

    } break;
  }

}

extern sf::Sound* hammer;
void GameState::PlaceHaimaIfPosible(sf::Vector2f pos, int& woodAmount, GameObject* oasis) 
{
    if (woodAmount >= WOOD_FOR_HAIMA && !mHaima) 
	{
      woodAmount -= WOOD_FOR_HAIMA;
      mHaima = mObjManager.Spawn(GameObjectType::HAIMA, pos.x, pos.y);
      oasis->type = GameObjectType::WATER_BROKEN;
	  hammer->play();
	  WOOD_FOR_HAIMA += 1;
    }
}


void GameState::Render(sf::RenderWindow& window) {
    switch (mState) {
      case EState::PLAYING:
      {
        sf::String str = "Day " + std::to_string(int(mDayManager.GetElapsedDays()));
        sf::Vector2f pos(RES_X / 2, RES_Y - 10);
        DrawText(window, str, pos, 32);
      } break;
      case EState::NEXT_DAY_ING:
      {
      } break;
      case EState::SHOWING_NEXT_DAY:
      {
        sf::String str = "Day " + std::to_string(int(mDayManager.GetElapsedDays()));
        DrawText(window, str, sf::Vector2f(RES_X / 2, RES_Y/2), 55);
      } break;
      case EState::DEAD:
      {
        sf::String str = "You survived until\n         Day " + std::to_string(int(mDayManager.GetElapsedDays())-1);
        DrawText(window, str, sf::Vector2f(RES_X / 2, RES_Y/2), 55);
      } break;
    }
}


bool GameState::IsPlaying() const {
  return mState == EState::PLAYING;
}
