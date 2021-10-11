#pragma once
#include <GameScene.h>
class SpriteFont;

class MenuScene : public GameScene
{
public:
	MenuScene();
	virtual ~MenuScene() = default;

	MenuScene(const MenuScene& other) = delete;
	MenuScene(MenuScene&& other) noexcept = delete;
	MenuScene& operator=(const MenuScene& other) = delete;
	MenuScene& operator=(MenuScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	FMOD::Sound* m_pStart = nullptr;
	FMOD::Channel* m_pChannel = nullptr;
};
