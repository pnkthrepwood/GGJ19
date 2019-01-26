#pragma once

#include <SFML/Graphics.hpp>

#include <memory>

class DayManager {
public:
  DayManager();

  void InitNightShader(sf::RenderTarget& renderTarget);
  sf::Glsl::Vec3 pSetHSV(float h, float s, float v );

  void Update(float dt);
  void RenderWithShader(sf::RenderWindow& window, const sf::RenderTexture& renderTexture);
  void ImGuiRender();

private:

  float GetDayFactor() const;

  std::unique_ptr<sf::Shader> mNightLight;
  sf::VertexArray mQuad;
  float mElapsed;
};
