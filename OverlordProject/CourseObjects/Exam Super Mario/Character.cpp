#include "stdafx.h"
#include "Character.h"
#include "Components.h"
#include "Prefabs.h"
#include "ModelAnimator.h"
#include "PhysxManager.h"
#include "SoundManager.h"

Character::Character(float radius, float height, float moveSpeed, bool thirdPerson, DirectX::XMFLOAT3 cameraOffset)
	:m_Radius(radius),
	m_Height(height),
	m_MoveSpeed(moveSpeed),
	m_pCamera(nullptr),
	m_pController(nullptr),
	m_TotalPitch(0),
	m_TotalYaw(0),
	m_RotationSpeed(0.5f),//90
	//Running
	m_MaxRunVelocity(30.0f),//50
	m_TerminalVelocity(20),
	m_NormalGravity(9.81f),
	m_Gravity(m_NormalGravity),
	m_RunAccelerationTime(0.3f),
	m_JumpAccelerationTime(0.8f),
	m_RunAcceleration(m_MaxRunVelocity / m_RunAccelerationTime),
	m_JumpAcceleration(m_Gravity / m_JumpAccelerationTime),
	m_RunVelocity(0),
	m_JumpVelocity(0),
	m_Velocity(0, 0, 0),
	m_thirdPerson(thirdPerson),
	m_CameraOffset(cameraOffset),
	m_InitJumpVelocity(60), //200//50
	m_MaxDoubleJumpTime(0.2f),
	m_DoubleJumpTime(m_MaxDoubleJumpTime),
	m_DoubleJumpInitVelocity(100),//70
	m_GroundPoundGravity(900),
	m_IsPaused(false),
	m_ArrowRotationSpeed(90),
	m_ControllerRotationSpeed(150),
	m_SmokeParticle(nullptr),
	m_Visuals(nullptr),
	m_pAnimator(nullptr),
	m_pCameraBoomObject(nullptr),
	m_MaxPitch(70),
	m_MinPitch(0),
	m_HasEverMoved(false)
{}

void Character::Initialize(const GameContext& gameContext)
{
	//TODO: Create controller
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	m_pController = new ControllerComponent(physX->createMaterial(1, 1, 1), m_Radius, m_Height,
		L"Character", physx::PxCapsuleClimbingMode::eEASY);
	AddComponent(m_pController);
	//TODO: Add a fixed camera as child
	m_pCamera = new CameraComponent();
	GameObject* go = new GameObject();
	go->AddComponent(m_pCamera);
	m_pCameraBoomObject = new GameObject;
	m_pCameraBoomObject->AddChild(go);
	AddChild(m_pCameraBoomObject);
	go->GetTransform()->Translate(m_CameraOffset);
	go->GetTransform()->Rotate(10, 0, 0);
	//TODO: Register all Input Actions
	// movement
	gameContext.pInput->AddInputAction(InputAction(FORWARD, InputTriggerState::Down, 'W'));
	gameContext.pInput->AddInputAction(InputAction(BACKWARD, InputTriggerState::Down, 'S'));
	gameContext.pInput->AddInputAction(InputAction(LEFT, InputTriggerState::Down, 'A'));
	gameContext.pInput->AddInputAction(InputAction(RIGHT, InputTriggerState::Down, 'D'));
	gameContext.pInput->AddInputAction(InputAction(JUMP, InputTriggerState::Pressed, VK_SPACE));
	gameContext.pInput->AddInputAction(InputAction(GROUND_POUND, InputTriggerState::Pressed, VK_SHIFT));
	// controller actions
	gameContext.pInput->AddInputAction(InputAction(JUMP_CONTROLLER, InputTriggerState::Pressed, -1, -1, XINPUT_GAMEPAD_A));
	gameContext.pInput->AddInputAction(InputAction(GROUND_POUND_CONTROLLER, InputTriggerState::Pressed, -1, -1, XINPUT_GAMEPAD_B));
	// camera
	gameContext.pInput->AddInputAction(InputAction(CAMERA_UP, InputTriggerState::Down, VK_UP));
	gameContext.pInput->AddInputAction(InputAction(CAMERA_DOWN, InputTriggerState::Down, VK_DOWN));
	gameContext.pInput->AddInputAction(InputAction(CAMERA_LEFT, InputTriggerState::Down, VK_LEFT));
	gameContext.pInput->AddInputAction(InputAction(CAMERA_RIGHT, InputTriggerState::Down, VK_RIGHT));
	// sound
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/mario-hoo.wav", FMOD_2D, 0, &m_pHoo);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/mario-woohoo.wav", FMOD_2D, 0, &m_pWoohoo);
	SoundManager::GetInstance()->GetSystem()->createSound("./Resources/Sound/mario-ya.wav", FMOD_2D, 0, &m_pYa);
}

void Character::PostInitialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: Set the camera as active
	m_pCamera->SetActive();
	// We need to do this in the PostInitialize because child game objects only get initialized after the Initialize of the current object finishes
}

void Character::Update(const GameContext& gameContext)
{
	using namespace DirectX;
	//TODO: Update the character (Camera rotation, Character Movement, Character Gravity)
	if (!m_IsPaused && m_pCamera->IsActive())
	{
		//HANDLE INPUT
		auto move = DirectX::XMFLOAT2(0, 0);
		move.y = gameContext.pInput->IsActionTriggered(FORWARD) ? 1.0f : 0.0f;
		if (move.y == 0) move.y = -(gameContext.pInput->IsActionTriggered(BACKWARD) ? 1.0f : 0.0f);

		move.x = gameContext.pInput->IsActionTriggered(RIGHT) ? 1.0f : 0.0f;
		if (move.x == 0) move.x = -(gameContext.pInput->IsActionTriggered(LEFT) ? 1.0f : 0.0f);

		if (move.x == 0 && move.y == 0)
		{
			move = InputManager::GetThumbstickPosition(true);
		}

		auto look = DirectX::XMFLOAT2(0, 0);

		const auto mouseMove = InputManager::GetMouseMovement();
		look.x = static_cast<float>(mouseMove.x);
		look.y = static_cast<float>(mouseMove.y);

		if (look.x == 0 && look.y == 0)
		{
			look = InputManager::GetThumbstickPosition(false);
			look.x *= m_ControllerRotationSpeed * gameContext.pGameTime->GetElapsed();
			look.y *= m_ControllerRotationSpeed * gameContext.pGameTime->GetElapsed();
		}

		//CALCULATE TRANSFORMS
		/*const auto forward = XMLoadFloat3(&GetTransform()->GetForward());
		const auto right = XMLoadFloat3(&GetTransform()->GetRight());*/
		const auto forward = XMLoadFloat3(&m_pCameraBoomObject->GetTransform()->GetForward());//to move it the direction of the camera
		const auto right = XMLoadFloat3(&m_pCameraBoomObject->GetTransform()->GetRight());
		/*XMFLOAT3 worldForward = { 0,0,1 };
		XMFLOAT3 worldRight = { 1,0,0 };
		const XMVECTOR forward = XMLoadFloat3(&worldForward);
		const XMVECTOR right = XMLoadFloat3(&worldRight);
		auto currPos = XMLoadFloat3(&GetTransform()->GetPosition());*/
		XMVECTOR moveDir{};
		moveDir += forward * move.y;
		moveDir += right * move.x;
		bool isMoving = true;
		if (move.x != 0 || move.y != 0)
		{
			m_HasEverMoved = true;
			// Movement.
			m_RunVelocity += m_RunAcceleration * gameContext.pGameTime->GetElapsed();
			if (m_RunVelocity > m_MaxRunVelocity)
			{
				m_RunVelocity = m_MaxRunVelocity;
			}
			if (m_RunVelocity < -m_MaxRunVelocity)
			{
				m_RunVelocity = -m_MaxRunVelocity;
			}
			float temp = m_Velocity.y;
			XMStoreFloat3(&m_Velocity, moveDir * m_RunVelocity);
			m_Velocity.y = temp;
		}
		else
		{
			// No movement.
			m_Velocity.x = 0;
			m_Velocity.z = 0;
			m_RunVelocity = 0;
			isMoving = false;
			if (m_SmokeParticle)
				m_SmokeParticle->SetActive(false);
		}
		if (!(m_pController->GetCollisionFlags().isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN)))
		{
			// character is not on the ground
			if (gameContext.pInput->IsActionTriggered(GROUND_POUND) || gameContext.pInput->IsActionTriggered(GROUND_POUND_CONTROLLER))
			{
				if (m_JumpVelocity > 0)
				{
					m_JumpVelocity = 0;
				}
				m_Gravity = m_GroundPoundGravity;
			}
			m_JumpVelocity -= m_Gravity * gameContext.pGameTime->GetElapsed() * gameContext.pGameTime->GetElapsed() * 100;//correct
			if (m_JumpVelocity > m_TerminalVelocity)
			{
				m_JumpVelocity = m_TerminalVelocity;
			}
			if (m_JumpVelocity < -m_TerminalVelocity)
			{
				m_JumpVelocity = -m_TerminalVelocity;
			}
			if (m_SmokeParticle)
				m_SmokeParticle->SetActive(false);
		}
		else
		{
			// is on ground
			// increase double jump time
			m_DoubleJumpTime += gameContext.pGameTime->GetElapsed();
			m_Gravity = m_NormalGravity;
			if (gameContext.pInput->IsActionTriggered(JUMP) || gameContext.pInput->IsActionTriggered(JUMP_CONTROLLER))
			{
				// jump action triggered
				if (m_DoubleJumpTime < m_MaxDoubleJumpTime)
				{
					// do double jump
					m_JumpVelocity = 0;
					m_Velocity.y = m_DoubleJumpInitVelocity;
					// play jump animation
					if (m_pAnimator)
					{
						m_pAnimator->SetAnimation(static_cast<UINT>(MarioAnimation::JUMP));
						m_pAnimator->SetAnimationSpeed(1.7f);
						m_pAnimator->Play();
					}
					// reset double jump time
					m_DoubleJumpTime = 0;
					SoundManager::GetInstance()->GetSystem()->playSound(m_pWoohoo, nullptr, false, &m_pChannel);
				}
				else
				{
					// character wants to jump
					m_JumpVelocity = 0;
					m_Velocity.y = m_InitJumpVelocity;
					// play jump animation
					if (m_pAnimator)
					{
						m_pAnimator->SetAnimation(static_cast<UINT>(MarioAnimation::JUMP));
						m_pAnimator->SetAnimationSpeed(2.2f);
						m_pAnimator->Play();
					}
					// reset double jump time
					m_DoubleJumpTime = 0;
					SoundManager::GetInstance()->GetSystem()->playSound(m_pHoo, nullptr, false, &m_pChannel);
				}
			}
			else
			{
				// doesn't want to jump
				m_Velocity.y = 0;
				if (isMoving)
				{
					// play walk/run/run_hard animation
					if (m_pAnimator && m_pAnimator->GetClipNumber() != static_cast<UINT>(MarioAnimation::RUN))
					{
						// if not already playing this clip
						m_pAnimator->SetAnimation(static_cast<UINT>(MarioAnimation::RUN));
						m_pAnimator->Play();
					}
					if (m_SmokeParticle)
						m_SmokeParticle->SetActive(true);
				}
				else
				{
					// play idle animation
					if (m_pAnimator && m_pAnimator->GetClipNumber() != static_cast<UINT>(MarioAnimation::IDLE))
					{
						m_pAnimator->SetAnimation(static_cast<UINT>(MarioAnimation::IDLE));
						m_pAnimator->Play();
					}
				}
			}
		}
		m_Velocity.y += m_JumpVelocity;

		// arrow camera controls
		if (gameContext.pInput->IsActionTriggered(CAMERA_UP))
		{
			m_TotalPitch += m_ArrowRotationSpeed * gameContext.pGameTime->GetElapsed();
		}
		if (gameContext.pInput->IsActionTriggered(CAMERA_DOWN))
		{
			m_TotalPitch -= m_ArrowRotationSpeed * gameContext.pGameTime->GetElapsed();
		}
		if (gameContext.pInput->IsActionTriggered(CAMERA_LEFT))
		{
			m_TotalYaw -= m_ArrowRotationSpeed * gameContext.pGameTime->GetElapsed();
		}
		if (gameContext.pInput->IsActionTriggered(CAMERA_RIGHT))
		{
			m_TotalYaw += m_ArrowRotationSpeed * gameContext.pGameTime->GetElapsed();
		}

		m_TotalYaw += look.x * m_RotationSpeed /** gameContext.pGameTime->GetElapsed()*/;
		m_TotalPitch += look.y * m_RotationSpeed /** gameContext.pGameTime->GetElapsed()*/;

		if (m_TotalPitch < m_MinPitch)
		{
			m_TotalPitch = m_MinPitch;
		}
		if (m_TotalPitch > m_MaxPitch)
		{
			m_TotalPitch = m_MaxPitch;
		}
		m_pCameraBoomObject->GetTransform()->Rotate(m_TotalPitch, m_TotalYaw, 0);

		// turn the visuals to the move direction
		if (m_Velocity.x != 0 || m_Velocity.z != 0)
		{
			const float pi = 3.1415926535f;
			float angle = atan2(GetTransform()->GetForward().z, GetTransform()->GetForward().x) - atan2(m_Velocity.z, m_Velocity.x);
			angle *= (180 / pi);
			m_Visuals->GetTransform()->Rotate(0, angle + 180, 0);
		}

		auto temp = XMLoadFloat3(&m_Velocity);
		XMFLOAT3 displacement;
		XMStoreFloat3(&displacement, temp * gameContext.pGameTime->GetElapsed());//correct
		m_pController->Move(displacement);
	}
}
void Character::SetAnimator(ModelAnimator* pAnimator)
{
	m_pAnimator = pAnimator;
}
bool Character::GetIsOnGround() const
{
	return m_pController->GetCollisionFlags().isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
}