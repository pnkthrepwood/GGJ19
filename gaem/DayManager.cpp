#include "DayManager.h"

#include "imgui.h"
#include "imgui-SFML.h"

namespace {
   const float SECONDS_OF_RAW_CICLE = 3.f;
   float SECONDS_OF_TARGET_CICLE = 180.f;
}

DayManager::DayManager()
: mQuad(sf::Quads, 4)
, mElapsed(0) {

}

void DayManager::InitNightShader(sf::RenderTarget& renderTarget) {
  mNightLight = std::unique_ptr<sf::Shader>(new sf::Shader());
  mNightLight->loadFromFile("nightLight.frag", sf::Shader::Type::Fragment);

  auto size = renderTarget.getSize();
  mQuad[0].position = sf::Vector2f(0,0);
  mQuad[1].position = sf::Vector2f(0,size.y);
  mQuad[2].position = sf::Vector2f(size.x,size.y);
  mQuad[3].position = sf::Vector2f(size.x,0);

  mQuad[0].texCoords = sf::Vector2f(0,0);
  mQuad[1].texCoords = sf::Vector2f(0,size.y);
  mQuad[2].texCoords = sf::Vector2f(size.x,size.y);
  mQuad[3].texCoords = sf::Vector2f(size.x,0);
}

sf::Glsl::Vec3 DayManager::pSetHSV(float h, float s, float v ) {
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
    return sf::Glsl::Vec3(0.2, 0.2, 0.2);
}

void DayManager::Update(float dt) {
  mElapsed += (dt / SECONDS_OF_TARGET_CICLE) * SECONDS_OF_RAW_CICLE;
}

void DayManager::RenderWithShader(sf::RenderWindow& window, const sf::RenderTexture& renderTexture) {
  sf::RenderStates states;

  mNightLight->setUniform("texture", sf::Shader::CurrentTexture);
  mNightLight->setUniform("dayTime", GetDayFactor());
  mNightLight->setUniform("day_color", sf::Glsl::Vec3(1,1,1));
  mNightLight->setUniform("sun_set_color", sf::Glsl::Vec3(1,0.5,0));
  mNightLight->setUniform("night_color", sf::Glsl::Vec3(0.2,0,1));

  states.shader = mNightLight.get();
  states.texture = &renderTexture.getTexture();
  window.draw(mQuad, states);
}

void DayManager::ImGuiRender() {
  ImGui::Begin("DayManager");
  ImGui::DragFloat("Day lenght", &SECONDS_OF_TARGET_CICLE);
  ImGui::DragFloat("Elapsed", &mElapsed);
  ImGui::End();
}

float DayManager::GetDayFactor() const {
    return 1.f-cos(1.f - std::abs(pow(sin(mElapsed),10.f))) + 0.5f;;
}
