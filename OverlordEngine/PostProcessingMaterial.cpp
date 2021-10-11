#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"

PostProcessingMaterial::PostProcessingMaterial(std::wstring effectFile, unsigned int renderIndex,
	std::wstring technique)
	: m_IsInitialized(false),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_effectFile(std::move(effectFile)),
	m_InputLayoutID(0),
	m_RenderIndex(renderIndex),
	m_pRenderTarget(nullptr),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_NumVertices(0),
	m_NumIndices(0),
	m_VertexBufferStride(sizeof(VertexPosTex)),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_TechniqueName(std::move(technique))
{
}

PostProcessingMaterial::~PostProcessingMaterial()
{
	//TODO: delete and/or release necessary objects and/or resources
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pInputLayout->Release();
	SafeDelete(m_pRenderTarget);
}

void PostProcessingMaterial::Initialize(const GameContext& gameContext)
{
	if (!m_IsInitialized)
	{
		//TODO: complete
		//1. LoadEffect (LoadEffect(...))
		LoadEffect(gameContext, m_effectFile);
		//2. CreateInputLaytout (CreateInputLayout(...))
		EffectHelper::BuildInputLayout(gameContext.pDevice, m_pTechnique, &m_pInputLayout, m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID);
		//   CreateVertexBuffer (CreateVertexBuffer(...)) > As a TriangleStrip (FullScreen Quad)
		CreateVertexBuffer(gameContext);
		// CreateIndexBuffer
		CreateIndexBuffer(gameContext);
		//3. Create RenderTarget (m_pRenderTarget)
		//		Take a look at the class, figure out how to initialize/create a RenderTarget Object
		//		GameSettings > OverlordGame::GetGameSettings()
		m_pRenderTarget = new RenderTarget(gameContext.pDevice);
		RENDERTARGET_DESC desc{};
		desc.EnableColorSRV = true;
		desc.EnableDepthSRV = false;
		desc.Height = OverlordGame::GetGameSettings().Window.Height;
		desc.Width = OverlordGame::GetGameSettings().Window.Width;
		desc.GenerateMipMaps_Color = true;
		m_pRenderTarget->Create(desc);

		m_IsInitialized = true;
	}
}

bool PostProcessingMaterial::LoadEffect(const GameContext& gameContext, const std::wstring& effectFile)
{
	UNREFERENCED_PARAMETER(gameContext);

	//TODO: complete
	//Load Effect through ContentManager
	m_pEffect = ContentManager::Load<ID3DX11Effect>(effectFile);
	//Check if m_TechniqueName (default constructor parameter) is set
	if (!m_TechniqueName.empty())
	{
		// If SET > Use this Technique (+ check if valid)
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string techString = converter.to_bytes(m_TechniqueName);
		//auto techString = std::string(m_TechniqueName.begin(), m_TechniqueName.end());
		m_pTechnique = m_pEffect->GetTechniqueByName(techString.c_str());
	}
	else
	{
		// If !SET > Use Technique with index 0
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	}
	//Call LoadEffectVariables
	LoadEffectVariables();
	return true;
}

void PostProcessingMaterial::Draw(const GameContext& gameContext, RenderTarget* previousRendertarget)
{
	//TODO: complete
	//1. Clear the object's RenderTarget (m_pRenderTarget) [Check RenderTarget Class]
	const FLOAT clearColor[4]{ 0.0f, 0.2f, 0.4f, 1.0f };
	m_pRenderTarget->Clear(gameContext, clearColor);
	//2. Call UpdateEffectVariables(...)
	UpdateEffectVariables(previousRendertarget);
	//3. Set InputLayout
	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);
	//4. Set VertexBuffer
	unsigned int offset = 0;
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_VertexBufferStride, &offset);
	// Set IndexBuffer
	gameContext.pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//5. Set PrimitiveTopology (TRIANGLELIST)
	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//6. Draw
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	// Iterate Technique Passes
	for (unsigned int p = 0; p < techDesc.Passes; ++p)
	{
		// Apply Pass(= prepping the pipeline)
		m_pTechnique->GetPassByIndex(p)->Apply(0, gameContext.pDeviceContext);
		// DRAW!(by using DrawIndexed(...))
		gameContext.pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}

	// Generate Mips
	gameContext.pDeviceContext->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	m_NumVertices = 4;

	//TODO: complete
	//Create vertex array containing four elements in system memory
	VertexPosTex vertices[4]
	{
		VertexPosTex{{-1,1,0}, {0,0}},
		VertexPosTex{{1,1,0}, {1,0}},
		VertexPosTex{{1,-1,0}, {1,1}},
		VertexPosTex{{-1,-1,0}, {0,1}}
	};

	//fill a buffer description to copy the vertexdata into graphics memory
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VertexPosTex) * m_NumVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	//create a ID3D10Buffer in graphics memory containing the vertex info
	D3D11_SUBRESOURCE_DATA initData;
	//initData.pSysMem = malloc(sizeof(VertexPosTex) * m_NumVertices);
	initData.pSysMem = vertices;

	//create a ID3D10Buffer in graphics memory containing the vertex info
	gameContext.pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
}

void PostProcessingMaterial::CreateIndexBuffer(const GameContext& gameContext)
{
	m_NumIndices = 6;

	//TODO: complete
	// Create index buffer
	//UINT indices[6]{ 0,3,1,1,3,2 }; // counter clockwise
	UINT indices[6]{ 0,1,3,1,2,3 }; // clockwise

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(UINT) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	//initData.pSysMem = malloc(sizeof(DWORD) * m_NumVertices);
	initData.pSysMem = indices;

	gameContext.pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
}