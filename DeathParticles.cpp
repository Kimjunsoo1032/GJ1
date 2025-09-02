#include "DeathParticles.h"
#include "MyMath.h"
#include <algorithm>
#include <cassert>
#include <numbers>
using namespace KamataEngine;
using namespace MathUtility;
void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	for (WorldTransform& worldTransform : worldTransform_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}
	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};
}

void DeathParticles::Update() {
	for (WorldTransform& worldTransform : worldTransform_) {
		worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
		worldTransform.TransferMatrix();
	}
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		Vector3 velocity = {kSpeed, 0.0f, 0.0f};
		float angle = kAngleUnit * i;
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		velocity = Transform(velocity, matrixRotation);
		worldTransform_[i].translation_ += velocity;
	}
	
	counter_ += 1.0f / 60.0f;

	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinished_ = true;
	}

	if (isFinished_) {
		return;
	}
	color_.w = std::clamp(1.0f-counter_/kDuration, 0.0f, 1.0f);
	objectColor_.SetColor(color_);
}

void DeathParticles::Draw() { 
	
	for (WorldTransform& worldTransform : worldTransform_) {
		model_->Draw(worldTransform, *camera_,&objectColor_);
	}
	if (isFinished_) {
		return;
	}
}

