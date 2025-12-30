#ifndef COIN_H
#define COIN_H

#include "../Game/WorldEntity.h"

enum class CoinType
{
	Coin1,    // 10 points
	Coin2,    // 5 points
	Diamond   // 15 points
};

struct CoinTextureInfo
{
	string path;
	int frameCount;
	CoinType type;
	int pointValue;
};

class Coin : public WorldEntity
{
public:
	Coin();
	virtual ~Coin();

	void Initialize(float x, float y) override;
	void Reset() override;
	
	void Collect() { m_isActive = false; }
	
	CoinType GetCoinType() const { return m_coinType; }
	int GetPointValue() const { return m_pointValue; }

protected:
	void OnRepositionAhead() override;

private:
	void RandomCoin();
	void SetCoinTexture(const CoinTextureInfo& texInfo);
	
	CoinType m_coinType;
	int m_pointValue;
};

#endif // COIN_H
