#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include <algorithm>

using namespace KamataEngine;
using namespace MathUtility;

void CameraController::Initialize() { camera_.Initialize(); }

void CameraController::Update() {
	// 타깃 현재 위치/속도
	const WorldTransform& wt = target_->GetWorldTransform();
	const Vector3& vel = target_->GetVelocity();

	// 타깃 위치 기록(dead-zone 등에 쓸 수 있음)
	targetPosition_ = wt.translation_;

	// 따라가야 할 이상적 카메라 위치(오프셋 + 속도 바이어스)
	Vector3 desired = wt.translation_ + targetOffset_ + vel * kVelocityBias;

	// ✨ X, Y 모두 부드럽게 추적 (Z는 오프셋 고정)
	camera_.translation_.x = Lerp(camera_.translation_.x, desired.x, kInterpolationRate);
	camera_.translation_.y = Lerp(camera_.translation_.y, desired.y, kInterpolationRate);
	camera_.translation_.z = desired.z; // 필요하면 Lerp도 가능

	// 이동 범위 제한 (헤더의 movableArea_):contentReference[oaicite:2]{index=2}
	camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);
	camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);

	camera_.UpdateMatrix();
}

void CameraController::Reset() {
	const WorldTransform& wt = target_->GetWorldTransform();
	camera_.translation_ = wt.translation_ + targetOffset_;
	camera_.UpdateMatrix();
}