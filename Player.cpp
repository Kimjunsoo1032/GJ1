#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include "MyMath.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;
using namespace MathUtility;

Vector3 Player::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}
void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// メンバ変数への代入処理(省略)
	// ここにインゲームの初期化処理を書く
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}
void Player::Update() {

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}
	const float groundHeight = 1.0f;
	if (!onGround_) {
		if (velocity_.y < 0.0f && worldTransform_.translation_.y <= groundHeight) {
			worldTransform_.translation_.y = groundHeight;
			velocity_.y = 0.0f;
			velocity_.x *= (1.0f - kAttenuation);
			onGround_ = true;
		}
	}

	InputMove();

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.move = velocity_;

	CheckMapCollision(collisionMapInfo);
	CheckMapMove(collisionMapInfo);
	CheckMapCeiling(collisionMapInfo);
	CheckMapLanding(collisionMapInfo);
	CheckPlatformLandingXZ();
	velocity_.y -= 0.05f;    
	InputMove();              
	CheckPlatformLandingXZ();
	worldTransform_.translation_ += velocity_;
	AnimateTurn();
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}
void Player::InputMove() {

	if (onGround_) {
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			Vector3 acceleration = {};

			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}

			velocity_ += acceleration;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			velocity_.y = kJumpAcceleration;
			onGround_ = false;
		}

	} else {
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}


	if (Input::GetInstance()->PushKey(DIK_UP)) {
		worldTransform_.translation_.z+= kUpDownSpeed;
		onGround_ = false; 
	}
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		worldTransform_.translation_.z-= kUpDownSpeed;
		
	}

}
void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}
// マップ衝突判定_上
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {

	if (info.move.y <= 0) {
		return;
	}
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	MapChipField::IndexSet indexSet;
	// 左上点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右上点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;
		}
	}
}
// マップ衝突判定_下
void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.move.y >= 0)
		return;

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	auto isLandingOnPlatform = [&](uint32_t xIndex, uint32_t yIndex) -> bool {

		MapChipType cur = mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex);
		if (cur != MapChipType::kPlatform)
			return false;

		MapChipField::Rect rect = mapChipField_->GetRectByIndex(xIndex, yIndex);
		float footY = worldTransform_.translation_.y - kHeight * 0.5f;
		const float eps = 0.01f;
		if (footY >= rect.top - eps && info.move.y < 0.0f) {

			info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
			info.landing = true;
			return true;
		}
		return false;
	};

	MapChipType mapChipType, mapChipTypeNext;
	bool hit = false;
	MapChipField::IndexSet indexSet;

	auto testCorner = [&](Corner c) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[c]);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

		if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
			hit = true;
		}
		if (!hit && mapChipType == MapChipType::kPlatform) {
			if (isLandingOnPlatform(indexSet.xIndex, indexSet.yIndex)) {
				hit = true;
			}
		}
	};

	testCorner(kLeftBottom);
	testCorner(kRightBottom);

	if (hit) {
		if (!info.landing) {
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
			MapChipField::IndexSet indexSetNow;
			indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));
			if (indexSetNow.yIndex != indexSet.yIndex) {
				MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
				info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
				info.landing = true;
			}
		}
	}
}
// マップ衝突判定_右
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {

	if (info.move.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	MapChipField::IndexSet indexSet;
	// 右上点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}
// マップ衝突判定_左
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {

	if (info.move.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	MapChipField::IndexSet indexSet;
	// 左上点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左下点
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::AnimateTurn() {
	if (turnTimer_ > 0.0f) {
	}
}
KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}
void Player::CheckMapMove(const CollisionMapInfo& info) { worldTransform_.translation_ += info.move; }

void Player::CheckMapCeiling(const CollisionMapInfo& info) {
	if (info.ceiling) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

// 壁に接触している場合の処理
void Player::CheckMapWall(const CollisionMapInfo& info) {
	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// 接地状態の切り替え
void Player::CheckMapLanding(const CollisionMapInfo& info) {

	if (info.landing && velocity_.y <= 0.0f) {
		onGround_ = true;
		velocity_.x *= (1.0f - kAttenuationLanding);
		velocity_.y = 0.0f;
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;

	auto indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	onGround_ = hit;
}
AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}
void Player::CheckPlatformLandingXZ() {
	if (onGround_)
		return; 
	if (velocity_.y >= 0.0f)
		return;
	if (!mapChipField_)
		return;

	float footY = worldTransform_.translation_.y - kHeight * 0.5f;
	constexpr float kPlatformTopY = 0.05f;
	constexpr float kEps = 0.02f;

	Vector3 feet[2] = {
	    {worldTransform_.translation_.x - kWidth * 0.5f, 0.0f, worldTransform_.translation_.z},
        {worldTransform_.translation_.x + kWidth * 0.5f, 0.0f, worldTransform_.translation_.z}
    };

	const uint32_t H = mapChipField_->GetNumBlockVirtical();   // Z
	const uint32_t W = mapChipField_->GetNumBlockHorizontal(); // X

	auto isPlatformAt = [&](int ix, int iz) {
		if (ix < 0 || ix >= (int)W || iz < 0 || iz >= (int)H)
			return false;
		return mapChipField_->GetMapChipTypeByIndex(ix, iz) == MapChipType::kPlatform;
	};

	bool landing = false;

	for (Vector3 f : feet) {
		int ix = (int)std::floor(f.x + 0.5f);
		int iz = (int)std::floor(f.z + 0.5f);

		if (!isPlatformAt(ix, iz))
			continue;

		if (footY >= kPlatformTopY - kEps) {
			landing = true;
			break;
		}
	}

	if (landing) {
		worldTransform_.translation_.y = kPlatformTopY + kHeight * 0.5f;
		velocity_.y = 0.0f;
		onGround_ = true;
		velocity_.x *= (1.0f - kAttenuationLanding);
		return;
	}
}
void Player::Draw() { model_->Draw(worldTransform_, *camera_); }
