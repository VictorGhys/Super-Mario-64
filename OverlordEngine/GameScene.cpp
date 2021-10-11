#include "stdafx.h"
#include "GameScene.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "Prefabs.h"
#include "Components.h"
#include "DebugRenderer.h"
#include "RenderTarget.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "PhysxProxy.h"
#include "SoundManager.h"
#include <algorithm>

#include "PostProcessingMaterial.h"

GameScene::GameScene(std::wstring sceneName) :
	m_pChildren(std::vector<GameObject*>()),
	m_GameContext(GameContext()),
	m_IsInitialized(false),
	m_SceneName(std::move(sceneName)),
	m_pDefaultCamera(nullptr),
	m_pActiveCamera(nullptr),
	m_pPhysxProxy(nullptr)
{
}

GameScene::~GameScene()
{
	SafeDelete(m_GameContext.pGameTime);
	SafeDelete(m_GameContext.pInput);
	SafeDelete(m_GameContext.pMaterialManager);
	SafeDelete(m_GameContext.pShadowMapper);

	for (auto pChild : m_pChildren)
	{
		SafeDelete(pChild);
	}
	SafeDelete(m_pPhysxProxy);
	//TODO: delete all m_PostProcessingEffects
	for (auto pEffect : m_PostProcessingEffects)
	{
		SafeDelete(pEffect);
	}
}

void GameScene::AddChild(GameObject* obj)
{
#if _DEBUG
	if (obj->m_pParentScene)
	{
		if (obj->m_pParentScene == this)
			Logger::LogWarning(L"GameScene::AddChild > GameObject is already attached to this GameScene");
		else
			Logger::LogWarning(
				L"GameScene::AddChild > GameObject is already attached to another GameScene. Detach it from it's current scene before attaching it to another one.");

		return;
	}

	if (obj->m_pParentObject)
	{
		Logger::LogWarning(
			L"GameScene::AddChild > GameObject is currently attached to a GameObject. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	obj->m_pParentScene = this;
	obj->RootInitialize(m_GameContext);
	m_pChildren.push_back(obj);
}

void GameScene::RemoveChild(GameObject* obj, bool deleteObject)
{
	const auto it = find(m_pChildren.begin(), m_pChildren.end(), obj);

#if _DEBUG
	if (it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameScene::RemoveChild > GameObject to remove is not attached to this GameScene!");
		return;
	}
#endif

	m_pChildren.erase(it);
	if (deleteObject)
	{
		delete obj;
		obj = nullptr;
	}
	else
		obj->m_pParentScene = nullptr;
}

void GameScene::RootInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	if (m_IsInitialized)
		return;

	//Create DefaultCamera
	auto freeCam = new FreeCamera();
	freeCam->SetRotation(30, 0);
	freeCam->GetTransform()->Translate(0, 50, -80);
	AddChild(freeCam);
	m_pDefaultCamera = freeCam->GetComponent<CameraComponent>();
	m_pActiveCamera = m_pDefaultCamera;
	m_GameContext.pCamera = m_pDefaultCamera;

	//Create GameContext
	m_GameContext.pGameTime = new GameTime();
	m_GameContext.pGameTime->Reset();
	m_GameContext.pGameTime->Stop();

	m_GameContext.pInput = new InputManager();
	InputManager::Initialize();

	m_GameContext.pMaterialManager = new MaterialManager();
	m_GameContext.pShadowMapper = new ShadowMapRenderer();

	m_GameContext.pDevice = pDevice;
	m_GameContext.pDeviceContext = pDeviceContext;

	//Initialize ShadowMapper
	m_GameContext.pShadowMapper->Initialize(m_GameContext);

	// Initialize Physx
	m_pPhysxProxy = new PhysxProxy();
	m_pPhysxProxy->Initialize(this);

	//User-Scene Initialize
	Initialize();

	//Root-Scene Initialize
	for (auto pChild : m_pChildren)
	{
		pChild->RootInitialize(m_GameContext);
	}
	//TODO: initialize all m_PostprocessingEffect
	for (auto pEffect : m_PostProcessingEffects)
	{
		pEffect->Initialize(m_GameContext);
	}
	m_IsInitialized = true;
}

void GameScene::RootUpdate()
{
	m_GameContext.pGameTime->Update();
	m_GameContext.pInput->Update();
	m_GameContext.pCamera = m_pActiveCamera;

	SoundManager::GetInstance()->GetSystem()->update();

	//User-Scene Update
	Update();

	//Root-Scene Update
	for (auto pChild : m_pChildren)
	{
		pChild->RootUpdate(m_GameContext);
	}
	//TODO: sort effects based on render index (render smaller numbers first)
	std::sort(m_PostProcessingEffects.begin(), m_PostProcessingEffects.end(),
		[](PostProcessingMaterial* pEffect, PostProcessingMaterial* pEffect2)
		{
			return pEffect->GetRenderIndex() < pEffect2->GetRenderIndex();
		});

	m_pPhysxProxy->Update(m_GameContext);
}

void GameScene::RootDraw()
{
	//TODO: object-Scene SHADOW_PASS - start by setting the correct render target, render all to shadow map and end by reset default render target
	m_GameContext.pShadowMapper->Begin(m_GameContext);
	for (auto pChild : m_pChildren)
	{
		pChild->RootDrawShadowMap(m_GameContext);
	}
	m_GameContext.pShadowMapper->End(m_GameContext);

	//User-Scene Draw
	Draw();

	//Object-Scene Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootDraw(m_GameContext);
	}

	//Object-Scene Post-Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootPostDraw(m_GameContext);
	}

	//Draw PhysX
	m_pPhysxProxy->Draw(m_GameContext);

	//Draw Debug Stuff
	DebugRenderer::Draw(m_GameContext);
	SpriteRenderer::GetInstance()->Draw(m_GameContext);
	TextRenderer::GetInstance()->Draw(m_GameContext);

	//TODO: complete
	//1. Check if our m_PostProcessingEffects isn't empty (if empty, ignore PP)
	if (m_PostProcessingEffects.empty())
		return;
	//2. Get the current (= INIT_RT) rendertarger from the game (OverlordGame::GetRenderTarget...)
	RenderTarget* INIT_RT = SceneManager::GetInstance()->GetGame()->GetRenderTarget();
	//3. Create a new variable to hold our previous rendertarget (= PREV_RT) that holds the content of the previous drawcall
	//		and want to use as a ShaderResource (Texture) for the next PP Effect.
	RenderTarget* PREV_RT = nullptr;
	//4. Set PREV_RT = INIT_RT (the first pp effect uses the contents from the default rendertarget)
	PREV_RT = INIT_RT;
	//4. For each effect
	for (PostProcessingMaterial* CURR_MAT : m_PostProcessingEffects)
	{
		//4.1 Get the RT from CURR_MAT (= TEMP_RT)
		RenderTarget* TEMP_RT = CURR_MAT->GetRenderTarget();
		//4.2 Use TEMP_RT as current rendertarget (OverlordGame::SetRenderTarget)
		SceneManager::GetInstance()->GetGame()->SetRenderTarget(TEMP_RT);
		//4.3 Draw CURR_MAT (PREV_MAT provides the Texture used in the PPEffect)
		CURR_MAT->Draw(m_GameContext, PREV_RT);
		//4.4 Change PREV_MAT to TEMP_MAT
		PREV_RT = TEMP_RT;
	}
	// to fix: resource being set to OM rendertarget slot 0 is still bound on input warning
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_GameContext.pDeviceContext->PSSetShaderResources(0, 1, nullSRV);

	//5. Restore the current rendertarget with INIT_RT
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(INIT_RT);
	//6. Use SpriteRenderer::DrawImmediate to draw the content of the last postprocessed rendertarget > PREV_RT
	SpriteRenderer::GetInstance()->DrawImmediate(m_GameContext, PREV_RT->GetShaderResourceView(), { 0,0 });
}

void GameScene::RootSceneActivated()
{
	//Start Timer
	m_GameContext.pGameTime->Start();
	SceneActivated();
}

void GameScene::RootSceneDeactivated()
{
	//Stop Timer
	m_GameContext.pGameTime->Stop();
	SceneDeactivated();
}

void GameScene::RootWindowStateChanged(int state, bool active) const
{
	//TIMER
	if (state == 0)
	{
		if (active)m_GameContext.pGameTime->Start();
		else m_GameContext.pGameTime->Stop();
	}
}

void GameScene::SetActiveCamera(CameraComponent* pCameraComponent)
{
	if (m_pActiveCamera != nullptr)
		m_pActiveCamera->m_IsActive = false;

	m_pActiveCamera = (pCameraComponent) ? pCameraComponent : m_pDefaultCamera;
	m_pActiveCamera->m_IsActive = true;
}

void GameScene::AddPostProcessingEffect(PostProcessingMaterial* effect)
{
	//TODO: complete
	//Add the givel effect to m_PostProcessingEffects

	//Check for duplicates -> so only one instance can be added!
	//Hint: std::find
	auto It = std::find(m_PostProcessingEffects.begin(), m_PostProcessingEffects.end(), effect);
	if (It == m_PostProcessingEffects.end())
	{
		m_PostProcessingEffects.push_back(effect);
	}
	//Initialize effect if not initialized
	effect->Initialize(m_GameContext);
}
void GameScene::RemovePostProcessingEffect(PostProcessingMaterial* effect)
{
	//TODO: complete
	// Erase and delete effect from m_PostProcessingEffects
	m_PostProcessingEffects.erase(std::find(m_PostProcessingEffects.begin(), m_PostProcessingEffects.end(), effect));
}