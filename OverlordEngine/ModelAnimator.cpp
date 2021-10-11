#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter) :
	m_pMeshFilter(pMeshFilter),
	m_Transforms(std::vector<DirectX::XMFLOAT4X4>()),
	m_IsPlaying(false),
	m_Reversed(false),
	m_ClipSet(false),
	m_TickCount(0),
	m_AnimationSpeed(1.0f),
	m_CurrentClipNumber(0)
{
	SetAnimation(0);
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (clipNumber >= GetClipCount())
	{
		//If not,
			//	Call Reset
		Reset();
		//	Log a warning with an appropriate message
		Logger::Log(LogLevel::Error, L"clipnumber was bigger than the clipsize");
		//	return
		return;
	}
	//else
	else
	{
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		AnimationClip clip = m_pMeshFilter->m_AnimationClips.at(clipNumber);
		//	Call SetAnimation(AnimationClip clip)
		SetAnimation(clip);
		m_CurrentClipNumber = clipNumber;
	}
}

void ModelAnimator::SetAnimation(std::wstring clipName)
{
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	auto It = std::find_if(m_pMeshFilter->m_AnimationClips.begin(), m_pMeshFilter->m_AnimationClips.end(), [clipName](AnimationClip clip)
		{
			return clip.Name == clipName;
		});
	if (It != m_pMeshFilter->m_AnimationClips.end())
	{
		//If found,
		//	Call SetAnimation(Animation Clip) with the found clip
		SetAnimation(*It);
	}
	else
	{
		//Else
		//	Call Reset
		Reset();
		//	Log a warning with an appropriate message}
		Logger::Log(LogLevel::Error, L"clip with name " + clipName + L" was not found");
	}
}

void ModelAnimator::SetAnimation(AnimationClip clip)
{
	//TODO: complete
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	//Call Reset(false)
	Reset();
}

void ModelAnimator::Reset(bool pause)
{
	//TODO: complete
	//If pause is true, set m_IsPlaying to false
	if (pause)
	{
		m_IsPlaying = false;
	}
	//Set m_TickCount to zero
	m_TickCount = 0;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.f;
	if (m_ClipSet)
	{
		//If m_ClipSet is true
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		auto boneTransform = m_CurrentClip.Keys.front().BoneTransforms;
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(boneTransform.begin(), boneTransform.end());
	}
	else
	{
		//Else
		//	Create an IdentityMatrix
		DirectX::XMFLOAT4X4 identity;
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
		//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identity);
	}
}

void ModelAnimator::Update(const GameContext& gameContext)
{
	//TODO: complete
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1.
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = gameContext.pGameTime->GetElapsed() * m_CurrentClip.TicksPerSecond * m_AnimationSpeed;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.Duration);
		//2.
		if (m_Reversed)
		{
			//IF m_Reversed is true
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			if (m_TickCount < 0)
			{
				m_TickCount += m_CurrentClip.Duration;
			}
		}
		else
		{
			//ELSE
			//	Add passedTicks to m_TickCount
			m_TickCount += passedTicks;
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			if (m_TickCount > m_CurrentClip.Duration)
			{
				m_TickCount -= m_CurrentClip.Duration;
			}
		}

		//3.
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		for (auto key : m_CurrentClip.Keys)
		{
			//keyA > Closest Key with Tick before/smaller than m_TickCount
			if (key.Tick < m_TickCount)
			{
				keyA = key;
			}
			//keyB > Closest Key with Tick after/bigger than m_TickCount
			if (key.Tick >= m_TickCount)
			{
				keyB = key;
				break;
			}
		}
		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		float blendB = (m_TickCount - keyA.Tick) / (keyB.Tick - keyA.Tick);
		//float blendA = 1 - blendB;

		using namespace DirectX;
		//Clear the m_Transforms vector
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (int b{}; b < m_pMeshFilter->m_BoneCount; b++)
		{
			//	Retrieve the transform from keyA (transformA)
			auto transformA = XMLoadFloat4x4(&keyA.BoneTransforms.at(b));
			// 	Retrieve the transform from keyB (transformB)
			auto transformB = XMLoadFloat4x4(&keyB.BoneTransforms.at(b));
			//	Decompose both transforms
			XMVECTOR scaleA, rotationA, translationA;
			XMMatrixDecompose(&scaleA, &rotationA, &translationA, transformA);
			XMVECTOR scaleB, rotationB, translationB;
			XMMatrixDecompose(&scaleB, &rotationB, &translationB, transformB);
			//	Lerp between all the transformations (Position, Scale, Rotation)
			XMVECTOR scaleLerp = XMVectorLerp(scaleA, scaleB, blendB);
			XMVECTOR rotationLerp = XMQuaternionSlerp(rotationA, rotationB, blendB);
			XMVECTOR translationLerp = XMVectorLerp(translationA, translationB, blendB);
			//	Compose a transformation matrix with the lerp-results
			XMFLOAT4X4 transformation;
			XMStoreFloat4x4(&transformation, XMMatrixScalingFromVector(scaleLerp) * XMMatrixRotationQuaternion(rotationLerp) * XMMatrixTranslationFromVector(translationLerp));
			//	Add the resulting matrix to the m_Transforms vector
			m_Transforms.push_back(transformation);
		}
	}
}