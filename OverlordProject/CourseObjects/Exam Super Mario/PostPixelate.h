#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostPixelate : public PostProcessingMaterial
{
public:
	PostPixelate(const DirectX::XMFLOAT2& pixels);
	PostPixelate(const PostPixelate& other) = delete;
	PostPixelate(PostPixelate&& other) noexcept = delete;
	PostPixelate& operator=(const PostPixelate& other) = delete;
	PostPixelate& operator=(PostPixelate&& other) noexcept = delete;
	virtual ~PostPixelate() = default;

protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(RenderTarget* pRendertarget) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
	ID3DX11EffectVectorVariable* m_pPixelsVariable;
	DirectX::XMFLOAT2 m_Pixels;
};
