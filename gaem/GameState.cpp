#include "GameState.h"

#include "ObjectManager.h"
#include "DayManager.h"

extern sf::Font* font;
extern float RES_Y;
extern float RES_X;

namespace {
  const int WOOD_FOR_HAIMA = 1;
}

GameState::GameState(DayManager& dayManager)
: mState(EState::PLAYING)
, mDayManager(dayManager)
, mHaima(nullptr)
{

}


void GameState::Update(float dt, std::function<bool()> AreAllPlayersInHaima) {
  switch (mState) {
    case EState::PLAYING:
    {
      if (AreAllPlayersInHaima()) {
        mState = EState::NEXT_DAY_ING;
        mDayManager.FastForwardUntilNextMorning();
      }
      else if (mDayManager.GetDayTime() < 0.1f) {
        mState = EState::DEAD;
      }
    } break;
    case EState::NEXT_DAY_ING:
    {
      if (!mDayManager.IsFastForwarding()) {
        mState = EState::SHOWING_NEXT_DAY;
      }
    } break;
    case EState::SHOWING_NEXT_DAY:
    {
      // TODO: Animation showing next day
      mState = EState::PLAYING;
    } break;
    case EState::DEAD:
    {

    } break;
  }

}

void GameState::PlaceHaimaIfPosible(ObjManager& obj_manager, sf::Vector2f pos, int& woodAmount) {
    if (woodAmount >= WOOD_FOR_HAIMA && !mHaima) {
      woodAmount -= WOOD_FOR_HAIMA;

      mHaima = obj_manager.Spawn(GameObjectType::HAIMA, pos.x, pos.y);
    }
}


void GameState::Render(sf::RenderWindow& window) {
    switch (mState) {
      case EState::PLAYING:
      {
        sf::Text txt_day;
        txt_day.setFont(*font);
        sf::String str = "Day " + std::to_string(int(mDayManager.GetElapsedDays()));
        txt_day.setString(str);
        txt_day.setFillColor(sf::Color::White);
        txt_day.setOutlineColor(sf::Color::Black);
        txt_day.setOutlineThickness(2);
        txt_day.setCharacterSize(32);
        sf::FloatRect textRect = txt_day.getLocalBounds();
        txt_day.setPosition(RES_X / 2, RES_Y - textRect.height - 10);
        txt_day.setOrigin(textRect.width/2, 0);
        window.draw(txt_day);
      } break;
      case EState::NEXT_DAY_ING:
      {
      } break;
      case EState::SHOWING_NEXT_DAY:
      {
      } break;
      case EState::DEAD:
      {
      } break;
    }
}


bool GameState::IsPlaying() const {
  return mState == EState::PLAYING;
}
