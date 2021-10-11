#include "stdafx.h"
#include "SuperMarioScene.h"

#include "Components.h"
#include "ContentManager.h"
#include "PhysxManager.h"
#include "Prefabs.h"
#include "GameObject.h"
#include "Goomba.h"
#include "../../Materials/DiffuseMaterial.h"
#include "../../Materials/SkinnedDiffuseMaterial.h"
#include "ModelAnimator.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "SpriteFont.h"
#include "../../Materials/UberMaterial.h"
#include "../../Materials/Shadow/DiffuseMaterial_Shadow.h"
#include "../../Materials/Shadow/SkinnedDiffuseMaterial_Shadow.h"
#include "Character.h"
#include "TextRenderer.h"
#include "PostPixelate.h"

int SuperMarioScene::resets = 0;

SuperMarioScene::SuperMarioScene(const std::wstring& name)
	:GameScene(name),
	m_CollectedCoins{ 0 },
	m_CollectedStars(0),
	m_MaxMarioLives(8),
	m_MarioLives(m_MaxMarioLives),
	m_MaxInvincibleTime(3),
	m_InvincibleTime(m_MaxInvincibleTime),
	m_BallSpawnInterval(20),
	m_BallSpawnTime(0),
	m_pLivesHud(nullptr),
	m_GameOver(false),
	m_IsPaused(false),
	m_PauseMenu(nullptr),
	m_WinMenu(nullptr),
	m_LoseMenu(nullptr),
	m_ControlScheme(nullptr),
	m_StarsToCollect(1),
	m_HasWon(false),
	m_pFont(nullptr)
{
}
SuperMarioScene::~SuperMarioScene()
{
	std::cout << "wait\n";
	m_pRollMaterial->release();
	m_pMusicChannel->stop();
}
void coinCollected(GameObject* triggerobject, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"mario")
	{
		//coin collected
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddCoin();
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddToDelete(triggerobject);
	}
}
void hitBall(GameObject*, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"mario")
	{
		//ball hit mario
		static_cast<SuperMarioScene*>(otherobject->GetScene())->RemoveLives(2);
	}
}
void starCollected(GameObject* triggerobject, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"mario")
	{
		//mario won!
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddStar();
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddToDelete(triggerobject);
	}
}
void ballInTrigger(GameObject*, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"ball")
	{
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddToDelete(otherobject);
	}
}
void SuperMarioScene::Initialize()
{
	using namespace physx;
	const auto gameContext = GetGameContext();
	DirectX::XMFLOAT3 lightDirection{};
	DirectX::XMStoreFloat3(&lightDirection, DirectX::XMVector3Normalize({ -0.3f, -0.8f, -0.3f }));
	gameContext.pShadowMapper->SetLight({ 0,100.f,0 }, lightDirection);

	//GetPhysxProxy()->EnablePhysxDebugRendering(true);//uncomment to enable physX debug rendering
	GetGameContext().pGameTime->ForceElapsedUpperbound(true);

	// Create PhysX GROUND PLANE -------------------------------------------------------------------------------------------------------------------------------
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	auto bouncyMaterial = physX->createMaterial(0, 0, 1);
	auto ground = new GameObject();

	ground->GetTransform()->Translate(0, 0, 0);
	ground->SetTag(L"ground");
	ground->AddComponent(new RigidBodyComponent(true));
	std::shared_ptr<physx::PxGeometry> geom(new physx::PxPlaneGeometry());
	ground->AddComponent(new ColliderComponent(geom, *bouncyMaterial, physx::PxTransform(physx::PxQuat(DirectX::XM_PIDIV2, physx::PxVec3(0, 0, 1)))));
	AddChild(ground);

	// ground shadow test
	auto diffMat2 = new DiffuseMaterial_Shadow();
	diffMat2->SetDiffuseTexture(L"./Resources/Textures/GroundBrick.jpg");
	diffMat2->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(diffMat2, 1);

	// MAP -------------------------------------------------------------------------------------------------------------------------------
	for (int i{ 1 }; i <= m_AmountOfMapParts; i++)
	{
		m_MapParts.push_back(MakeMapPart(i, physX, gameContext));
	}
	// BALL -------------------------------------------------------------------------------------------------------------------------------
	auto superBouncyMaterial = physX->createMaterial(0, 0, 1.2f);
	// Pit
	CreateBall(4, { -174, 60, -325 }, superBouncyMaterial);
	CreateBall(4, { -188, 60, -300 }, superBouncyMaterial);
	CreateBall(4, { -150, 60, -297 }, superBouncyMaterial);
	// Top
	m_pRollMaterial = physX->createMaterial(2, 2, 0);
	m_BallSpawnPosTop = { -235, 117, -368 };
	m_BallSpawnPosMid = { -290, 112, -394 };
	m_BallSpawnPosMid2 = { -339, 100, -369 }; //-339, 100, -365
	m_BallSpawnPosLow = { -201, 80, -363 };

	CreateBall(4, m_BallSpawnPosTop, m_pRollMaterial);//top
	CreateBall(4, m_BallSpawnPosMid, m_pRollMaterial);//mid
	CreateBall(4, m_BallSpawnPosMid2, m_pRollMaterial);//mid2
	CreateBall(4, m_BallSpawnPosLow, m_pRollMaterial);//low

	// BALL TRIGGER
	CreateTrigger({ -170, 5, -207 }, physX, { 10,30,100 });//down
	CreateTrigger({ -427, 64, -401 }, physX, { 10,30,100 });//side
	CreateTrigger({ -221, 37, -259 }, physX, { 10,20,30 });//pit
	// BALL TRIGGER
	CreateInvisibleWall({ -175, 50, 61 }, physX, { 600,100,10 });//north
	CreateInvisibleWall({ 56, 50, -195 }, physX, { 10,100,600 });//east
	CreateInvisibleWall({ -138, 50, -437 }, physX, { 650,150,10 });//south
	CreateInvisibleWall({ -445, 50, -195 }, physX, { 10,150,600 });//west
	CreateInvisibleWall({ -185, 80, -400 }, physX, { 10,100,68 });//cheat alley
	CreateInvisibleWall({ -170, 5, -207 }, physX, { 5,70,100 });//marble slope

	// STAR -------------------------------------------------------------------------------------------------------------------------------
	// Create a DiffuseMaterial
	auto diffuseMaterial = new DiffuseMaterial();
	diffuseMaterial->SetDiffuseTexture(L"Resources/Textures/eye.png");
	// Assign the material to the modelcomponent
	gameContext.pMaterialManager->AddMaterial(diffuseMaterial, 3);

	diffuseMaterial = new DiffuseMaterial();
	diffuseMaterial->SetDiffuseTexture(L"Resources/Textures/body.png");
	// Assign the material to the modelcomponent
	gameContext.pMaterialManager->AddMaterial(diffuseMaterial, 4);

	SpawnStar({ -279, 135, -339 }, physX);//top
	//SpawnStar({ -50, 2, 0 }, physX);//test star

	// COIN -------------------------------------------------------------------------------------------------------------------------------
	// Create DifuseMaterial
	auto coinMaterial = new DiffuseMaterial_Shadow();
	coinMaterial->SetDiffuseTexture(L"./Resources/Textures/coin.png");
	coinMaterial->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(coinMaterial, 5);

	//SpawnCoin({ 5,2,-3 });//test
	SpawnCoin({ -252, 27, 9 });//flower
	SpawnCoin({ -341, 42, -159 });//rock
	SpawnCoin({ -68, 25, -86 });//park2
	SpawnCoin({ 3, 35, -288 });//flower2
	SpawnCoin({ -276, 91, -245 });//plank
	SpawnCoin({ -200, 118, -342 });//corner tower

	// ADD	MARIO -------------------------------------------------------------------------------------------------------------------------------
	m_pMario = new Character(1, 2.5f, 100, true, { 0,5,-20 });
	m_pMario->SetTag(L"mario");

	// Create SkinnedDifuseMaterial
	auto marioBodyMaterial = new SkinnedDiffuseMaterial_Shadow();
	marioBodyMaterial->SetDiffuseTexture(L"./Resources/Textures/mario_all.png");
	marioBodyMaterial->SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	gameContext.pMaterialManager->AddMaterial(marioBodyMaterial, 7);

	auto body = new ModelComponent{ L"Resources/Meshes/mariowithface.ovm" };
	body->SetMaterial(7);
	GameObject* visuals = new GameObject();
	visuals->AddComponent(body);
	m_pMario->AddChild(visuals);
	m_pMario->SetVisuals(visuals);
	const float scale = 2.5f;//2.5
	visuals->GetTransform()->Scale(scale, scale, scale);
	visuals->GetTransform()->Translate(0, -2, 0);
	visuals->GetTransform()->Rotate(0, 180, 0);

	// Mario Smoke
	auto marioSmokeParticles = new ParticleEmitterComponent(L"./Resources/Textures/smokepuf.png", 60);
	marioSmokeParticles->SetVelocity(DirectX::XMFLOAT3(0, 0.f, -1));
	marioSmokeParticles->SetMinSize(1.0f);
	marioSmokeParticles->SetMaxSize(2.0f);
	marioSmokeParticles->SetMinEnergy(.5f);
	marioSmokeParticles->SetMaxEnergy(1.0f);
	marioSmokeParticles->SetMinSizeGrow(3.5f);
	marioSmokeParticles->SetMaxSizeGrow(5.5f);
	marioSmokeParticles->SetMinEmitterRange(0.2f);
	marioSmokeParticles->SetMaxEmitterRange(0.5f);
	marioSmokeParticles->SetColor(DirectX::XMFLOAT4(1.5f, 1.5f, 1.5f, 0.3f));
	visuals->AddComponent(marioSmokeParticles);
	m_pMario->SetSmokeParticle(marioSmokeParticles);

	AddChild(m_pMario);
	body->GetAnimator()->SetAnimation(0);
	body->GetAnimator()->Play();
	m_pMario->GetTransform()->Translate(0, 1, 0);
	//m_pMario->GetTransform()->Translate(-138, 55, -358);//pit
	//m_pMario->GetTransform()->Translate(-279, 135, -339);//Top
	//m_pMario->GetTransform()->Translate(-354, 101, -375);//Mid
	//m_pMario->GetTransform()->Translate(-179, 81, -349);//Mid corner
	m_pMario->SetAnimator(body->GetAnimator());

	// ADD GOOMBA -------------------------------------------------------------------------------------------------------------------------------
	// Create a Material
	auto uberMaterial = new UberMaterial();
	uberMaterial->SetDiffuseTexture(L"Resources/Textures/goomba_grp.png");
	uberMaterial->EnableDiffuseTexture(true);
	uberMaterial->SetOpacityTexture(L"Resources/Textures/goomba_opacity.png");
	uberMaterial->EnableOpacityMap(true);
	gameContext.pMaterialManager->AddMaterial(uberMaterial, 2);

	m_Goombas.push_back(SpawnGoomba({ -71, 2, 11 }, physX));//start
	m_Goombas.push_back(SpawnGoomba({ -157, 10, -21 }, physX));//bridge
	m_Goombas.push_back(SpawnGoomba({ -285, 25, 13 }, physX));//park
	m_Goombas.push_back(SpawnGoomba({ -398, 28, -235 }, physX));//park
	m_Goombas.push_back(SpawnGoomba({ -203, 25, -111 }, physX));//cage
	m_Goombas.push_back(SpawnGoomba({ -58, 25, -142 }, physX));//east park
	m_Goombas.push_back(SpawnGoomba({ -17, 25, -118 }, physX));//east park2
	m_Goombas.push_back(SpawnGoomba({ -6, 33, -233 }, physX));//east park3
	m_Goombas.push_back(SpawnGoomba({ 2, 33, -331 }, physX));//east park4
	m_Goombas.push_back(SpawnGoomba({ -400, 62, -341 }, physX));//west tower
	m_Goombas.push_back(SpawnGoomba({ -139, 2, -268 }, physX));//down

	// HUD -------------------------------------------------------------------------------------------------------------------------------
	auto pHudCoin = new GameObject();
	pHudCoin->AddComponent(new SpriteComponent(L"./Resources/Textures/coin_hud.png", DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT4(1, 1, 1, 1.f)));
	AddChild(pHudCoin);
	pHudCoin->GetTransform()->Scale(3, 3, 3);
	pHudCoin->GetTransform()->Translate(400, 10, 0);

	auto pHudStar = new GameObject();
	pHudStar->AddComponent(new SpriteComponent(L"./Resources/Textures/star_hud.png", DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT4(1, 1, 1, 1.f)));
	AddChild(pHudStar);
	pHudStar->GetTransform()->Scale(3, 3, 3);
	pHudStar->GetTransform()->Translate(550, 10, 0);

	m_pLivesHud = new GameObject();
	m_pLivesHud->AddComponent(new SpriteComponent(L"./Resources/Textures/hudLives8.png", DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT4(1, 1, 1, 1.f)));
	AddChild(m_pLivesHud);
	m_pLivesHud->GetTransform()->Scale(3, 3, 3);
	m_pLivesHud->GetTransform()->Translate(80, 10, 0);

	m_ControlScheme = new GameObject();
	m_ControlScheme->AddComponent(new SpriteComponent(L"./Resources/Textures/ControlScheme.png", DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT4(1, 1, 1, 1.f)));
	AddChild(m_ControlScheme);

	// MENU -------------------------------------------------------------------------------------------------------------------------------

	// FONT -------------------------------------------------------------------------------------------------------------------------------
	m_pFont = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/SuperMario_40.fnt");
	// POST PROCESSING -------------------------------------------------------------------------------------------------------------------------------
	AddPostProcessingEffect(new PostPixelate({ 1920.f / 2.f,1080.f / 2.f }));

	// Sound -------------------------------------------------------------------------------------------------------------------------------
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/mario-gameover.wav", FMOD_2D, nullptr, &m_pGameOver);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/mario-ya.wav", FMOD_2D, nullptr, &m_pYa);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_coin.wav", FMOD_2D, nullptr, &m_pCoin);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_goomba_flattened.wav", FMOD_2D, nullptr, &m_pGoomba);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_pause.wav", FMOD_2D, nullptr, &m_pPause);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_here_we_go.wav", FMOD_2D, nullptr, &m_pHereWeGo);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/Super Mario 64 - Main Theme Music - Bob-Omb Battlefield.mp3", FMOD_2D, nullptr, &m_pBobOmb);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_mario_lets_go.wav", FMOD_2D, nullptr, &m_pLetsGo);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/sm64_mario_hurt.wav", FMOD_2D, nullptr, &m_pHurt);

	SoundManager::GetInstance()->GetSystem()->playSound(m_pLetsGo, nullptr, false, &m_pChannel);
	SoundManager::GetInstance()->GetSystem()->playSound(m_pBobOmb, nullptr, false, &m_pMusicChannel);
	m_pMusicChannel->setVolume(0.01f);

	//Input
	//*****
}
void SuperMarioScene::Update()
{
	using namespace DirectX;
	const auto gameContext = GetGameContext();

	if (m_ControlScheme && m_pMario->GetHasEverMoved())
	{
		RemoveChild(m_ControlScheme);
		m_ControlScheme = nullptr;
	}

	// pausing
	if (gameContext.pInput->IsKeyboardKeyDown(VK_ESCAPE) && !gameContext.pInput->IsKeyboardKeyDown(VK_ESCAPE, true))
	{
		m_IsPaused = !m_IsPaused;
		static_cast<Character*>(m_pMario)->SetIsPaused(m_IsPaused);
		if (m_IsPaused)
		{
			std::cout << "pause" << std::endl;
			if (m_PauseMenu == nullptr)
			{
				m_PauseMenu = new GameObject();
				m_PauseMenu->AddComponent(new SpriteComponent(L"./Resources/Textures/PauseMenu.png", DirectX::XMFLOAT2(0.5f, 0.5f),
					DirectX::XMFLOAT4(1, 1, 1, 1.f)));
				AddChild(m_PauseMenu);
			}
		}
		else
		{
			std::cout << "unpause" << std::endl;
			RemoveChild(m_PauseMenu);
			m_PauseMenu = nullptr;
		}
		SoundManager::GetInstance()->GetSystem()->playSound(m_pPause, nullptr, false, &m_pChannel);
	}
	if (!(m_IsPaused || m_HasWon || m_GameOver))
	{
		DirectX::XMFLOAT3 shadowPos{};
		DirectX::XMStoreFloat3(&shadowPos, XMLoadFloat3(&m_pMario->GetTransform()->GetWorldPosition()) + DirectX::XMVECTOR{ 0, 10, 0 });
		gameContext.pShadowMapper->Translate(shadowPos);

		// show pos
		if (gameContext.pInput->IsKeyboardKeyDown('P') && !gameContext.pInput->IsKeyboardKeyDown('P', true))
		{
			std::cout << int(m_pMario->GetTransform()->GetPosition().x) << ", " << int(m_pMario->GetTransform()->GetPosition().y)
				<< ", " << int(m_pMario->GetTransform()->GetPosition().z) << std::endl;
		}

		for (auto go : m_GameObjectsToDelete)
		{
			RemoveChild(go);
		}
		m_GameObjectsToDelete.clear();

		m_InvincibleTime += gameContext.pGameTime->GetElapsed();
		// Ball spawning
		m_BallSpawnTime += gameContext.pGameTime->GetElapsed();
		if (m_BallSpawnTime >= m_BallSpawnInterval)
		{
			m_BallSpawnTime -= m_BallSpawnInterval;
			CreateBall(4, m_BallSpawnPosTop, m_pRollMaterial);//top
			CreateBall(4, m_BallSpawnPosMid, m_pRollMaterial);//mid
			CreateBall(4, m_BallSpawnPosMid2, m_pRollMaterial);//mid2
			CreateBall(4, m_BallSpawnPosLow, m_pRollMaterial);//low
		}
		// coin spawning
		for (auto coinSpawnPos : m_DelayedCoinSpawns)
		{
			SpawnCoin(coinSpawnPos);
		}
		m_DelayedCoinSpawns.clear();
	}
	else
	{
		if (gameContext.pInput->IsMouseButtonDown(VK_LBUTTON) && !gameContext.pInput->IsMouseButtonDown(VK_LBUTTON, true))
		{
			POINT mousePos = InputManager::GetMousePosition();
			if (mousePos.x > 481 && mousePos.y < 320 && mousePos.x < 795 && mousePos.y > 204)
			{
				// to main menu
				SceneManager::GetInstance()->SetActiveGameScene(L"MenuScene");
				m_pMusicChannel->stop();
			}
			else if (mousePos.x > 482 && mousePos.y < 456 && mousePos.x < 797 && mousePos.y > 340)
			{
				// restart
				resets++;
				auto newSceneName = L"SuperMarioScene" + std::to_wstring(resets);
				SceneManager::GetInstance()->AddGameScene(new SuperMarioScene(newSceneName));
				SceneManager::GetInstance()->SetActiveGameScene(L"SuperMarioScene" + std::to_wstring(resets));
				m_pMusicChannel->stop();
			}
			else if (mousePos.x > 480 && mousePos.y < 593 && mousePos.x < 796 && mousePos.y > 482)
			{
				// quit
				PostQuitMessage(WM_QUIT);
			}
		}
	}
}
void SuperMarioScene::Draw()
{
	if (m_pFont->GetFontName() != L"")
	{
		std::wstringstream coins;
		coins << m_CollectedCoins;
		TextRenderer::GetInstance()->DrawText(m_pFont, coins.str(), DirectX::XMFLOAT2(850, 25), DirectX::XMFLOAT4(DirectX::Colors::Orange));

		std::wstringstream stars;
		stars << m_CollectedStars;
		TextRenderer::GetInstance()->DrawText(m_pFont, stars.str(), DirectX::XMFLOAT2(1150, 25), DirectX::XMFLOAT4(DirectX::Colors::Orange));
	}
}

GameObject* SuperMarioScene::MakeMapPart(int nr, physx::PxPhysics* physX, GameContext gameContext)
{
	GameObject* map = new GameObject();
	ModelComponent* model = new ModelComponent{ L"Resources/Meshes/SuperMarioMap" + std::to_wstring(nr) + L".ovm" };
	map->AddComponent(model);

	auto pMapRigidBody = new RigidBodyComponent();
	pMapRigidBody->SetKinematic(true);
	map->AddComponent(pMapRigidBody);

	physx::PxTriangleMesh* pTriangleMesh = ContentManager::Load<physx::PxTriangleMesh>(L"Resources/Meshes/SuperMarioMap" + std::to_wstring(nr) + L".ovpt");
	physx::PxMeshScale triangleMeshScale{ 30.f };
	std::shared_ptr<physx::PxGeometry> pTriangleGeometry = std::make_shared<physx::PxTriangleMeshGeometry>(pTriangleMesh, triangleMeshScale);
	physx::PxMaterial* const pTriangleMeshMat = physX->createMaterial(1.f, 1.f, 0.f);
	ColliderComponent* triangleMeshCollider = new ColliderComponent(pTriangleGeometry, *pTriangleMeshMat);//collider component get's initialized when addchild is called
	map->AddComponent(triangleMeshCollider);

	AddChild(map);
	map->GetTransform()->Scale(30, 30, 30);
	map->GetTransform()->Translate(-220, 0, -210/*-130, 0, -150*/);

	// Create a DiffuseMaterial
	auto mapMaterial = new DiffuseMaterial_Shadow();
	mapMaterial->SetDiffuseTexture(L"Resources/Textures/SuperMarioMap/" + std::to_wstring(nr) + L".png");
	// Assign the material to the modelcomponent
	gameContext.pMaterialManager->AddMaterial(mapMaterial, 10 + nr);
	model->SetMaterial(10 + nr);
	return map;
}

GameObject* SuperMarioScene::SpawnCoin(DirectX::XMFLOAT3 pos)
{
	GameObject* coin = new GameObject();
	coin->SetTag(L"coin");

	ModelComponent* model = new ModelComponent{ L"Resources/Meshes/Coin.ovm" };
	coin->AddComponent(model);

	auto pRigidBody = new RigidBodyComponent();
	pRigidBody->SetKinematic(true);
	coin->AddComponent(pRigidBody);
	// CONVEX MESH
	auto pConvexMesh = ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/Coin.ovpc");
	physx::PxMeshScale meshScaleCoin{ 1.f };
	std::shared_ptr<physx::PxGeometry> pGeometry = std::make_shared<physx::PxConvexMeshGeometry>(pConvexMesh, meshScaleCoin);

	auto defaultMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0, 0, 0);
	auto trigger = new ColliderComponent(pGeometry, *defaultMaterial);
	trigger->EnableTrigger(true);
	coin->AddComponent(trigger);
	coin->SetOnTriggerCallBack(coinCollected);

	coin->GetTransform()->Translate(pos);
	AddChild(coin);

	model->SetMaterial(5);
	return coin;
}

void SuperMarioScene::AddCoin()
{
	m_CollectedCoins++;
	//std::cout << "collected coins: " << m_CollectedCoins << std::endl;
	SoundManager::GetInstance()->GetSystem()->playSound(m_pCoin, nullptr, false, &m_pChannel);
}

void SuperMarioScene::AddStar()
{
	m_CollectedStars++;
	//std::cout << "collected stars: " << m_CollectedStars << std::endl;
	if (m_CollectedStars >= m_StarsToCollect)
	{
		std::cout << "Congratulations you won!\n";
		m_HasWon = true;
		m_IsPaused = true;
		static_cast<Character*>(m_pMario)->SetIsPaused(m_IsPaused);

		if (m_WinMenu == nullptr)
		{
			m_WinMenu = new GameObject();
			m_WinMenu->AddComponent(new SpriteComponent(L"./Resources/Textures/WinMenu.png", DirectX::XMFLOAT2(0.5f, 0.5f),
				DirectX::XMFLOAT4(1, 1, 1, 1.f)));
			AddChild(m_WinMenu);
		}
		SoundManager::GetInstance()->GetSystem()->playSound(m_pHereWeGo, nullptr, false, &m_pChannel);
	}
}

void SuperMarioScene::RemoveLives(int lives)
{
	if (m_InvincibleTime >= m_MaxInvincibleTime)
	{
		m_MarioLives -= lives;
		m_InvincibleTime = 0;
		//std::cout << "mario has " << m_MarioLives << " lives\n";
		if (m_MarioLives >= 0 && m_MarioLives <= m_MaxMarioLives)
		{
			//update lives;
			std::wstring newLivesSprite = L"./Resources/Textures/hudLives" + std::to_wstring(m_MarioLives) + L".png";
			m_pLivesHud->GetComponent<SpriteComponent>()->SetTexture(newLivesSprite);
			SoundManager::GetInstance()->GetSystem()->playSound(m_pHurt, nullptr, false, &m_pChannel);
		}

		if (m_MarioLives <= 0 && !m_GameOver)
		{
			// GAME OVER!
			std::cout << "GAME OVER!\n" << std::endl;
			m_GameOver = true;

			m_IsPaused = true;
			static_cast<Character*>(m_pMario)->SetIsPaused(m_IsPaused);
			// show lose menu
			if (m_LoseMenu == nullptr)
			{
				m_LoseMenu = new GameObject();
				m_LoseMenu->AddComponent(new SpriteComponent(L"./Resources/Textures/LoseMenu.png", DirectX::XMFLOAT2(0.5f, 0.5f),
					DirectX::XMFLOAT4(1, 1, 1, 1.f)));
				AddChild(m_LoseMenu);
			}
			SoundManager::GetInstance()->GetSystem()->playSound(m_pGameOver, nullptr, false, &m_pChannel);
		}
	}
}

GameObject* SuperMarioScene::SpawnGoomba(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX)
{
	auto pGoomba = new Goomba(pos, m_pMario);
	AddChild(pGoomba);

	auto d6Joint = PxD6JointCreate(*physX, nullptr, physx::PxTransform::createIdentity(), pGoomba->GetComponent<ColliderComponent>()->GetShape()->getActor(),
		physx::PxTransform::createIdentity());
	d6Joint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);
	d6Joint->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eLOCKED);
	d6Joint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eLOCKED);
	d6Joint->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eLOCKED);

	return pGoomba;
}
GameObject* SuperMarioScene::SpawnStar(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX)
{
	GameObject* pStar = new GameObject();
	// Star eyes
	auto model = new ModelComponent{ L"Resources/Meshes/star.ovm" };
	pStar->AddComponent(model);
	model->SetMaterial(3);

	// Star body
	model = new ModelComponent{ L"Resources/Meshes/star_body.ovm" };
	pStar->AddComponent(model);
	model->SetMaterial(4);

	auto pRigidBody = new RigidBodyComponent();
	pRigidBody->SetKinematic(true);
	pStar->AddComponent(pRigidBody);

	physx::PxConvexMesh* pConvexMesh = ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/star_body.ovpc");
	physx::PxMeshScale meshScale{ 1.f };
	std::shared_ptr<physx::PxGeometry> pGeometry = std::make_shared<physx::PxConvexMeshGeometry>(pConvexMesh, meshScale);
	physx::PxMaterial* const pMeshMat = physX->createMaterial(1.f, 1.f, 0.f);
	ColliderComponent* meshCollider = new ColliderComponent(pGeometry, *pMeshMat);//collider component get's initialized when addchild is called
	meshCollider->EnableTrigger(true);
	pStar->AddComponent(meshCollider);
	pStar->SetOnTriggerCallBack(starCollected);

	pStar->GetTransform()->Translate(pos);
	AddChild(pStar);
	return pStar;
}
GameObject* SuperMarioScene::CreateBall(float r, DirectX::XMFLOAT3 pos, physx::PxMaterial* material)
{
	auto pSphere = new SpherePrefab(r);
	pSphere->SetTag(L"ball");
	auto pRigidBody = new RigidBodyComponent();
	pSphere->AddComponent(pRigidBody);

	std::shared_ptr<physx::PxGeometry> sphereGeom(new physx::PxSphereGeometry(r));
	pSphere->AddComponent(new ColliderComponent(sphereGeom, *material));
	// Trigger
	std::shared_ptr<physx::PxGeometry> triggerSphereGeom(new physx::PxSphereGeometry(r + 0.5f));
	auto collider = new ColliderComponent(triggerSphereGeom, *material);
	collider->EnableTrigger(true);
	pSphere->AddComponent(collider);
	pSphere->SetOnTriggerCallBack(hitBall);

	AddChild(pSphere);
	pSphere->GetTransform()->Translate(pos);
	return pSphere;
}
void SuperMarioScene::AddToDelete(GameObject* go)
{
	m_GameObjectsToDelete.push_back(go);
	if (go->GetTag() == L"goomba")
	{
		auto It = std::find(m_Goombas.begin(), m_Goombas.end(), go);
		if (It != m_Goombas.end())
		{
			m_Goombas.erase(It);
		}
	}
}
GameObject* SuperMarioScene::CreateTrigger(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX, DirectX::XMFLOAT3 scale)
{
	std::shared_ptr<physx::PxGeometry> boxGeom(new physx::PxBoxGeometry(scale.x / 2.f, scale.y / 2.f, scale.z / 2.f));
	auto defaultMaterial = physX->createMaterial(0, 0, 0);
	auto collider = new ColliderComponent(boxGeom, *defaultMaterial);
	collider->EnableTrigger(true);
	auto trigger = new GameObject;
	trigger->SetTag(L"trigger");
	// Add rigidbody needed for collider
	auto pTriggerRigidBody = new RigidBodyComponent();
	pTriggerRigidBody->SetKinematic(true);
	trigger->AddComponent(pTriggerRigidBody);

	trigger->AddComponent(collider);
	AddChild(trigger);
	trigger->GetTransform()->Translate(DirectX::XMFLOAT3{ pos });
	trigger->SetOnTriggerCallBack(ballInTrigger);
	return trigger;
}
GameObject* SuperMarioScene::CreateInvisibleWall(DirectX::XMFLOAT3 pos, physx::PxPhysics* physX, DirectX::XMFLOAT3 scale)
{
	std::shared_ptr<physx::PxGeometry> boxGeom(new physx::PxBoxGeometry(scale.x / 2.f, scale.y / 2.f, scale.z / 2.f));
	auto defaultMaterial = physX->createMaterial(0, 0, 0);
	auto collider = new ColliderComponent(boxGeom, *defaultMaterial);
	auto trigger = new GameObject;
	trigger->SetTag(L"trigger");
	// Add rigidbody needed for collider
	auto pTriggerRigidBody = new RigidBodyComponent();
	pTriggerRigidBody->SetKinematic(true);
	trigger->AddComponent(pTriggerRigidBody);

	trigger->AddComponent(collider);
	AddChild(trigger);
	trigger->GetTransform()->Translate(DirectX::XMFLOAT3{ pos });
	trigger->SetOnTriggerCallBack(ballInTrigger);
	return trigger;
}
void SuperMarioScene::SpawnCoinDelayed(DirectX::XMFLOAT3 pos)
{
	m_DelayedCoinSpawns.push_back(pos);
}
void SuperMarioScene::PlayGoombaSound()
{
	SoundManager::GetInstance()->GetSystem()->playSound(m_pGoomba, nullptr, false, &m_pChannel);
}