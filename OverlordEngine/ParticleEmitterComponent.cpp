#include "stdafx.h"
#include "ParticleEmitterComponent.h"
#include <utility>
#include "EffectHelper.h"
#include "ContentManager.h"
#include "GameObject.h"
#include "TextureDataLoader.h"
#include "Particle.h"
#include "TransformComponent.h"

ParticleEmitterComponent::ParticleEmitterComponent(std::wstring  assetFile, int particleCount) :
	m_pVertexBuffer(nullptr),
	m_pEffect(nullptr),
	m_pParticleTexture(nullptr),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_Settings(ParticleEmitterSettings()),
	m_ParticleCount(particleCount),
	m_ActiveParticles(0),
	m_LastParticleInit(0.0f),
	m_AssetFile(std::move(assetFile))
{
	//TODO: See Lab9_2
	for (int i{}; i < m_ParticleCount; i++)
	{
		m_Particles.push_back(new Particle(m_Settings));
	}
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	//TODO: See Lab9_2
	for (auto particle : m_Particles)
	{
		delete particle;
	}
	m_Particles.clear();
	m_pInputLayout->Release();
	m_pVertexBuffer->Release();
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	//TODO: See Lab9_2
	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	//TODO: See Lab9_2
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ParticleRenderer.fx");
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);
	m_pWvpVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	m_pTextureVariable = m_pEffect->GetVariableByName("gParticleTexture")->AsShaderResource();
	EffectHelper::BuildInputLayout(gameContext.pDevice, m_pDefaultTechnique, &m_pInputLayout, m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	//TODO: See Lab9_2
	//1
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	//2
	D3D11_BUFFER_DESC buffDesc;
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;
	buffDesc.ByteWidth = sizeof(ParticleVertex) * m_ParticleCount;
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffDesc.MiscFlags = 0;

	auto result = gameContext.pDevice->CreateBuffer(&buffDesc, nullptr, &m_pVertexBuffer);
	if (FAILED(result))
	{
		Logger::Log(LogLevel::Error, L"ParticleEmitterComponent -> couldn't create vertex buffer");
	}
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	//TODO: See Lab9_2
	//1
	float particleInterval = (m_Settings.MaxEnergy - m_Settings.MinEnergy) / m_ParticleCount;
	//2
	m_LastParticleInit += gameContext.pGameTime->GetElapsed();
	//3a
	m_ActiveParticles = 0;
	//3b
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	gameContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = (ParticleVertex*)mappedResource.pData;
	//3e
	for (auto particle : m_Particles)
	{
		//3f
		particle->Update(gameContext);
		//3g
		if (particle->IsActive())
		{
			pBuffer[m_ActiveParticles] = particle->GetVertexInfo();
			m_ActiveParticles++;
		}
		else if (m_LastParticleInit >= particleInterval)
		{
			particle->Init(m_pGameObject->GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles] = particle->GetVertexInfo();
			m_ActiveParticles++;
			m_LastParticleInit = 0;
		}
	}
	//3 i & j
	gameContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Draw(const GameContext&)
{}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	if (m_IsActive)
	{
		//TODO: See Lab9_2
		//1
		//m_pWvpVariable->SetMatrix(reinterpret_cast<const float*>(&gameContext.pCamera->GetViewProjection()));
		m_pWvpVariable->SetMatrix(&gameContext.pCamera->GetViewProjection()._11);

		//2
		m_pViewInverseVariable->SetMatrix(&gameContext.pCamera->GetViewInverse()._11);
		//3
		m_pTextureVariable->SetResource(m_pParticleTexture->GetShaderResourceView());
		//4
		gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
		//5
		gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		//6
		unsigned int stride = sizeof(ParticleVertex);
		unsigned int offset = 0;
		gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		//7
		D3DX11_TECHNIQUE_DESC techDesc;
		m_pDefaultTechnique->GetDesc(&techDesc);
		for (unsigned int i = 0; i < techDesc.Passes; ++i)
		{
			m_pDefaultTechnique->GetPassByIndex(i)->Apply(0, gameContext.pDeviceContext);
			gameContext.pDeviceContext->Draw(m_ActiveParticles, 0);
		}
	}
}