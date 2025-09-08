#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"
#include <Windows.h>
using namespace KamataEngine;
namespace {
int bgmSoundHandle = -1;
int bgmVoiceHandle = -1;
bool bgmStarted = false;

void StartBGMOnce() {
	if (bgmStarted)
		return;
	auto* audio = Audio::GetInstance();
	bgmSoundHandle = audio->LoadWave("fanfare.wav"); 
	bgmVoiceHandle = audio->PlayWave(bgmSoundHandle, true);
	bgmStarted = true;
}
void StopBGM() {
	if (bgmVoiceHandle != -1) {
		Audio::GetInstance()->StopWave(bgmVoiceHandle);
		bgmVoiceHandle = -1;
	}
}
} // namespace
GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
ClearScene* clearScene = nullptr; 

enum class Scene { kUnknown = 0, kTitle, kGame, kClear };

Scene scene = Scene::kUnknown;

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize();
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			if (gameScene->IsCleared()) { 
				scene = Scene::kClear;
				delete gameScene;
				gameScene = nullptr;
				clearScene = new ClearScene();
				clearScene->Initialize();
			} else { 
				scene = Scene::kTitle;
				delete gameScene;
				gameScene = nullptr;
				titleScene = new TitleScene();
				titleScene->Initialize();
			}
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			scene = Scene::kTitle;
			delete clearScene;
			clearScene = nullptr;
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kClear:
		clearScene->Update();
		break; 
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kClear:
		clearScene->Draw();
		break;
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize(L"6042_キム_タマイ_ナマイ_GJ1");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();


	StartBGMOnce();

	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	while (true) {
		if (KamataEngine::Update())
			break;

		ChangeScene();
		UpdateScene();

		dxCommon->PreDraw();
		DrawScene();
		dxCommon->PostDraw();
	}

	delete titleScene;
	titleScene = nullptr;
	delete gameScene;
	gameScene = nullptr;
	delete clearScene;
	clearScene = nullptr;

	KamataEngine::Finalize();
	return 0;
}