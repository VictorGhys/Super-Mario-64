#pragma once
#include <GameObject.h>
class Goomba : public GameObject
{
public:
	Goomba(DirectX::XMFLOAT3 spawnPos, GameObject* mario, DirectX::XMFLOAT3 moveDirection = { 1,0,0 });
	virtual ~Goomba() = default;

	Goomba(const Goomba& other) = delete;
	Goomba(Goomba&& other) noexcept = delete;
	Goomba& operator=(const Goomba& other) = delete;
	Goomba& operator=(Goomba&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	void SetHasHitPlayer(bool hasHit) { m_HasHitPlayer = hasHit; }
	void ResetAttackCooldown() { m_AttackCooldown = 0; }
private:
	GameObject* m_GoombaStompTrigger;
	GameObject* m_Visuals;
	DirectX::XMFLOAT3 m_SpawnPos;
	DirectX::XMFLOAT3 m_MoveDirection;
	float m_MoveLength;
	float m_PrevSine;
	bool m_IsAttacking;
	GameObject* m_pMario;
	float m_AttackDistance;
	float m_AttackMoveSpeed;
	float m_MaxAttackCooldown;
	float m_AttackCooldown;
	bool m_HasHitPlayer;
	float m_WalkSpeed;
};
