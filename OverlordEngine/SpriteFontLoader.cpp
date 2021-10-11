#include "stdafx.h"
#include "SpriteFontLoader.h"
#include "BinaryReader.h"
#include "ContentManager.h"
#include "TextureData.h"

SpriteFont* SpriteFontLoader::LoadContent(const std::wstring& assetFile)
{
	auto pBinReader = new BinaryReader();
	pBinReader->Open(assetFile);

	if (!pBinReader->Exists())
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Failed to read the assetFile!\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	char identification1 = pBinReader->Read<char>();
	char identification2 = pBinReader->Read<char>();
	char identification3 = pBinReader->Read<char>();
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	if (identification1 != 'B' && identification2 != 'M' && identification3 != 'F')
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid.fnt font");
		return nullptr;
	}

	//Parse the version (version 3 required)
	char version = pBinReader->Read<char>();
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	if (version < 3)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	//Valid .fnt file
	auto pSpriteFont = new SpriteFont();
	//SpriteFontLoader is a friend class of SpriteFont
	//That means you have access to its privates (pSpriteFont->m_FontName = ... is valid)

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	pBinReader->Read<char>();
	pBinReader->Read<int>();

	//Retrieve the FontSize (will be -25, BMF bug) [SpriteFont::m_FontSize]
	pSpriteFont->m_FontSize = pBinReader->Read<short>();
	
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	//Retrieve the FontName [SpriteFont::m_FontName]
	pBinReader->MoveBufferPosition(12);
	pSpriteFont->m_FontName = pBinReader->ReadNullString();

	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	pBinReader->Read<char>();
	pBinReader->Read<int>();

	//Retrieve Texture Width & Height [SpriteFont::m_TextureWidth/m_TextureHeight]
	pBinReader->MoveBufferPosition(4);
	//std::cout << sizeof(unsigned int);
	pSpriteFont->m_TextureWidth = pBinReader->Read<unsigned short int>();
	pSpriteFont->m_TextureHeight = pBinReader->Read<unsigned short int>();

	//Retrieve PageCount
	unsigned int pageCount = pBinReader->Read<unsigned short int>();
	//> if pagecount > 1
	//> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed)
	if (pageCount > 1)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed");
	}
	//Advance to Block2 (Move Reader)
	pBinReader->MoveBufferPosition(5);

	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	pBinReader->Read<char>();
	pBinReader->Read<int>();

	//Retrieve the PageName (store Local)
	std::wstring pageName = pBinReader->ReadNullString();
	//	> If PageName is empty
	//	> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty])
	if (pageName.size() <= 0)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty]");
	}
	//>Retrieve texture filepath from the assetFile path
	//> (ex. c:/Example/somefont.fnt => c:/Example/) [Have a look at: wstring::rfind()]
	std::wstring filePath = assetFile.substr(0, assetFile.rfind('/') + 1);
	//>Use path and PageName to load the texture using the ContentManager [SpriteFont::m_pTexture]
	//> (ex. c:/Example/ + 'PageName' => c:/Example/somefont_0.png)
	//...
	pSpriteFont->m_pTexture = ContentManager::Load<TextureData>(filePath + pageName);
	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	pBinReader->Read<char>();
	int block3size = pBinReader->Read<int>();

	//Retrieve Character Count (see documentation)
	int numChars = block3size / 20;
	//Retrieve Every Character, For every Character:
	for (int i = 0; i < numChars; ++i)
	{
		//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
		wchar_t charId = static_cast<wchar_t>( pBinReader->Read<unsigned int>());
		
		//> Check if CharacterId is valid (SpriteFont::IsCharValid), Log Warning and advance to next character if not valid
		if (!pSpriteFont->IsCharValid(charId))
		{
			Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Character");
			continue;
		}
		//> Retrieve the corresponding FontMetric (SpriteFont::GetMetric) [REFERENCE!!!]
		FontMetric& fontMetric = pSpriteFont->GetMetric(charId);
		//> Set IsValid to true [FontMetric::IsValid]
		fontMetric.IsValid = true;
		//> Set Character (CharacterId) [FontMetric::Character]
		fontMetric.Character = charId;
		//> Retrieve Xposition (store Local)
		unsigned int Xposition = pBinReader->Read<unsigned short int>();
		//std::cout << sizeof(unsigned short int) << std::endl;

		//> Retrieve Yposition (store Local)
		unsigned int Yposition = pBinReader->Read<unsigned short int>();

		//> Retrieve & Set Width [FontMetric::Width]
		fontMetric.Width = pBinReader->Read<unsigned short int>();

		//> Retrieve & Set Height [FontMetric::Height]
		fontMetric.Height = pBinReader->Read<unsigned short int>();

		//> Retrieve & Set OffsetX [FontMetric::OffsetX]
		fontMetric.OffsetX = pBinReader->Read<short int>();
		//std::cout << sizeof(short int) << std::endl;

		//> Retrieve & Set OffsetY [FontMetric::OffsetY]
		fontMetric.OffsetY = pBinReader->Read<short int>();

		//> Retrieve & Set AdvanceX [FontMetric::AdvanceX]
		fontMetric.AdvanceX = pBinReader->Read<short int>();

		//> Retrieve & Set Page [FontMetric::Page]
		fontMetric.Page = pBinReader->Read<unsigned char>();
		//std::cout << sizeof(unsigned char) << std::endl;

		//> Retrieve Channel (BITFIELD!!!)
		//	> See documentation for BitField meaning [FontMetrix::Channel]
		auto channel = pBinReader->Read<unsigned char>();
		switch (channel)
		{
		case 1:
			fontMetric.Channel = 2;
			break;
		case 2:
			fontMetric.Channel = 1;
			break;
		case 4:
			fontMetric.Channel = 0;
			break;
		case 8:
			fontMetric.Channel = 3;
			break;
		}

		//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]
		float texX = float(Xposition) / float(pSpriteFont->m_TextureWidth);
		float texY = float(Yposition) / float(pSpriteFont->m_TextureHeight);
		fontMetric.TexCoord = DirectX::XMFLOAT2(texX, texY);
	}
	//DONE :)

	delete pBinReader;
	return pSpriteFont;
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
