#ifndef PLAYER_H
#define PLAYER_H

#include "StandardIncludes.h"
#include "Renderer.h"
#include "SpriteSheet.h"

class Player
{
public:
	Player();
	virtual ~Player();

	void Initialize();
	void Update(float _deltaTime);
	void Render(Renderer* _renderer);

private:
	SpriteSheet* m_sprite;
	Point m_position;

	float scale;
};

#endif // PLAYER_H

