class Server : public Engine
{
public:

	static bool StaticInit();

	virtual void DoFrame() override;

	virtual int Run();

	void HandleNewClient(ClientProxyPtr inClientProxy);
	void HandleLostClient(ClientProxyPtr inClientProxy);

	RoboCatPtr	GetCatForPlayer(int inPlayerId);
	void	SpawnCatForPlayer(int inPlayerId);
	void SpawnAsteroides(int nbAsteroides);
	sf::Vector2f GetRandomPosition(int size, std::vector<sf::Vector2f> existingAsteroides, std::vector<int> existingAsteroidesSize);




private:
	Server();

	bool	InitNetworkManager();
	void	SetupWorld();

};

