#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

struct InputManager 
{

	static bool IsMousePressed(sf::Mouse::Button b = sf::Mouse::Left) 
	{
		return (button_states[b] == PRESSED || button_states[b] == JUST_PRESSED);
	}
	
	static bool IsMouseJustPressed(sf::Mouse::Button b = sf::Mouse::Left) 
	{
		return (button_states[b] == JUST_PRESSED);
	}
	
	static bool IsMouseReleased(sf::Mouse::Button b = sf::Mouse::Left) 
	{
		return (button_states[b] == RELEASED || button_states[b] == JUST_RELEASED);
	}
	
	static bool IsMouseJustReleased(sf::Mouse::Button b = sf::Mouse::Left) 
	{
		return (button_states[b] == JUST_RELEASED);
	}

     static void UpdateInputState()
     {
        for (int i = 0; i < sf::Mouse::ButtonCount; i++) 
		{
			if (sf::Mouse::isButtonPressed((sf::Mouse::Button)i)) {
				if (button_states[i] == JUST_PRESSED || button_states[i] == PRESSED) 
				{
					button_states[i] = PRESSED;
				} 
				else 
				{
					button_states[i] = JUST_PRESSED;
				}
			} 
			else 
			{
				if (button_states[i] == JUST_RELEASED || button_states[i] == RELEASED) 
				{
					button_states[i] = RELEASED;
				} 
				else 
				{
					button_states[i] = JUST_RELEASED;
				}
			}
		}
	}

private:
	InputManager() { }

	enum KeyStates { JUST_RELEASED, RELEASED, JUST_PRESSED, PRESSED };
	
	static KeyStates button_states[sf::Mouse::ButtonCount];
};
