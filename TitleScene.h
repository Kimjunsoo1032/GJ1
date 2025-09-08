#pragma once
#include "KamataEngine.h"
#include "Fade.h"
class TitleScene {
public:
	~TitleScene();
	void Initialize();
	void Update();
	void Draw();
	bool IsFinished() const { return finished_; }
	Fade* fade_ = nullptr;
	enum class Phase { 
	kFadeIn ,
	kMain,
	kFadeOut,
	};
	Phase phase_ = Phase::kFadeIn;

private :
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform worldTransformPlayer_;
	KamataEngine::Model* modelPlayer_ = nullptr;       
	KamataEngine::Camera camera_; 
	bool finished_ = false;   
	float rotate = 0.0f;
	uint32_t bgTexHandle_ = 0u;
	KamataEngine::Sprite* bgSprite_ = nullptr;
};