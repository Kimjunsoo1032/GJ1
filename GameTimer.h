#pragma once
#include "KamataEngine.h"
#include "MyMath.h"


class GameTimer {
public:
	void Initialize();
	void Update(uint32_t number,uint32_t bestNumber);
	void Draw();

	KamataEngine::Vector2 size = {32.0f, 64.0f};

private:

	KamataEngine::Sprite* sprite_[3] = {};
	KamataEngine::Sprite* bestSprite_[3] = {};

	uint32_t textureHandle_ = 0;
};
