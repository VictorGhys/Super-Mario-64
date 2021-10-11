//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ShadowMapMaterial.h"
#include "ContentManager.h"

ShadowMapMaterial::~ShadowMapMaterial()
{
	//TODO: make sure you don't have memory leaks and/or resource leaks :) -> Figure out if you need to do something here
	//m_pWorldMatrixVariable->Release();
	//m_pBoneTransforms->Release();
	//m_pLightVPMatrixVariable->Release();
	//m_pShadowEffect->Release();
}

void ShadowMapMaterial::Initialize(const GameContext& gameContext)
{
	//UNREFERENCED_PARAMETER(gameContext);
	if (!m_IsInitialized)
	{
		//TODO: initialize the effect, techniques, shader variables, input layouts (hint use EffectHelper::BuildInputLayout), etc.
		//Initialize Effect
		m_pShadowEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ShadowMapGenerator.fx");
		//Initialize Techniques
		m_pShadowTechs[0] = m_pShadowEffect->GetTechniqueByIndex(0);//generate shadows
		m_pShadowTechs[1] = m_pShadowEffect->GetTechniqueByIndex(1);//generate shadows skinned

		//Initialize shader variables
		auto effectVar = m_pShadowEffect->GetVariableByName("gWorld");
		m_pWorldMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gLightViewProj");
		m_pLightVPMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gBones");
		m_pBoneTransforms = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;

		//Initialize InputLayout
		if (!EffectHelper::BuildInputLayout(gameContext.pDevice, m_pShadowTechs[0], &m_pInputLayouts[0], m_InputLayoutDescriptions[0],
			m_InputLayoutSizes[0], m_InputLayoutIds[0]))
		{
			Logger::LogWarning(L"ShadowMapMaterial::Initialize() > BuildInputLayout error");
		}

		if (!EffectHelper::BuildInputLayout(gameContext.pDevice, m_pShadowTechs[1], &m_pInputLayouts[1], m_InputLayoutDescriptions[1],
			m_InputLayoutSizes[1], m_InputLayoutIds[1]))
		{
			Logger::LogWarning(L"ShadowMapMaterial::Initialize() > BuildInputLayout error");
		}

		m_IsInitialized = true;
	}
}

void ShadowMapMaterial::SetLightVP(DirectX::XMFLOAT4X4 lightVP) const
{
	//TODO: set the correct shader variable
	if (m_pLightVPMatrixVariable)
	{
		m_pLightVPMatrixVariable->SetMatrix(&lightVP._11);
	}
}

void ShadowMapMaterial::SetWorld(DirectX::XMFLOAT4X4 world) const
{
	//TODO: set the correct shader variable
	if (m_pWorldMatrixVariable)
	{
		m_pWorldMatrixVariable->SetMatrix(&world._11);
	}
}

void ShadowMapMaterial::SetBones(const float* pData, int count) const
{
	//TODO: set the correct shader variable
	if (m_pBoneTransforms)
	{
		m_pBoneTransforms->SetMatrixArray(pData, 0, count);
	}
}