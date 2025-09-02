#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include <array>
#include <numbers>
using namespace KamataEngine;
class DeathParticles {

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	static inline const uint32_t kNumParticles = 8;
	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransform_;
	static inline const float kDuration = 1.0f;
	static inline const float kSpeed = 0.1f;
	static inline const float kAngleUnit = 2.0f * std::numbers::pi_v<float> / 12.0f;
	bool isFinished_ = false;
	float counter_ = 0.0f;
	ObjectColor objectColor_;
	Vector4 color_;
	bool IsFinished() const { return isFinished_; }

private:
	KamataEngine::Model* model_ = nullptr;        // モデルのポインタ
	KamataEngine::Camera* camera_ = nullptr;      // カメラのポインタ
};
