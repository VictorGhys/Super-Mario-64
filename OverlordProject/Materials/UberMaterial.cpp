#include "stdafx.h"
#include "UberMaterial.h"

#include "ContentManager.h"
#include "TextureData.h"

//LIGHT
//*****
ID3DX11EffectVectorVariable* UberMaterial::m_pLightDirectionVariable = nullptr;

//DIFFUSE
//*******
ID3DX11EffectScalarVariable* UberMaterial::m_pUseDiffuseTextureVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pDiffuseColorVariable = nullptr;

//SPECULAR
//********
ID3DX11EffectVectorVariable* UberMaterial::m_pSpecularColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseSpecularLevelTextureVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pSpecularLevelSRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pShininessVariable = nullptr;

//AMBIENT
//*******
ID3DX11EffectVectorVariable* UberMaterial::m_pAmbientColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pAmbientIntensityVariable = nullptr;

//NORMAL MAPPING
//**************
ID3DX11EffectScalarVariable* UberMaterial::m_pFlipGreenChannelVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseNormalMappingVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pNormalMappingSRVvariable = nullptr;

//ENVIRONMENT MAPPING
//*******************
ID3DX11EffectScalarVariable* UberMaterial::m_pUseEnvironmentMappingVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pEnvironmentSRVvariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pReflectionStrengthVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionStrengthVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pRefractionIndexVariable = nullptr;

//OPACITY
//***************
ID3DX11EffectScalarVariable* UberMaterial::m_pOpacityVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUseOpacityMapVariable = nullptr;
ID3DX11EffectShaderResourceVariable* UberMaterial::m_pOpacitySRVvariable = nullptr;

//SPECULAR MODELS
//***************
ID3DX11EffectScalarVariable* UberMaterial::m_pUseBlinnVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pUsePhongVariable = nullptr;

//FRESNEL FALLOFF
//***************
ID3DX11EffectScalarVariable* UberMaterial::m_pUseFresnelFalloffVariable = nullptr;
ID3DX11EffectVectorVariable* UberMaterial::m_pFresnelColorVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelPowerVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelMultiplierVariable = nullptr;
ID3DX11EffectScalarVariable* UberMaterial::m_pFresnelHardnessVariable = nullptr;

UberMaterial::UberMaterial()
	:Material(L"Resources/Effects/UberShader.fx")
	, m_pDiffuseTexture(nullptr)
	, m_pSpecularLevelTexture(nullptr)
	, m_pNormalMappingTexture(nullptr)
	, m_pEnvironmentCube(nullptr)
	, m_pOpacityMap(nullptr)
	, m_bDiffuseTexture(false)
	, m_bSpecularLevelTexture(false)
	, m_bFlipGreenChannel(false)
	, m_bNormalMapping(false)
	, m_bEnvironmentMapping(false)
	, m_bOpacityMap(false)
	, m_bSpecularBlinn(false)
	, m_bSpecularPhong(false)
	, m_bFresnelFaloff(false)
	, m_Opacity(1)
	, m_LightDirection(0.577f, 0.577f, 0.577f)
	, m_ColorDiffuse(1, 1, 1, 1)
	, m_ColorSpecular(1, 1, 1, 1)
	, m_Shininess(15)
	, m_ColorAmbient(0, 0, 0, 1)
	, m_ColorFresnel(1, 1, 1, 1)
	, m_FresnelPower(1)
	, m_FresnelMultiplier(1)
	, m_ReflectionStrength(0)
	, m_RefractionStrength(0)
	, m_RefractionIndex(0.3f)
	, m_AmbientIntensity(0)
	, m_FresnelHardness(0)
{
}
UberMaterial::~UberMaterial()
{
	/*if (m_pLightDirectionVariable != nullptr)
		m_pLightDirectionVariable->Release();*/
		//m_pDiffuseSRVvariable->Release();
	/*SafeRelease(m_pDiffuseSRVvariable);
	m_pSpecularLevelSRVvariable->Release();
	m_pNormalMappingSRVvariable->Release();
	m_pEnvironmentSRVvariable->Release();
	m_pOpacitySRVvariable->Release();*/
}
void UberMaterial::SetLightDirection(DirectX::XMFLOAT3 direction)
{
	m_LightDirection = direction;
}
void UberMaterial::EnableDiffuseTexture(bool enable)
{
	m_bDiffuseTexture = enable;
}
void UberMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<TextureData>(assetFile);
}
void UberMaterial::SetDiffuseColor(DirectX::XMFLOAT4 color)
{
	m_ColorDiffuse = color;
}
void UberMaterial::SetSpecularColor(DirectX::XMFLOAT4 color)
{
	m_ColorSpecular = color;
}
void UberMaterial::EnableSpecularLevelTexture(bool enable)
{
	m_bSpecularLevelTexture = enable;
}
void UberMaterial::SetSpecularLevelTexture(const std::wstring& assetFile)
{
	m_pSpecularLevelTexture = ContentManager::Load<TextureData>(assetFile);
}
void UberMaterial::SetShininess(int shininess)
{
	m_Shininess = shininess;
}
void UberMaterial::SetAmbientColor(DirectX::XMFLOAT4 color)
{
	m_ColorAmbient = color;
}
void UberMaterial::SetAmbientIntensity(float intensity)
{
	m_AmbientIntensity = intensity;
}
void UberMaterial::FlipNormalGreenCHannel(bool flip)
{
	m_bFlipGreenChannel = flip;
}
void UberMaterial::EnableNormalMapping(bool enable)
{
	m_bNormalMapping = enable;
}
void UberMaterial::SetNormalMapTexture(const std::wstring& assetFile)
{
	m_pNormalMappingTexture = ContentManager::Load<TextureData>(assetFile);
}
void UberMaterial::EnableEnvironmentMapping(bool enable)
{
	m_bEnvironmentMapping = enable;
}
void UberMaterial::SetEnvironmentCube(const std::wstring& assetFile)
{
	m_pEnvironmentCube = ContentManager::Load<TextureData>(assetFile);
}
void UberMaterial::SetReflectionStrength(float strength)
{
	m_ReflectionStrength = strength;
}
void UberMaterial::SetRefractionStrength(float strength)
{
	m_RefractionStrength = strength;
}
void UberMaterial::SetRefractionIndex(float index)
{
	m_RefractionIndex = index;
}
void UberMaterial::SetOpacity(float opacity)
{
	m_Opacity = opacity;
}
void UberMaterial::EnableOpacityMap(bool enable)
{
	m_bOpacityMap = enable;
}
void UberMaterial::SetOpacityTexture(const std::wstring& assetFile)
{
	m_pOpacityMap = ContentManager::Load<TextureData>(assetFile);
}
void UberMaterial::EnableSpecularBlinn(bool enable)
{
	m_bSpecularBlinn = enable;
}
void UberMaterial::EnableSpecularPhong(bool enable)
{
	m_bSpecularPhong = enable;
}
void UberMaterial::EnableFresnelFaloff(bool enable)
{
	m_bFresnelFaloff = enable;
}
void UberMaterial::SetFresnelColor(DirectX::XMFLOAT4 color)
{
	m_ColorFresnel = color;
}
void UberMaterial::SetFresnelPower(float power)
{
	m_FresnelPower = power;
}

void UberMaterial::SetFresnelMultiplier(float multiplier)
{
	m_FresnelMultiplier = multiplier;
}

void UberMaterial::SetFresnelHardness(float hardness)
{
	m_FresnelHardness = hardness;
}

void UberMaterial::LoadEffectVariables()
{
	//LIGHT
	//*****
	if (!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if (!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gLightDirection\' variable not found!");
			m_pLightDirectionVariable = nullptr;
		}
	}
	//DIFFUSE
	//*******
	if (!m_pUseDiffuseTextureVariable)
	{
		m_pUseDiffuseTextureVariable = GetEffect()->GetVariableByName("gUseTextureDiffuse")->AsScalar();
		if (!m_pUseDiffuseTextureVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseTextureDiffuse\' variable not found!");
			m_pUseDiffuseTextureVariable = nullptr;
		}
	}
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gTextureDiffuse")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gTextureDiffuse\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}
	if (!m_pDiffuseColorVariable)
	{
		m_pDiffuseColorVariable = GetEffect()->GetVariableByName("gColorDiffuse")->AsVector();
		if (!m_pDiffuseColorVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gColorDiffuse\' variable not found!");
			m_pDiffuseColorVariable = nullptr;
		}
	}
	//SPECULAR
	//********
	if (!m_pSpecularColorVariable)
	{
		m_pSpecularColorVariable = GetEffect()->GetVariableByName("gColorSpecular")->AsVector();
		if (!m_pSpecularColorVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gColorSpecular\' variable not found!");
			m_pSpecularColorVariable = nullptr;
		}
	}
	if (!m_pUseSpecularLevelTextureVariable)
	{
		m_pUseSpecularLevelTextureVariable = GetEffect()->GetVariableByName("gUseTextureSpecularIntensity")->AsScalar();
		if (!m_pUseSpecularLevelTextureVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseTextureSpecularIntensity\' variable not found!");
			m_pUseSpecularLevelTextureVariable = nullptr;
		}
	}
	if (!m_pSpecularLevelSRVvariable)
	{
		m_pSpecularLevelSRVvariable = GetEffect()->GetVariableByName("gTextureSpecularIntensity")->AsShaderResource();
		if (!m_pSpecularLevelSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gTextureSpecularIntensity\' variable not found!");
			m_pSpecularLevelSRVvariable = nullptr;
		}
	}
	if (!m_pShininessVariable)
	{
		m_pShininessVariable = GetEffect()->GetVariableByName("gShininess")->AsScalar();
		if (!m_pShininessVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gShininess\' variable not found!");
			m_pShininessVariable = nullptr;
		}
	}
	//AMBIENT
	//*******
	if (!m_pAmbientColorVariable)
	{
		m_pAmbientColorVariable = GetEffect()->GetVariableByName("gColorAmbient")->AsVector();
		if (!m_pAmbientColorVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gColorAmbient\' variable not found!");
			m_pAmbientColorVariable = nullptr;
		}
	}
	if (!m_pAmbientIntensityVariable)
	{
		m_pAmbientIntensityVariable = GetEffect()->GetVariableByName("gAmbientIntensity")->AsScalar();
		if (!m_pAmbientIntensityVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gAmbientIntensity\' variable not found!");
			m_pAmbientIntensityVariable = nullptr;
		}
	}
	//NORMAL MAPPING
	//**************
	if (!m_pFlipGreenChannelVariable)
	{
		m_pFlipGreenChannelVariable = GetEffect()->GetVariableByName("gFlipGreenChannel")->AsScalar();
		if (!m_pFlipGreenChannelVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gFlipGreenChannel\' variable not found!");
			m_pFlipGreenChannelVariable = nullptr;
		}
	}
	if (!m_pUseNormalMappingVariable)
	{
		m_pUseNormalMappingVariable = GetEffect()->GetVariableByName("gUseTextureNormal")->AsScalar();
		if (!m_pUseNormalMappingVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseTextureNormal\' variable not found!");
			m_pUseNormalMappingVariable = nullptr;
		}
	}
	if (!m_pNormalMappingSRVvariable)
	{
		m_pNormalMappingSRVvariable = GetEffect()->GetVariableByName("gTextureNormal")->AsShaderResource();
		if (!m_pNormalMappingSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gTextureNormal\' variable not found!");
			m_pNormalMappingSRVvariable = nullptr;
		}
	}
	//ENVIRONMENT MAPPING
	//*******************
	if (!m_pUseEnvironmentMappingVariable)
	{
		m_pUseEnvironmentMappingVariable = GetEffect()->GetVariableByName("gUseEnvironmentMapping")->AsScalar();
		if (!m_pUseEnvironmentMappingVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseEnvironmentMapping\' variable not found!");
			m_pUseEnvironmentMappingVariable = nullptr;
		}
	}
	if (!m_pEnvironmentSRVvariable)
	{
		m_pEnvironmentSRVvariable = GetEffect()->GetVariableByName("gCubeEnvironment")->AsShaderResource();
		if (!m_pEnvironmentSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gCubeEnvironment\' variable not found!");
			m_pEnvironmentSRVvariable = nullptr;
		}
	}
	if (!m_pReflectionStrengthVariable)
	{
		m_pReflectionStrengthVariable = GetEffect()->GetVariableByName("gReflectionStrength")->AsScalar();
		if (!m_pReflectionStrengthVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gReflectionStrength\' variable not found!");
			m_pReflectionStrengthVariable = nullptr;
		}
	}
	if (!m_pRefractionStrengthVariable)
	{
		m_pRefractionStrengthVariable = GetEffect()->GetVariableByName("gRefractionStrength")->AsScalar();
		if (!m_pRefractionStrengthVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gRefractionStrength\' variable not found!");
			m_pRefractionStrengthVariable = nullptr;
		}
	}
	if (!m_pRefractionIndexVariable)
	{
		m_pRefractionIndexVariable = GetEffect()->GetVariableByName("gRefractionIndex")->AsScalar();
		if (!m_pRefractionIndexVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gRefractionIndex\' variable not found!");
			m_pRefractionIndexVariable = nullptr;
		}
	}
	//OPACITY
	//***************
	if (!m_pOpacityVariable)
	{
		m_pOpacityVariable = GetEffect()->GetVariableByName("gOpacityIntensity")->AsScalar();
		if (!m_pOpacityVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gOpacityIntensity\' variable not found!");
			m_pOpacityVariable = nullptr;
		}
	}
	if (!m_pUseOpacityMapVariable)
	{
		m_pUseOpacityMapVariable = GetEffect()->GetVariableByName("gTextureOpacityIntensity")->AsScalar();
		if (!m_pUseOpacityMapVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gTextureOpacityIntensity\' variable not found!");
			m_pUseOpacityMapVariable = nullptr;
		}
	}
	if (!m_pOpacitySRVvariable)
	{
		m_pOpacitySRVvariable = GetEffect()->GetVariableByName("gTextureOpacity")->AsShaderResource();
		if (!m_pOpacitySRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gTextureOpacity\' variable not found!");
			m_pOpacitySRVvariable = nullptr;
		}
	}
	//SPECULAR MODELS
	//***************
	if (!m_pUseBlinnVariable)
	{
		m_pUseBlinnVariable = GetEffect()->GetVariableByName("gUseSpecularBlinn")->AsScalar();
		if (!m_pUseBlinnVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseSpecularBlinn\' variable not found!");
			m_pUseBlinnVariable = nullptr;
		}
	}
	if (!m_pUsePhongVariable)
	{
		m_pUsePhongVariable = GetEffect()->GetVariableByName("gUseSpecularPhong")->AsScalar();
		if (!m_pUsePhongVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseSpecularPhong\' variable not found!");
			m_pUsePhongVariable = nullptr;
		}
	}
	//FRESNEL FALLOFF
	//***************
	if (!m_pUseFresnelFalloffVariable)
	{
		m_pUseFresnelFalloffVariable = GetEffect()->GetVariableByName("gUseFresnelFalloff")->AsScalar();
		if (!m_pUseFresnelFalloffVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gUseFresnelFalloff\' variable not found!");
			m_pUseFresnelFalloffVariable = nullptr;
		}
	}
	if (!m_pFresnelColorVariable)
	{
		m_pFresnelColorVariable = GetEffect()->GetVariableByName("gColorFresnel")->AsVector();
		if (!m_pFresnelColorVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gColorFresnel\' variable not found!");
			m_pFresnelColorVariable = nullptr;
		}
	}
	if (!m_pFresnelPowerVariable)
	{
		m_pFresnelPowerVariable = GetEffect()->GetVariableByName("gFresnelPower")->AsScalar();
		if (!m_pFresnelPowerVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gFresnelPower\' variable not found!");
			m_pFresnelPowerVariable = nullptr;
		}
	}
	if (!m_pFresnelMultiplierVariable)
	{
		m_pFresnelMultiplierVariable = GetEffect()->GetVariableByName("gFresnelMultiplier")->AsScalar();
		if (!m_pFresnelMultiplierVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gFresnelMultiplier\' variable not found!");
			m_pFresnelMultiplierVariable = nullptr;
		}
	}
	if (!m_pFresnelHardnessVariable)
	{
		m_pFresnelHardnessVariable = GetEffect()->GetVariableByName("gFresnelHardness")->AsScalar();
		if (!m_pFresnelHardnessVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gFresnelHardness\' variable not found!");
			m_pFresnelHardnessVariable = nullptr;
		}
	}
}

void UberMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);
	//LIGHT
	//*****
	if (m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	}
	//DIFFUSE
	//*******
	if (m_pUseDiffuseTextureVariable)
	{
		m_pUseDiffuseTextureVariable->SetBool(m_bDiffuseTexture);
	}
	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(m_pDiffuseTexture->GetShaderResourceView());
	}
	if (m_pDiffuseColorVariable)
	{
		m_pDiffuseColorVariable->SetFloatVector(&m_ColorDiffuse.x);
	}
	//SPECULAR
	//********
	if (m_pSpecularColorVariable)
	{
		m_pSpecularColorVariable->SetFloatVector(&m_ColorSpecular.x);
	}
	if (m_pUseSpecularLevelTextureVariable)
	{
		m_pUseSpecularLevelTextureVariable->SetBool(m_bSpecularLevelTexture);
	}
	if (m_pSpecularLevelTexture && m_pSpecularLevelSRVvariable)
	{
		m_pSpecularLevelSRVvariable->SetResource(m_pSpecularLevelTexture->GetShaderResourceView());
	}
	if (m_pShininessVariable)
	{
		m_pShininessVariable->SetInt(m_Shininess);
	}
	//AMBIENT
	//*******
	if (m_pAmbientColorVariable)
	{
		m_pAmbientColorVariable->SetFloatVector(&m_ColorAmbient.x);
	}
	if (m_pAmbientIntensityVariable)
	{
		m_pAmbientIntensityVariable->SetFloat(m_AmbientIntensity);
	}
	//NORMAL MAPPING
	//**************
	if (m_pFlipGreenChannelVariable)
	{
		m_pFlipGreenChannelVariable->SetBool(m_bFlipGreenChannel);
	}
	if (m_pUseNormalMappingVariable)
	{
		m_pUseNormalMappingVariable->SetBool(m_bNormalMapping);
	}
	if (m_pNormalMappingTexture && m_pNormalMappingSRVvariable)
	{
		m_pNormalMappingSRVvariable->SetResource(m_pNormalMappingTexture->GetShaderResourceView());
	}
	//ENVIRONMENT MAPPING
	//*******************
	if (m_pUseEnvironmentMappingVariable)
	{
		m_pUseEnvironmentMappingVariable->SetBool(m_bEnvironmentMapping);
	}
	if (m_pEnvironmentCube && m_pEnvironmentSRVvariable)
	{
		m_pEnvironmentSRVvariable->SetResource(m_pEnvironmentCube->GetShaderResourceView());
	}
	if (m_pReflectionStrengthVariable)
	{
		m_pReflectionStrengthVariable->SetFloat(m_ReflectionStrength);
	}
	if (m_pRefractionStrengthVariable)
	{
		m_pRefractionStrengthVariable->SetFloat(m_RefractionStrength);
	}
	if (m_pRefractionIndexVariable)
	{
		m_pRefractionIndexVariable->SetFloat(m_RefractionIndex);
	}
	////OPACITY
	////***************
	if (m_pOpacityVariable)
	{
		m_pOpacityVariable->SetFloat(m_Opacity);
	}
	if (m_pUseOpacityMapVariable)
	{
		m_pUseOpacityMapVariable->SetBool(m_bOpacityMap);
	}
	if (m_pOpacityMap && m_pOpacitySRVvariable)
	{
		m_pOpacitySRVvariable->SetResource(m_pOpacityMap->GetShaderResourceView());
	}
	//SPECULAR MODELS
	//***************
	if (m_pUseBlinnVariable)
	{
		m_pUseBlinnVariable->SetBool(m_bSpecularBlinn);
	}
	if (m_pUsePhongVariable)
	{
		m_pUsePhongVariable->SetBool(m_bSpecularPhong);
	}
	//FRESNEL FALLOFF
	//***************
	if (m_pUseFresnelFalloffVariable)
	{
		m_pUseFresnelFalloffVariable->SetBool(m_bFresnelFaloff);
	}
	if (m_pFresnelColorVariable)
	{
		m_pFresnelColorVariable->SetFloatVector(&m_ColorFresnel.x);
	}
	if (m_pFresnelPowerVariable)
	{
		m_pFresnelPowerVariable->SetFloat(m_FresnelPower);
	}
	if (m_pFresnelMultiplierVariable)
	{
		m_pFresnelMultiplierVariable->SetFloat(m_FresnelMultiplier);
	}
	if (m_pFresnelHardnessVariable)
	{
		m_pFresnelHardnessVariable->SetFloat(m_FresnelHardness);
	}
}