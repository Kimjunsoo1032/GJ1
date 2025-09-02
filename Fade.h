#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;
class Fade {
public:
	enum class Status {
		None,
		FadeIn,
		FadeOut,
	};
	void Initialize();
	void Update();
	void Draw();
	void Start(Status status,float duration);
	void Stop();
	bool isFinished() const;
	~Fade();

private:
	uint32_t textureHandle_ = 0;
	KamataEngine::Sprite* sprite_ = nullptr;
	Status status_ = Status::None;
	float duration_ = 0.0f;
	float counter_ = 0.0f;
};
