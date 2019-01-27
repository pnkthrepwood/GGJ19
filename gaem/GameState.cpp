#include "GameState.h"

#include "ObjectManager.h"
#include "DayManager.h"


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
      else if (false) {
        mState = EState::DEAD;
      }
    } break;
    case EState::NEXT_DAY_ING:
    {
      if (mDayManager.IsFastForwarding()) {
        mState = EState::SHOWING_NEXT_DAY;
      }
    } break;
    case EState::SHOWING_NEXT_DAY:
    {
      // TODO: Animation showing next day
      mState = EState::PLAYING;
    } break;
  }

}

void GameState::PlaceHaimaIfPosible(ObjManager& obj_manager, sf::Vector2f pos, int& woodAmount) {
    if (woodAmount >= WOOD_FOR_HAIMA && !mHaima) {
      woodAmount -= WOOD_FOR_HAIMA;

      mHaima = obj_manager.Spawn(GameObjectType::HAIMA, pos.x, pos.y);
    }
}
