#include "Skydome.h"
#include <cassert>  
using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
}
void Skydome::Update() {
	// ここにインゲームの更新処理を書く
	worldTransform_.TransferMatrix();
}
void Skydome::Draw() { model_->Draw(worldTransform_, *camera_); };