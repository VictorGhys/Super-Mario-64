#include "stdafx.h"
#include "Particle.h"
#include "MathHelper.h"

// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
using namespace DirectX;

Particle::Particle(const ParticleEmitterSettings& emitterSettings) :
	m_VertexInfo(ParticleVertex()),
	m_EmitterSettings(emitterSettings),
	m_IsActive(false),
	m_TotalEnergy(0),
	m_CurrentEnergy(0),
	m_SizeGrow(0),
	m_InitSize(0)
{}

void Particle::Update(const GameContext& gameContext)
{
	//TODO: See Lab9_2
	//1
	if (!m_IsActive)
	{
		return;
	}
	//2
	m_CurrentEnergy -= gameContext.pGameTime->GetElapsed();
	if (m_CurrentEnergy < 0)
	{
		m_IsActive = false;
	}
	//3
	using namespace DirectX;
	//3a
	XMVECTOR newPos = XMLoadFloat3(&m_VertexInfo.Position) + XMLoadFloat3(&m_EmitterSettings.Velocity) * gameContext.pGameTime->GetElapsed();
	XMStoreFloat3(&m_VertexInfo.Position, newPos);
	//3b
	m_VertexInfo.Color = m_EmitterSettings.Color;
	float particleLifePercent = m_CurrentEnergy / m_TotalEnergy;//1 at the beginning, 0 at the end of the lifetime
	m_VertexInfo.Color.w = particleLifePercent * 2 * m_EmitterSettings.Color.w;
	//3c
	if (m_SizeGrow < 1)
	{
		m_VertexInfo.Size = m_InitSize + m_InitSize * (m_SizeGrow - 1) * (1 - particleLifePercent);
	}
	if (m_SizeGrow > 1)
	{
		m_VertexInfo.Size = m_InitSize + m_InitSize * (m_SizeGrow - 1) * (1 - particleLifePercent);
	}
	//sizeGrow * ( 1- particleLifePercent)
}

void Particle::Init(XMFLOAT3 initPosition)
{
	//TODO: See Lab9_2
	//1
	m_IsActive = true;
	//2
	float randEnergy = randF(m_EmitterSettings.MinEnergy, m_EmitterSettings.MaxEnergy);
	m_TotalEnergy = randEnergy;
	m_CurrentEnergy = randEnergy;
	//3ab
	XMVECTOR randomDirection{ 1,0,0 };
	XMMATRIX randomRotationMatrix = XMMatrixRotationRollPitchYaw(randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI), randF(-XM_PI, XM_PI));
	XMVECTOR randomNormalizedVector = XMVector3TransformNormal(randomDirection, randomRotationMatrix);
	//3c
	float randomDistance = randF(m_EmitterSettings.MinEmitterRange, m_EmitterSettings.MaxEmitterRange);
	//3d
	XMStoreFloat3(&m_VertexInfo.Position, XMLoadFloat3(&initPosition) + randomNormalizedVector * randomDistance);
	//4
	float size = randF(m_EmitterSettings.MinSize, m_EmitterSettings.MaxSize);
	m_VertexInfo.Size = size;
	m_InitSize = size;
	m_SizeGrow = randF(m_EmitterSettings.MinSizeGrow, m_EmitterSettings.MaxSizeGrow);
	//5
	m_VertexInfo.Rotation = randF(-XM_PI, XM_PI);
}