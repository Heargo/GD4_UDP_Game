
#include "RoboCatServerPCH.hpp"
#include <iostream>



//uncomment this when you begin working on the server

bool Server::StaticInit()
{
	s_instance.reset(new Server());

	return true;
}

Server::Server()
{

	GameObjectRegistry::sInstance->RegisterCreationFunction('RCAT', RoboCatServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('MOUS', MouseServer::StaticCreate);
	GameObjectRegistry::sInstance->RegisterCreationFunction('YARN', YarnServer::StaticCreate);

	InitNetworkManager();

	// Setup latency
	float latency = 0.0f;
	string latencyString = StringUtils::GetCommandLineArg(2);
	if (!latencyString.empty())
	{
		latency = stof(latencyString);
	}
	NetworkManagerServer::sInstance->SetSimulatedLatency(latency);
}


int Server::Run()
{
	SetupWorld();

	return Engine::Run();
}

bool Server::InitNetworkManager()
{
	string portString = StringUtils::GetCommandLineArg(1);
	uint16_t port = stoi(portString);

	return NetworkManagerServer::StaticInit(port);
}


namespace
{

	void CreateRandomMice(int inMouseCount)
	{
		Vector3 mouseMin(100.f, 100.f, 0.f);
		Vector3 mouseMax(1180.f, 620.f, 0.f);
		GameObjectPtr go;

		//make a mouse somewhere- where will these come from?
		for (int i = 0; i < inMouseCount; ++i)
		{
			go = GameObjectRegistry::sInstance->CreateGameObject('MOUS');
			Vector3 mouseLocation = RoboMath::GetRandomVector(mouseMin, mouseMax);
			go->SetLocation(mouseLocation);
		}
	}


}


void Server::SetupWorld()
{
	//spawn some random mice
	//CreateRandomMice(10);
	SpawnAsteroides(10);

	//spawn more random mice!
	//CreateRandomMice(10);
}

void Server::DoFrame()
{
	NetworkManagerServer::sInstance->ProcessIncomingPackets();

	NetworkManagerServer::sInstance->CheckForDisconnects();

	NetworkManagerServer::sInstance->RespawnCats();

	Engine::DoFrame();

	NetworkManagerServer::sInstance->SendOutgoingPackets();

}

void Server::HandleNewClient(ClientProxyPtr inClientProxy)
{

	int playerId = inClientProxy->GetPlayerId();
	LOG("Player %d has joined", playerId)

	ScoreBoardManager::sInstance->AddEntry(playerId, inClientProxy->GetName());
	SpawnCatForPlayer(playerId);
}

void Server::SpawnCatForPlayer(int inPlayerId)
{
	RoboCatPtr cat = std::static_pointer_cast<RoboCat>(GameObjectRegistry::sInstance->CreateGameObject('RCAT'));
	cat->SetColor(ScoreBoardManager::sInstance->GetEntry(inPlayerId)->GetColor());
	cat->SetPlayerId(inPlayerId);
	cat->SetTeam(inPlayerId % 2);
	//gotta pick a better spawn location than this...
	cat->SetLocation(Vector3(600.f - static_cast<float>(inPlayerId), 400.f, 0.f));
}

void Server::HandleLostClient(ClientProxyPtr inClientProxy)
{
	//kill client's cat
	//remove client from scoreboard
	int playerId = inClientProxy->GetPlayerId();

	ScoreBoardManager::sInstance->RemoveEntry(playerId);
	RoboCatPtr cat = GetCatForPlayer(playerId);
	if (cat)
	{
		cat->SetDoesWantToDie(true);
	}
}

RoboCatPtr Server::GetCatForPlayer(int inPlayerId)
{
	//run through the objects till we find the cat...
	//it would be nice if we kept a pointer to the cat on the clientproxy
	//but then we'd have to clean it up when the cat died, etc.
	//this will work for now until it's a perf issue
	const auto& gameObjects = World::sInstance->GetGameObjects();
	for (int i = 0, c = gameObjects.size(); i < c; ++i)
	{
		GameObjectPtr go = gameObjects[i];
		RoboCat* cat = go->GetAsCat();
		if (cat && cat->GetPlayerId() == inPlayerId)
		{
			return std::static_pointer_cast<RoboCat>(go);
		}
	}

	return nullptr;

}

void Server::SpawnAsteroides(int nbAsteroides)
{
	//hardcoded seed for now 
	srand(1);

	////list of existing asteroides position and size
	std::vector<sf::Vector2f> existingAsteroides;
	std::vector<int> existingAsteroidesSize;

	GameObjectPtr go;
	
	//Spawn the asteroids
	for (int i = 0; i < nbAsteroides; i++)
	{
		//get a random size between 50px and 200px with a step of 50px
		int size = 50 * (rand() % 4 + 1);

		sf::Vector2f pos = GetRandomPosition(size, existingAsteroides, existingAsteroidesSize);

		//skip if there is no valid position for this size. Get random position return a (0,0) if there where to many attemps 
		if (pos == sf::Vector2f(0.f, 0.f)) continue;

		//add pos and size to history
		existingAsteroides.push_back(pos);
		existingAsteroidesSize.push_back(size);

		go = GameObjectRegistry::sInstance->CreateGameObject('MOUS');
		Vector3 mouseLocation = Vector3(pos.x, pos.y, 0.f);
		go->SetLocation(mouseLocation);


	}
}

sf::Vector2f Server::GetRandomPosition(int size, std::vector<sf::Vector2f> existingAsteroides, std::vector<int> existingAsteroidesSize)
{

	bool tooClose = false;
	//margin in px
	int margin = 30;

	sf::Rect<float> m_world_bounds = sf::Rect<float>(100.f, 100.f, 1180.f, 620.f);
	

	//position is within word size
	int marginWorld = 50;

	//random position
	int x = (rand() % int(m_world_bounds.width - margin)) + margin;
	int y = ((rand() % int(m_world_bounds.height - margin)) + margin);


	int attemps = 0;
	//while the position is too close from an existing asteriod, we try other position within 100 attemps limit.
	do {
		for (int j = 0; j < existingAsteroides.size(); j++)
		{
			float distance = std::sqrt((existingAsteroides[j].x - x) * (existingAsteroides[j].x - x) + (existingAsteroides[j].y - y) * (existingAsteroides[j].y - y));
			if (distance <= (existingAsteroidesSize[j] + size + margin))
			{
				tooClose = true;
				x = (rand() % int(m_world_bounds.width - margin)) + margin;
				y = ((rand() % int(m_world_bounds.height - margin)) + margin);
				//std::cout << "point too close !!!" << std::endl;
				break;
			}
			else
			{
				tooClose = false;
				//std::cout << " pos is ok compared to asteroid " << j << " it's " << distance << " from it" << std::endl;
			}
		}
		attemps++;
	} while (tooClose && attemps < 100);

	//return (0,0) position if there is no room for the asteroid of the size given.
	if (attemps == 100)
	{
		x = 0.f;
		y = 0.f;
	}

	return sf::Vector2f(x, y);



}