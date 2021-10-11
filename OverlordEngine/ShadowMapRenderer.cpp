#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "ContentManager.h"
#include "ShadowMapMaterial.h"
#include "RenderTarget.h"
#include "MeshFilter.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "TransformComponent.h"

ShadowMapRenderer::~ShadowMapRenderer()
{
	//TODO: make sure you don't have memory leaks and/or resource leaks :) -> Figure out if you need to do something here
	delete m_pShadowMat;
	delete m_pShadowRT;
}

void ShadowMapRenderer::Initialize(const GameContext& gameContext)
{
	if (m_IsInitialized)
		return;

	//TODO: create shadow generator material + initialize it
	m_pShadowMat = new ShadowMapMaterial();
	m_pShadowMat->Initialize(gameContext);
	//TODO: create a rendertarget with the correct settings (hint: depth only) for the shadow generator using a RENDERTARGET_DESC
	m_pShadowRT = new RenderTarget(gameContext.pDevice);
	RENDERTARGET_DESC desc{};
	desc.EnableDepthSRV = true;
	desc.Height = 720;//720
	desc.Width = 1280;//1280
	m_pShadowRT->Create(desc);
	m_Size = 200;//100

	m_IsInitialized = true;
}

void ShadowMapRenderer::SetLight(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction)
{
	//TODO: store the input parameters in the appropriate datamembers
	m_LightPosition = position;
	m_LightDirection = direction;
	//TODO: calculate the Light VP matrix (Directional Light only ;)) and store it in the appropriate datamember
	using namespace DirectX;
	auto windowSettings = OverlordGame::GetGameSettings().Window;
	float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
	float viewHeigth = (m_Size > 0) ? m_Size : windowSettings.Height;

	auto lightProj = XMMatrixOrthographicLH(viewWidth, viewHeigth, 0.1f, 500.0f);
	auto vPos = XMLoadFloat3(&m_LightPosition);
	auto vDir = XMLoadFloat3(&m_LightDirection);
	auto lightView = XMMatrixLookAtLH(vPos, XMVectorAdd(XMVector3Normalize(vDir), vPos), { 0,1,0,0 });
	XMStoreFloat4x4(&m_LightVP, lightView * lightProj);
}

void ShadowMapRenderer::Begin(const GameContext& gameContext) const
{
	//Reset Texture Register 5 (Unbind)
	ID3D11ShaderResourceView* const pSRV[] = { nullptr };
	gameContext.pDeviceContext->PSSetShaderResources(1, 1, pSRV);

	//TODO: set the appropriate render target that our shadow generator will write to (hint: use the OverlordGame::SetRenderTarget function through SceneManager)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(m_pShadowRT);
	//TODO: clear this render target
	const FLOAT clearColor[4]{ 0.0f, 0.2f, 0.4f, 1.0f };
	m_pShadowRT->Clear(gameContext, clearColor);
	//TODO: set the shader variables of this shadow generator material
	m_pShadowMat->SetLightVP(m_LightVP);
	m_pShadowMat->SetWorld(gameContext.pCamera->GetTransform()->GetWorld());
}

void ShadowMapRenderer::End(const GameContext& gameContext) const
{
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: restore default render target (hint: passing nullptr to OverlordGame::SetRenderTarget will do the trick)
	SceneManager::GetInstance()->GetGame()->SetRenderTarget(nullptr);
}

void ShadowMapRenderer::Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, DirectX::XMFLOAT4X4 world, const std::vector<DirectX::XMFLOAT4X4>& bones) const
{
	//TODO: update shader variables in material
	m_pShadowMat->SetWorld(world);
	m_pShadowMat->SetBones(&bones.data()->_11, bones.size());
	m_pShadowMat->SetLightVP(m_LightVP);

	//TODO: set the correct inputlayout, buffers, topology (some variables are set based on the generation type Skinned or Static)
	auto shadowType = (pMeshFilter->m_HasAnimations) ? ShadowMapMaterial::Skinned : ShadowMapMaterial::Static;
	// Set VertexBuffer(static / skinned)
	auto vBuffer = pMeshFilter->GetVertexBufferData(gameContext, m_pShadowMat->m_InputLayoutIds[shadowType]);
	unsigned int offset = 0;
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &vBuffer.pVertexBuffer, &vBuffer.VertexStride, &offset);
	// Set InputLayout(static / skinned)
	gameContext.pDeviceContext->IASetInputLayout(m_pShadowMat->m_pInputLayouts[shadowType]);
	// Set IndexBuffer
	gameContext.pDeviceContext->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set PrimitiveTopology
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//TODO: invoke draw call
	// Retrieve Shader Technique(static / skinned)
	// Retrieve Technique Descriptor
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pShadowMat->m_pShadowTechs[shadowType]->GetDesc(&techDesc);
	// Iterate Technique Passes
	for (unsigned int p = 0; p < techDesc.Passes; ++p)
	{
		// Apply Pass(= prepping the pipeline)
		m_pShadowMat->m_pShadowTechs[shadowType]->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		// DRAW!(by using DrawIndexed(...))
		gameContext.pDeviceContext->DrawIndexed(pMeshFilter->m_IndexCount, 0, 0);
	}
}

void ShadowMapRenderer::UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter)
{
	//TODO: based on the type (Skinned or Static) build the correct vertex buffers for the MeshFilter (Hint use MeshFilter::BuildVertexBuffer)
	auto type = (pMeshFilter->m_HasAnimations) ? ShadowMapMaterial::Skinned : ShadowMapMaterial::Static;
	pMeshFilter->BuildVertexBuffer(gameContext, m_pShadowMat->m_InputLayoutIds[type], m_pShadowMat->m_InputLayoutSizes[type],
		m_pShadowMat->m_InputLayoutDescriptions[type]);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	//TODO: return the depth shader resource view of the shadow generator render target
	return m_pShadowRT->GetDepthShaderResourceView();
}

void ShadowMapRenderer::Translate(const DirectX::XMFLOAT3& pos)
{
	SetLight(pos, m_LightDirection);
}