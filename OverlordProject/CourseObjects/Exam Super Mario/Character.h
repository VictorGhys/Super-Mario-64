#pragma once
#include "GameObject.h"

class ParticleEmitterComponent;
class ModelAnimator;
class ControllerComponent;
class CameraComponent;

class Character : public GameObject
{
public:
	enum CharacterMovement : UINT
	{
		LEFT = 0,
		RIGHT,
		FORWARD,
		BACKWARD,
		JUMP,
		GROUND_POUND,
		CAMERA_UP,
		CAMERA_DOWN,
		CAMERA_LEFT,
		CAMERA_RIGHT,
		JUMP_CONTROLLER,
		GROUND_POUND_CONTROLLER
	};
	enum class MarioAnimation : UINT
	{
		IDLE = 0,
		JUMP = 1,
		WALK = 2,
		RUN = 3,
		RUN_HARD = 4
	};
	Character(float radius = 2, float height = 5, float moveSpeed = 100, bool thirdPerson = false, DirectX::XMFLOAT3 cameraOffset = { 0,0,-5 });
	virtual ~Character() = default;

	Character(const Character& other) = delete;
	Character(Character&& other) noexcept = delete;
	Character& operator=(const Character& other) = delete;
	Character& operator=(Character&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void PostInitialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;

	CameraComponent* GetCamera() const { return m_pCamera; }
	void SetAnimator(ModelAnimator* pAnimator);
	void SetVisuals(GameObject* visuals) { m_Visuals = visuals; }
	bool GetIsOnGround() const;
	void SetSmokeParticle(ParticleEmitterComponent* smokeParticle) { m_SmokeParticle = smokeParticle; }
	void SetIsPaused(bool isPaused) { m_IsPaused = isPaused; }
	bool GetHasEverMoved() const { return m_HasEverMoved; }
protected:
	CameraComponent* m_pCamera;
	ControllerComponent* m_pController;

	float m_TotalPitch, m_TotalYaw;
	float m_MoveSpeed, m_RotationSpeed;
	float m_Radius, m_Height;

	//Running
	float m_MaxRunVelocity,
		m_TerminalVelocity,
		m_NormalGravity,
		m_Gravity,
		m_RunAccelerationTime,
		m_JumpAccelerationTime,
		m_RunAcceleration,
		m_JumpAcceleration,
		m_RunVelocity,
		m_JumpVelocity;

	DirectX::XMFLOAT3 m_Velocity;

	//my own
	bool m_thirdPerson;
	DirectX::XMFLOAT3 m_CameraOffset;
	GameObject* m_pCameraBoomObject;
	const float m_InitJumpVelocity;
	ModelAnimator* m_pAnimator;
	GameObject* m_Visuals;
	const float m_MaxDoubleJumpTime;
	float m_DoubleJumpTime;
	const float m_DoubleJumpInitVelocity;
	float m_GroundPoundGravity;
	ParticleEmitterComponent* m_SmokeParticle;
	bool m_IsPaused;
	const float m_ArrowRotationSpeed;
	const float m_ControllerRotationSpeed;
	const float m_MaxPitch;
	const float m_MinPitch;
	bool m_HasEverMoved;
	FMOD::Sound* m_pHoo = nullptr;
	FMOD::Sound* m_pWoohoo = nullptr;
	FMOD::Sound* m_pYa = nullptr;
	FMOD::Channel* m_pChannel = nullptr;
};
