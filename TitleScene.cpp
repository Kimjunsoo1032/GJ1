#include "TitleScene.h"
#include "MyMath.h"
#include <numbers>

using namespace KamataEngine;
TitleScene::~TitleScene() {
	delete modelPlayer_;
	delete fade_;
	delete bgSprite_;
}

void TitleScene::Initialize() {
	camera_.Initialize();
	worldTransform_.Initialize();
	worldTransform_.scale_ = {2, 2, 2};
	worldTransform_.translation_ = {0, 8, 0};

	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.scale_ = {10, 10, 10};
	worldTransformPlayer_.translation_ = {0, -15, 0};
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float>;

	bgTexHandle_ = KamataEngine::TextureManager::Load("titleBackGround.png");
	bgSprite_ = KamataEngine::Sprite::Create(bgTexHandle_, {0, 0});
	const int kWindowWidth = 1280;
	const int kWindowHeight = 720;
	bgSprite_->SetSize({(float)kWindowWidth, (float)kWindowHeight});

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}
void TitleScene::Update() {
	switch (phase_) {
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->isFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->isFinished()) {
			finished_ = true;
		}
	}
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	rotate += 0.1f;
	worldTransformPlayer_.rotation_.y = sin(rotate) + std::numbers::pi_v<float>;
	worldTransformPlayer_.matWorld_ = MakeAffineMatrix(worldTransformPlayer_.scale_, worldTransformPlayer_.rotation_, worldTransformPlayer_.translation_);
	worldTransformPlayer_.TransferMatrix();
}
void TitleScene::Draw() {
	using namespace KamataEngine;
	auto* dx = DirectXCommon::GetInstance();

	Sprite::PreDraw(dx->GetCommandList());
	if (bgSprite_)
		bgSprite_->Draw();
	Sprite::PostDraw();


	dx->ClearDepthBuffer(); 


	Sprite::PreDraw(dx->GetCommandList());
	Sprite::PostDraw();
	fade_->Draw();
}