#pragma once
#include "Fade.h"
#include "KamataEngine.h"

class ClearScene {
public:
	~ClearScene();
	void Initialize();
	void Update();
	void Draw();
	bool IsFinished() const { return finished_; }

private:
	KamataEngine::Camera camera_;
	uint32_t bgTexHandle_ = 0u;
	KamataEngine::Sprite* bgSprite_ = nullptr;
	Fade* fade_ = nullptr;

	bool exiting_ = false;
	bool finished_ = false;
};