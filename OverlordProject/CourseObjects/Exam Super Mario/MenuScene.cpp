#include "stdafx.h"

#include "MenuScene.h"
#include "GameObject.h"
#include "ContentManager.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SuperMarioScene.h"
#include "TextRenderer.h"

MenuScene::MenuScene(void) :
	GameScene(L"MenuScene")
{}

void MenuScene::Initialize()
{
	const auto gameContext = GetGameContext();
	GameObject* background = new GameObject();
	background->AddComponent(new SpriteComponent(L"./Resources/Textures/MainMenu.png", DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT4(1, 1, 1, 1.f)));
	AddChild(background);

	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_mario_press_start.wav", FMOD_2D, 0, &m_pStart);
	SoundManager::GetInstance()->GetSystem()->playSound(m_pStart, nullptr, false, &m_pChannel);
}

void MenuScene::Update()
{
	using namespace DirectX;
	const auto gameContext = GetGameContext();

	if (gameContext.pInput->IsMouseButtonDown(VK_LBUTTON) && !gameContext.pInput->IsMouseButtonDown(VK_LBUTTON, true))
	{
		POINT mousePos = InputManager::GetMousePosition();

		if (mousePos.x > 50 && mousePos.y < 519 && mousePos.x < 365 && mousePos.y > 404)
		{
			// start
			SuperMarioScene::resets++;
			auto newSceneName = L"SuperMarioScene" + std::to_wstring(SuperMarioScene::resets);
			SceneManager::GetInstance()->AddGameScene(new SuperMarioScene(newSceneName));
			SceneManager::GetInstance()->SetActiveGameScene(L"SuperMarioScene" + std::to_wstring(SuperMarioScene::resets));
		}
		else if (mousePos.x > 54 && mousePos.y < 656 && mousePos.x < 367 && mousePos.y > 542)
		{
			// quit
			PostQuitMessage(WM_QUIT);
		}
	}
}

void MenuScene::Draw()
{
}