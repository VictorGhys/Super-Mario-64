#pragma once
#include <GameScene.h>
#include "GameObject.h"
#include "SpriteFont.h"

class ParticleEmitterComponent;
class Character;

class SuperMarioScene final : public GameScene
{
public:
	SuperMarioScene(const std::wstring& name);
	virtual ~SuperMarioScene();

	SuperMarioScene(const SuperMarioScene& other) = delete;
	SuperMarioScene(SuperMarioScene&& other) noexcept = delete;
	SuperMarioScene& operator=(const SuperMarioScene& other) = delete;
	SuperMarioScene& operator=(SuperMarioScene&& other) noexcept = delete;
	void AddCoin();
	void AddStar();
	void AddToDelete(GameObject* go);
	void RemoveLives(int lives);
	static int resets;
	void SpawnCoinDelayed(DirectX::XMFLOAT3 pos);
	void PlayGoombaSound();
protected:
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	Character* m_pMario = nullptr;
	std::vector<GameObject*> m_Goombas;
	std::vector<GameObject*> m_pGoombaTriggers;
	const int m_AmountOfMapParts = 17;
	std::vector<GameObject*> m_MapParts;
	int m_CollectedCoins;
	int m_CollectedStars;
	int m_MaxMarioLives;
	int m_MarioLives;
	bool m_GameOver;
	float m_MaxInvincibleTime;
	float m_InvincibleTime;
	std::vector<GameObject*> m_GameObjectsToDelete;
	DirectX::XMFLOAT3 m_BallSpawnPosTop;
	DirectX::XMFLOAT3 m_BallSpawnPosMid;
	DirectX::XMFLOAT3 m_BallSpawnPosMid2;
	DirectX::XMFLOAT3 m_BallSpawnPosLow;
	float m_BallSpawnInterval;
	float m_BallSpawnTime;
	physx::PxMaterial* m_pRollMaterial;
	SpriteFont* m_pFont;
	GameObject* m_pLivesHud;
	bool m_IsPaused;
	bool m_HasWon;
	const int m_StarsToCollect;
	GameObject* m_PauseMenu;
	GameObject* m_WinMenu;
	GameObject* m_LoseMenu;
	GameObject* m_ControlScheme;
	std::vector<DirectX::XMFLOAT3> m_DelayedCoinSpawns;
	FMOD::Sound* m_pGameOver = nullptr;
	FMOD::Sound* m_pYa = nullptr;
	FMOD::Sound* m_pCoin = nullptr;
	FMOD::Sound* m_pGoomba = nullptr;
	FMOD::Sound* m_pPause = nullptr;
	FMOD::Sound* m_pHereWeGo = nullptr;
	FMOD::Sound* m_pBobOmb = nullptr;
	FMOD::Sound* m_pLetsGo = nullptr;
	FMOD::Sound* m_pHurt = nullptr;

	FMOD::Channel* m_pChannel = nullptr;
	FMOD::Channel* m_pMusicChannel = nullptr;

	GameObject* MakeMapPart(int nr, physx::PxPhysics* physX, GameContext gameContext);
	GameObject* SpawnGoomba(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX);
	GameObject* SpawnStar(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX);
	GameObject* CreateBall(float r, DirectX::XMFLOAT3 pos, physx::PxMaterial* material);
	GameObject* CreateTrigger(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX, DirectX::XMFLOAT3 scale);
	GameObject* CreateInvisibleWall(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX, DirectX::XMFLOAT3 scale);
	GameObject* SpawnCoin(DirectX::XMFLOAT3 pos);
};
