#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
class MapChipField;
class Player {

	

	public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	void InputMove();
	void AnimateTurn();
	KamataEngine::Vector3 velocity_ = {};

	KamataEngine::Camera* camera_ = nullptr;
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.1f;
	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;
	bool onGround_ = true;
	static inline const float kGravityAcceleration = 0.1f;
	static inline const float kLimitFallSpeed = 0.5f;
	static inline const float kJumpAcceleration = 1.0f;

    const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	float turnTimer_ = 0.0f;

	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall= false;
		KamataEngine::Vector3 move = {};
	};

	//マップ衝突判定
	void CheckMapCollision(CollisionMapInfo& info);
	// マップ衝突判定_上
	void CheckMapCollisionUp(CollisionMapInfo& info);
	// マップ衝突判定_下
	void CheckMapCollisionDown(CollisionMapInfo& info);
	// マップ衝突判定_右
	void CheckMapCollisionRight(CollisionMapInfo& info);
	// マップ衝突判定_左
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCorner
	};
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);
	static inline const float kBlank = 0.1f;
	//判定結果を反映して移動させる
	void CheckMapMove(const CollisionMapInfo& info);
	//天井に接触している場合の処理
	void CheckMapCeiling(const CollisionMapInfo& info);
	//壁に接触している場合の処理
	void CheckMapWall(const CollisionMapInfo& info);
	////接地状態の切り替え
	void CheckMapLanding(const CollisionMapInfo& info);

	static inline const float kAttenuationLanding = 0.5f;
	static inline const float kGroundSearchHeight = 0.3f;
	static inline const float kAttenuationWall = 0.5f;
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();
	bool isDead_ = false;
	bool IsDead() const { return isDead_; };
	void SetPosition(const KamataEngine::Vector3& pos) { worldTransform_.translation_ = pos; }
	void CheckPlatformLandingXZ();
	static constexpr float kUpDownSpeed = 0.12f;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
};
