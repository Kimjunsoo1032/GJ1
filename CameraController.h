#pragma once
#include "KamataEngine.h"

class Player;
class CameraController {

public:
	void Initialize();
	void Update();
	Player* target_ = nullptr;
	void SetTarget(Player* target) { target_ = target; }
	void Reset();
	KamataEngine::Vector3 targetOffset_ = {0, 0, -15.0f};
	const KamataEngine::Camera& GetViewProjection() const { return camera_; }
	//
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};
	//
	//
	Rect movableArea_ = {0, 100, 0, 100};
	void SetMovableArea(Rect area) { movableArea_ = area; }
	KamataEngine::Vector3 targetPosition_;
	static inline const float kInterpolationRate = 0.1f;
	static inline const float kVelocityBias = 1.0f;
	static inline const Rect targetMargin = {-9.0f, 9.0f, -5.0f, 5.0f};
	//
private:
	KamataEngine::Camera camera_;
};
