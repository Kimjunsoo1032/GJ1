#define NOMINMAX
#include "GameScene.h"
#include "MyMath.h"
#include <Windows.h>
#include <cassert>  
using namespace KamataEngine;

GameScene::~GameScene() {
	delete model_;
	delete player_;
	delete debugCamera_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete modelSkydome_;
	delete mapChipField_;
	delete cameraController_;
}
inline Matrix4x4 Mul(const Matrix4x4& A, const Matrix4x4& B) {
	Matrix4x4 R{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			R.m[i][j] = A.m[i][0] * B.m[0][j] + A.m[i][1] * B.m[1][j] + A.m[i][2] * B.m[2][j] + A.m[i][3] * B.m[3][j];
		}
	}
	return R;
}
namespace {
inline bool WorldToScreen(const KamataEngine::Vector3& wp, const KamataEngine::Camera& cam, float& outX, float& outY, float screenW = 1280.0f, float screenH = 720.0f) {
	using namespace KamataEngine;
	Matrix4x4 vp = Mul(cam.matView, cam.matProjection);


	float x = wp.x, y = wp.y, z = wp.z;
	float X = x * vp.m[0][0] + y * vp.m[1][0] + z * vp.m[2][0] + 1.0f * vp.m[3][0];
	float Y = x * vp.m[0][1] + y * vp.m[1][1] + z * vp.m[2][1] + 1.0f * vp.m[3][1];
	float W = x * vp.m[0][3] + y * vp.m[1][3] + z * vp.m[2][3] + 1.0f * vp.m[3][3];

	if (W == 0.0f)
		return false;
	float ndcX = X / W;
	float ndcY = Y / W;

	outX = (ndcX * 0.5f + 0.5f) * screenW;
	outY = (-ndcY * 0.5f + 0.5f) * screenH;
	return true;
}
} // namespace

void GameScene::Initialize() {

	model_ = Model::Create();
	modelPlayer_ = Model::CreateFromOBJ("player");
	modelBlock_ = Model::CreateFromOBJ("block");
	modelDeathParticles_ = Model::CreateFromOBJ("deathParticle");
	assert(model_);

	worldTransform_.Initialize();
	camera_.Initialize();


	player_ = new Player();

	debugCamera_ = new DebugCamera(1280, 720);

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	cameraController_ = new CameraController();
	cameraController_->Initialize();
	cameraController_->SetTarget(player_);
	float mapBottom = 0.0f;
	float mapTop = (mapChipField_->GetNumBlockVirtical() - 1) * 1.0f;

	CameraController::Rect cameraArea = {12.0f, 100.0f - 12.0f, mapBottom + 0.5f, mapTop - 0.5f};

	cameraController_->targetOffset_.y = 3.0f;
	cameraController_->targetOffset_.z = -18.0f;
	cameraController_->SetMovableArea(cameraArea);

	cameraController_->Reset();

	player_->SetMapChipField(mapChipField_);

	auto worldOf = [&](uint32_t x, uint32_t y) { return mapChipField_->GetMapChipPositionByIndex(x, y); };

	struct Quad {
		uint32_t x0, y0, x1, y1;
	};
	// 맵 중앙 좌표를 대략적으로 사용
	uint32_t H = mapChipField_->GetNumBlockVirtical();
	uint32_t W = mapChipField_->GetNumBlockHorizontal();
	uint32_t midY = H / 2;
	uint32_t midX = W / 2;
	std::vector<Quad> quads = {
	    {1,        1,        midX - 2, midY - 2}, // 좌상
	    {midX + 1, 1,        W - 2,    midY - 2}, // 우상
	    {1,        midY + 1, midX - 2, H - 2   }, // 좌하
	    {midX + 1, midY + 1, W - 2,    H - 2   }  // 우하
	};

	Vector3 bottomLeftPos = portals_[2].worldPos; 
	Vector3 topLeftPos = portals_[0].worldPos;   


	Vector3 targetTopLeft = topLeftPos;
	targetTopLeft.x -= 2.0f; 

	portals_[2].targetPos = targetTopLeft;

	phase_ = Phase::kFadeIn;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}
void GameScene::Update() {
	switch (phase_) {
	case Phase::kPlay:
		break;
	case Phase::kDeath:
		if (deathParticles_)
			deathParticles_->Update();
		break;
	case Phase::kFadeIn:
		if (fade_) {
			fade_->Update();
			if (fade_->isFinished())
				phase_ = Phase::kPlay;
		}
		break;
	case Phase::kFadeOut:
		if (fade_) {
			fade_->Update();
			if (fade_->isFinished())
				finished_ = true;
		}
		break;
	}

	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
			wt->TransferMatrix();
		}
	}

	player_->Update();

	const Vector3 p = player_->GetWorldPosition();

	if (!camAnchorInited_) {
		camAnchor_ = p;
		camAnchorInited_ = true;
	}

	const float deadzone = 0.05f;
	const float followK = 0.08f;
	Vector3 delta = {p.x - camAnchor_.x, p.y - camAnchor_.y, p.z - camAnchor_.z};
	float dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
	if (dist2 > deadzone * deadzone) {
		camAnchor_.x += delta.x * followK;
		camAnchor_.y += delta.y * followK;
		camAnchor_.z += delta.z * followK;
	}

	Vector3 target = camAnchor_;
	Vector3 eye = {target.x - 5.0f, target.y + 8.0f, target.z - 8.0f};

	camera_.translation_.x += (eye.x - camera_.translation_.x) * 0.12f;
	camera_.translation_.y += (eye.y - camera_.translation_.y) * 0.12f;
	camera_.translation_.z += (eye.z - camera_.translation_.z) * 0.12f;

	Vector3 dir = {target.x - camera_.translation_.x, target.y - camera_.translation_.y, target.z - camera_.translation_.z};
	float lenXZ = std::sqrt(dir.x * dir.x + dir.z * dir.z);
	float yaw = std::atan2(dir.x, dir.z);
	float pitch = -std::atan2(dir.y, lenXZ);

	auto LerpAngle = [](float a, float b, float t) {
		float d = b - a;
		if (d > 3.14159265f)
			d -= 6.28318531f;
		if (d < -3.14159265f)
			d += 6.28318531f;
		return a + d * t;
	};
	camera_.rotation_.y = LerpAngle(camera_.rotation_.y, yaw, 0.15f);
	camera_.rotation_.x = LerpAngle(camera_.rotation_.x, pitch, 0.15f);

	camera_.UpdateMatrix();

	if (skydome_)
		skydome_->Update();
	if (deathParticles_)
		deathParticles_->Update();
	if (fade_)
		fade_->Update();

	ChangePhase();
}
void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());
	if (phase_ == Phase::kPlay || phase_ == Phase::kFadeIn) {
		player_->Draw();
	}
	skydome_->Draw();

	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			if (!wt)
				continue;
			modelBlock_->Draw(*wt, camera_);
		}
	}
	if (deathParticles_) {
		deathParticles_->Draw();
	}
	Model::PostDraw();

	Sprite::PreDraw(dxCommon->GetCommandList());


	for (auto& p : portals_) {
		if (!p.sprite)
			continue;
		float sx = 0, sy = 0;
		if (WorldToScreen(p.worldPos, camera_, sx, sy)) {
			p.sprite->SetPosition({sx - 64.0f, sy - 64.0f});
			p.sprite->Draw();
		}
	}

	Sprite::PostDraw();

	fade_->Draw();
}
void GameScene::GenerateBlocks() {
	if (!worldTransformBlocks_.empty()) {
		for (auto& line : worldTransformBlocks_) {
			for (auto* wt : line)
				delete wt;
			line.clear();
		}
		worldTransformBlocks_.clear();
	}

	const uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();     // → Z 길이
	const uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal(); // → X 길이

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t z = 0; z < numBlockVirtical; ++z) {
		worldTransformBlocks_[z].resize(numBlockHorizontal, nullptr);
	}

	for (uint32_t z = 0; z < numBlockVirtical; ++z) {
		for (uint32_t x = 0; x < numBlockHorizontal; ++x) {
			MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, z);
			if (t == MapChipType::kBlank)
				continue;

			WorldTransform* wt = new WorldTransform();
			wt->Initialize();

			wt->translation_ = {(float)x, 0.0f, (float)z};

			if (t == MapChipType::kPlatform) {
			
				wt->scale_ = {1.0f, 0.1f, 1.0f};
			}

			worldTransformBlocks_[z][x] = wt;
		}
	}
}
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			if (deathParticles_) {
				delete deathParticles_;
				deathParticles_ = nullptr;
			}
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticles_, &camera_, deathParticlesPosition);
		}
		break;

	case Phase::kDeath:
		deathParticles_->Update();
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	}
}
void GameScene::CheckPortalCollision() {
	if (!player_)
		return;

	const Vector3 playerPos = player_->GetWorldPosition();

	for (int idx = 0; idx < (int)portals_.size(); ++idx) {
		auto& portal = portals_[idx];

		Vector3 diff = {playerPos.x - portal.worldPos.x, playerPos.y - portal.worldPos.y, playerPos.z - portal.worldPos.z};
		float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (distSq >= (portal.size * portal.size))
			continue;

		const uint32_t H = mapChipField_->GetNumBlockVirtical();
		const uint32_t W = mapChipField_->GetNumBlockHorizontal();
		const uint32_t midY = H / 2;
		const uint32_t midX = W / 2;

		auto worldOf = [&](uint32_t x, uint32_t y) { return mapChipField_->GetMapChipPositionByIndex(x, y); };

		const float playerHalfW = Player::kWidth * 0.5f;
		const float playerHalfH = Player::kHeight * 0.5f;
		const float margin = 0.1f;

		if (idx == 2) {
			uint32_t x0 = 1, y0 = 1;
			uint32_t x1 = midX - 2, y1 = midY - 2;

			auto p00 = worldOf(x0, y0);
			auto p11 = worldOf(x1, y1);
			float leftEdge = std::min(p00.x, p11.x) - 0.5f;
			uint32_t landX = x0;

			auto findFloorYInTopLeft = [&](uint32_t xIndex) -> float {
				for (int y = (int)y1; y >= (int)y0; --y) {
					auto cur = mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)y);
					auto above = (y > 0) ? mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)(y - 1)) : MapChipType::kBlank;
					if (cur == MapChipType::kBlock && above != MapChipType::kBlock) {
						auto rect = mapChipField_->GetRectByIndex(xIndex, (uint32_t)y);
						return rect.top;
					}
				}
				auto midP = worldOf(landX, (y0 + y1) / 2);
				return midP.y;
			};

			float floorY = findFloorYInTopLeft(landX);
			Vector3 dst = {leftEdge + playerHalfW + margin, floorY + playerHalfH, portal.worldPos.z};

			player_->SetPosition(dst);
			player_->velocity_ = {};
			cameraController_->Reset();
			return;
		}

		if (idx == 0) {
			uint32_t x0 = midX + 1, y0 = 1;
			uint32_t x1 = W - 2, y1 = midY - 2;

			auto p00 = worldOf(x0, y0);
			auto p11 = worldOf(x1, y1);
			float leftEdge = std::min(p00.x, p11.x) - 0.5f;
			uint32_t landX = x0;

			auto findFloorYInTopRight = [&](uint32_t xIndex) -> float {
				for (int y = (int)y1; y >= (int)y0; --y) {
					auto cur = mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)y);
					auto above = (y > 0) ? mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)(y - 1)) : MapChipType::kBlank;
					if (cur == MapChipType::kBlock && above != MapChipType::kBlock) {
						auto rect = mapChipField_->GetRectByIndex(xIndex, (uint32_t)y);
						return rect.top;
					}
				}
				auto midP = worldOf(landX, (y0 + y1) / 2);
				return midP.y;
			};

			float floorY = findFloorYInTopRight(landX);
			Vector3 dst = {leftEdge + playerHalfW + margin, floorY + playerHalfH, portal.worldPos.z};

			player_->SetPosition(dst);
			player_->velocity_ = {};
			cameraController_->Reset();
			return;
		}
//
		if (idx == 1) {
			uint32_t x0 = midX + 1, y0 = midY + 1;
			uint32_t x1 = W - 2, y1 = H - 2;

			auto p00 = worldOf(x0, y0);
			auto p11 = worldOf(x1, y1);
			float leftEdge = std::min(p00.x, p11.x) - 0.5f;
			uint32_t landX = x0;

			auto findFloorYInBottomRight = [&](uint32_t xIndex) -> float {
				for (int y = (int)y1; y >= (int)y0; --y) {
					auto cur = mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)y);
					auto above = (y > 0) ? mapChipField_->GetMapChipTypeByIndex(xIndex, (uint32_t)(y - 1)) : MapChipType::kBlank;
					if (cur == MapChipType::kBlock && above != MapChipType::kBlock) {
						auto rect = mapChipField_->GetRectByIndex(xIndex, (uint32_t)y);
						return rect.top;
					}
				}
				auto midP = worldOf(landX, (y0 + y1) / 2);
				return midP.y;
			};

			float floorY = findFloorYInBottomRight(landX);
			Vector3 dst = {leftEdge + playerHalfW + margin, floorY + playerHalfH, portal.worldPos.z};

			player_->SetPosition(dst);
			player_->velocity_ = {};
			cameraController_->Reset();
			return;
		}

//
		if (idx == 3) {
			cleared_ = true;
			phase_ = Phase::kFadeOut;
			if (fade_)
				fade_->Start(Fade::Status::FadeOut, 1.0f);
			cameraController_->Reset();
			return;
		}
	}
}