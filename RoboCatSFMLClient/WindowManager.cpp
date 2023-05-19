#include "RoboCatClientPCH.hpp"

std::unique_ptr<sf::RenderWindow>	WindowManager::sInstance;

bool WindowManager::StaticInit()
{
	int widthScreen = sf::VideoMode::getDesktopMode().width;
	int heightScreen = sf::VideoMode::getDesktopMode().height;
	sInstance.reset(new sf::RenderWindow(sf::VideoMode(widthScreen, heightScreen), "RoboCatSFML!"));
	return true;
}


