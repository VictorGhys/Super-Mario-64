#include "stdafx.h"
#include "PostPixelate.h"
#include "RenderTarget.h"

PostPixelate::PostPixelate(const DirectX::XMFLOAT2& pixels)
	: PostProcessingMaterial(L"./Resources/Effects/Post/Pixelate.fx", 1),
	m_pTextureMapVariabele(nullptr),
	m_Pixels(pixels)
{
}

void PostPixelate::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	if (!m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
		if (!m_pTextureMapVariabele->IsValid())
		{
			Logger::LogWarning(L"PostPixelate::LoadEffectVariables() > \'gTexture\' variable not found!");
			m_pTextureMapVariabele = nullptr;
		}
		m_pPixelsVariable = GetEffect()->GetVariableByName("gPixels")->AsVector();
		if (!m_pPixelsVariable->IsValid())
		{
			Logger::LogError(L"SpriteRenderer::Initialize() > Shader variable \'gTextureSize\' not valid!");
			return;
		}
	}
}

void PostPixelate::UpdateEffectVariables(RenderTarget* pRendertarget)
{
	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (pRendertarget && m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pRendertarget->GetShaderResourceView());
	}
	if (pRendertarget && m_pPixelsVariable)
	{
		m_pPixelsVariable->SetFloatVector(reinterpret_cast<float*>(&m_Pixels));
	}
}