#include "../Game/Coin.h"
#include "../Graphics/Camera.h"
#include <random>

static CoinTextureInfo GetRandomCoinTexture()
{
	static random_device rd;
	static mt19937 gen(rd());
	static uniform_int_distribution<> dist(0, 2);
	
	int coinType = dist(gen);
	switch (coinType)
	{
		case 0: return { "../Assets/Textures/Obstacles/coin1.png", 10, CoinType::Coin1, 10 };
		case 1: return { "../Assets/Textures/Obstacles/coin2.png", 10, CoinType::Coin2, 5 };
		case 2: return { "../Assets/Textures/Obstacles/diamond.png", 5, CoinType::Diamond, 15 };
		default: return { "../Assets/Textures/Obstacles/coin1.png", 10, CoinType::Coin1, 10 };
	}
}

Coin::Coin()
{
	m_coinType = CoinType::Coin1;
	m_pointValue = 10;
}

Coin::~Coin()
{
}

void Coin::Initialize(float x, float y)
{
	SetBasePosition(x, y);
	
	m_isActive = true;
	m_currentMapInstance = 0;
	
	RandomCoin();
}

void Coin::OnRepositionAhead()
{
	RandomCoin();
}

void Coin::Reset()
{
	WorldEntity::Reset();
	RandomCoin();
}

void Coin::RandomCoin()
{
	if (m_animLoader)
	{
		delete m_animLoader;
		m_animLoader = nullptr;
	}
	
	m_animLoader = new AnimatedSpriteLoader();
	CoinTextureInfo texInfo = GetRandomCoinTexture();
	m_coinType = texInfo.type;
	m_pointValue = texInfo.pointValue;
	m_animLoader->LoadAnimation("idle", texInfo.path, 1, texInfo.frameCount, 16, 16, texInfo.frameCount, 10.0f);
}
