#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>
#include "MapChipField.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
class GameScene {

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera camera_;

	Player* player_ = nullptr;
	Skydome* skydome_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;
	MapChipField* mapChipField_;
	CameraController* cameraController_;
	DeathParticles* deathParticles_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;
	enum class Phase {
		kFadeIn,
		kPlay,
		kDeath,
		kFadeOut,
	};
	Phase phase_;


	uint32_t portalTexture = 0u;
	struct Portal {
		KamataEngine::Vector3 worldPos;
		KamataEngine::Vector3 targetPos{};
		KamataEngine::Sprite* sprite = nullptr;
		float size = 0.9f;
	};
	std::array<Portal, 4> portals_; 
	void CheckPortalCollision();


	bool finished_ = false;
	Fade* fade_ = nullptr;
	bool cleared_ = false; 






public:
	~GameScene();
	void Initialize();
	void Update();
	void Draw();
	void GenerateBlocks();
	void ChangePhase();
	bool IsFinished() const { return finished_; }
	bool IsCleared() const { return cleared_; }

	KamataEngine::Vector3 camAnchor_ = {0, 0, 0};
	bool camAnchorInited_ = false;
};
