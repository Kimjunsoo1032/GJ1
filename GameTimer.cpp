#include "GameTimer.h"
#include <cassert>

using namespace KamataEngine;
using namespace MathUtility;

void GameTimer::Initialize() {
	// メンバ変数への代入処理(省略)
	// ここにインゲームの初期化処理を書く
	//assert(model);
	//model_ = model;

	textureHandle_ = TextureManager::Load("number.png");
	
	for (int i = 0; i < 3; i++) {
		
		sprite_[i] = Sprite::Create(textureHandle_, {100.0f + size.x * i, 5});
		sprite_[i]->SetSize(size);
	}


}
void GameTimer::Update(uint32_t number) {

	for (int i = 0; i < 3; i++) {
		sprite_[i]->SetSize(size);
		sprite_[i]->SetTextureRect({0, 0}, size);
	}
	(void)number;

	int32_t digit = 100;
	for (int i = 0; i < 5; i++) {
		int nowNumber = number / digit;
		sprite_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
		number %= digit;
		digit /= 10;

	}



}


	void GameTimer::Draw() {
	
	for (int i = 0; i < 3; i++) {

		sprite_[i]->Draw();
	}

}