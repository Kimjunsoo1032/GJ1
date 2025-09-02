#include "ClearScene.h"
#include <cmath>
#include "MyMath.h"
using namespace KamataEngine;

ClearScene::~ClearScene() {
	delete fade_;
	if (bgSprite_) {
		delete bgSprite_;
		bgSprite_ = nullptr;
	}
}

void ClearScene::Initialize() {
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -30.0f}; // 화면에 넉넉히 들어오게 뒤로
	camera_.rotation_ = {0.0f, 0.0f, 0.0f};
	camera_.TransferMatrix();

	// (선택) 배경
	bgTexHandle_ = TextureManager::Load("clearBackGround.png");
	bgSprite_ = Sprite::Create(bgTexHandle_, {0, 0});
	bgSprite_->SetSize({1280.0f, 720.0f});


	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}
void ClearScene::Update() {

	fade_->Update();

	if (!exiting_) {
		if (fade_->isFinished() && Input::GetInstance()->PushKey(DIK_SPACE)) {
			exiting_ = true;
			fade_->Start(Fade::Status::FadeOut, 0.5f);
		}
	} else {
		if (fade_->isFinished()) {
			finished_ = true;
		}
	}
}

void ClearScene::Draw() {
	auto* dx = DirectXCommon::GetInstance();

	// (선택) 배경
	Sprite::PreDraw(dx->GetCommandList());
	if (bgSprite_)
		bgSprite_->Draw();
	Sprite::PostDraw();

	if (fade_)
		fade_->Draw();
}