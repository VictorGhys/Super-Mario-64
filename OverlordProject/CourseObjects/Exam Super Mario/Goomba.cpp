#include "stdafx.h"
#include "Goomba.h"

#include "ModelComponent.h"
#include "PhysxManager.h"
#include "RigidBodyComponent.h"
#include "SuperMarioScene.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"

void killGoomba(GameObject* triggerobject, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"mario")
	{
		//std::cout << "goomba " << triggerobject << " dead\n";
		//std::cout << "killed by: " << otherobject << std::endl;
		//static_cast<SuperMarioScene*>(otherobject->GetScene())->RemoveChild(triggerobject->GetParent());
		using namespace DirectX;
		static_cast<SuperMarioScene*>(otherobject->GetScene())->AddToDelete(triggerobject->GetParent());
		XMFLOAT3 spawnPosCoin;
		XMStoreFloat3(&spawnPosCoin, XMLoadFloat3(&triggerobject->GetTransform()->GetPosition()) + XMVECTOR{ 0,-2,3 });
		static_cast<SuperMarioScene*>(otherobject->GetScene())->SpawnCoinDelayed(spawnPosCoin);
		static_cast<SuperMarioScene*>(otherobject->GetScene())->PlayGoombaSound();
	}
}
void hitGoomba(GameObject* triggerobject, GameObject* otherobject, GameObject::TriggerAction)
{
	if (otherobject->GetTag() == L"mario")
	{
		//std::cout << "ball hit mario\n";
		static_cast<SuperMarioScene*>(otherobject->GetScene())->RemoveLives(1);
		static_cast<Goomba*>(triggerobject)->SetHasHitPlayer(true);
		static_cast<Goomba*>(triggerobject)->ResetAttackCooldown();
	}
}

Goomba::Goomba(DirectX::XMFLOAT3 spawnPos, GameObject* mario, DirectX::XMFLOAT3 moveDirection)
	:m_SpawnPos(spawnPos),
	m_MoveDirection(moveDirection),
	m_MoveLength(10),
	m_pMario(mario),
	m_AttackDistance(12),
	m_AttackMoveSpeed(10),
	m_MaxAttackCooldown(5),
	m_AttackCooldown(m_MaxAttackCooldown),
	m_HasHitPlayer(false),
	m_WalkSpeed(5)
{
}
void Goomba::Initialize(const GameContext& /*gameContext*/)
{
	auto physX = PhysxManager::GetInstance()->GetPhysics();

	SetTag(L"goomba");

	m_Visuals = new GameObject();
	auto model = new ModelComponent{ L"Resources/Meshes/goomba.ovm" };
	//pGoomba->AddComponent(model);
	m_Visuals->AddComponent(model);

	auto pRigidBody = new RigidBodyComponent();
	//pRigidBody->SetKinematic(true);
	AddComponent(pRigidBody);

	// TRIGGER FOR GOOMBA STOMP
	const float width = 2.2f;
	const float height = 0.2f;
	const float depth = 2.2f;
	std::shared_ptr<physx::PxGeometry> boxGeom(new physx::PxBoxGeometry(width / 2.f, height / 2.f, depth / 2.f));
	auto defaultMaterial = physX->createMaterial(0, 0, 0);
	auto collider = new ColliderComponent(boxGeom, *defaultMaterial);
	collider->EnableTrigger(true);

	m_GoombaStompTrigger = new GameObject;
	m_GoombaStompTrigger->SetTag(L"goomba trigger");
	// Add rigidbody needed for collider
	auto pTriggerRigidBody = new RigidBodyComponent();
	m_GoombaStompTrigger->AddComponent(pTriggerRigidBody);

	m_GoombaStompTrigger->AddComponent(collider);
	AddChild(m_GoombaStompTrigger);
	m_GoombaStompTrigger->GetTransform()->Translate(m_SpawnPos);
	m_GoombaStompTrigger->SetOnTriggerCallBack(killGoomba);

	// CONVEX MESH
	//physx::PxRigidBody* const pMeshActor = physX->createRigidDynamic(physx::PxTransform::createIdentity());
	/*physx::PxConvexMesh* pConvexMesh = ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/goomba.ovpc");
	physx::PxMeshScale meshScale{ 0.1f };
	std::shared_ptr<physx::PxGeometry> pGeometry = std::make_shared<physx::PxConvexMeshGeometry>(pConvexMesh, meshScale);*/
	float boxSize{ 1.2f };
	std::shared_ptr<physx::PxGeometry> pGeometry(new physx::PxBoxGeometry(boxSize, boxSize, boxSize));

	physx::PxMaterial* const pMeshMat = physX->createMaterial(1.f, 1.f, 0.f);
	//physx::PxRigidBodyExt::updateMassAndInertia(*pMeshActor, 500);
	pRigidBody->SetDensity(500);

	ColliderComponent* meshCollider = new ColliderComponent(pGeometry, *pMeshMat);//collider component get's initialized when addchild is called
	AddComponent(meshCollider);

	// Damage Trigger
	/*physx::PxConvexMesh* pConvexMeshTrigger = ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/goomba.ovpc");
	physx::PxMeshScale meshScaleTrigger{ 0.15f };
	pGeometry = std::make_shared<physx::PxConvexMeshGeometry>(pConvexMeshTrigger, meshScaleTrigger);*/
	pGeometry = std::make_shared<physx::PxBoxGeometry>(1.5f, 1.5f, 1.5f);

	ColliderComponent* meshColliderTrigger = new ColliderComponent(pGeometry, *pMeshMat);//collider component get's initialized when addchild is called
	meshColliderTrigger->EnableTrigger(true);
	AddComponent(meshColliderTrigger);
	SetOnTriggerCallBack(hitGoomba);

	AddChild(m_Visuals);
	//AddChild(pGoomba);
	m_Visuals->GetTransform()->Translate(0, -12, 0);
	m_Visuals->GetTransform()->Rotate(0, -90, 0);

	GetTransform()->Translate(m_SpawnPos);
	const float goombaScale = 0.1f;
	GetTransform()->Scale(goombaScale, goombaScale, goombaScale);

	// Assign the material to the modelcomponent
	model->SetMaterial(2);
}
void Goomba::Update(const GameContext& gameContext)
{
	using namespace DirectX;
	const float pi = 3.1415926535f;

	XMFLOAT3 playerPos = m_pMario->GetTransform()->GetPosition();
	// set y value to the spawn y value
	playerPos.y = m_SpawnPos.y;
	XMVECTOR toPlayer = XMLoadFloat3(&playerPos) - XMLoadFloat3(&GetTransform()->GetPosition());

	float dist = *XMVector3Length(toPlayer).m128_f32;

	if (dist < m_AttackDistance && !m_HasHitPlayer)
	{
		// attack
		//std::cout << "Attack!\n";
		DirectX::XMVECTOR movement = XMVector3Normalize(toPlayer) * m_AttackMoveSpeed * gameContext.pGameTime->GetElapsed();
		GetTransform()->Translate(XMLoadFloat3(&GetTransform()->GetPosition()) + movement);

		// rotate towards player
		XMFLOAT3 movementFloat3;
		XMStoreFloat3(&movementFloat3, movement);
		float angle = atan2(GetTransform()->GetForward().z, GetTransform()->GetForward().x) - atan2(movementFloat3.z, movementFloat3.x);
		angle *= (180 / pi);
		m_Visuals->GetTransform()->Rotate(0, angle - 90, 0);
	}
	else
	{
		// move
		float sine = sin(gameContext.pGameTime->GetTotal());
		XMVECTOR targetPos{ XMLoadFloat3(&m_MoveDirection) * sine * m_MoveLength + XMLoadFloat3(&m_SpawnPos) };
		XMVECTOR toTarget{ targetPos - XMLoadFloat3(&GetTransform()->GetPosition()) };
		XMVECTOR movement = XMVector3Normalize(toTarget) * m_WalkSpeed * gameContext.pGameTime->GetElapsed();
		GetTransform()->Translate(XMLoadFloat3(&GetTransform()->GetPosition()) + movement);

		// rotate towards the move direction
		XMFLOAT3 movementFloat3;
		XMStoreFloat3(&movementFloat3, movement);
		float angle = atan2(GetTransform()->GetForward().z, GetTransform()->GetForward().x) - atan2(movementFloat3.z, movementFloat3.x);
		angle *= (180 / pi);
		m_Visuals->GetTransform()->Rotate(0, angle - 90, 0);

		if (m_HasHitPlayer)
		{
			m_AttackCooldown += gameContext.pGameTime->GetElapsed();
			if (m_AttackCooldown >= m_MaxAttackCooldown)
			{
				m_HasHitPlayer = false;
			}
		}
	}

	// position the trigger above the goomba
	XMVECTOR goombaPos = XMLoadFloat3(&GetTransform()->GetPosition());
	XMVECTOR trigger = XMLoadFloat3(&m_GoombaStompTrigger->GetTransform()->GetPosition());
	XMVECTOR heightOffset = { 0,2,0 };
	XMVECTOR diff = goombaPos + heightOffset - trigger;
	m_GoombaStompTrigger->GetTransform()->Translate(trigger + diff);
}