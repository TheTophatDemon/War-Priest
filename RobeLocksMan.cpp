#include "RobeLocksMan.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <iostream>

#include "RobeLocksMissile.h"
#include "RobeLocksBomb.h"
#include "Actor.h"

#define STATE_DEAD 0 
#define STATE_WANDER 1
#define STATE_SWITCH 32
#define STATE_ATTACK 33

#define REVIVE_ANIM "Models/enemy/robelocksman_revive.ani"
#define IDLE_ANIM "Models/enemy/robelocksman_idle.ani"
#define WALK_ANIM "Models/enemy/robelocksman_walk.ani"
#define JUMP_ANIM "Models/enemy/robelocksman_jump.ani"
#define IDLE_ANIM_HOLDING "Models/enemy/robelocksman_idle_holding.ani"
#define WALK_ANIM_HOLDING "Models/enemy/robelocksman_walk_holding.ani"
#define JUMP_ANIM_HOLDING "Models/enemy/robelocksman_jump_holding.ani"

RobeLocksMan::RobeLocksMan(Context* context) : Enemy(context),
	currentWeapon(Weapon::Launcher)
{
}

void RobeLocksMan::RegisterObject(Context* context)
{
	context->RegisterFactory<RobeLocksMan>();
}

void RobeLocksMan::DelayedStart()
{
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(JUMP_ANIM);
	cache->GetResource<Animation>(IDLE_ANIM_HOLDING);
	cache->GetResource<Animation>(WALK_ANIM_HOLDING);
	cache->GetResource<Animation>(JUMP_ANIM_HOLDING);

	soundSource = node_->CreateComponent<SoundSounder>();
	weaponNode = modelNode->GetChild("weapon", true);
	weaponModel = weaponNode->GetComponent<StaticModel>();
	weaponNode->SetEnabled(false);

	Enemy::DelayedStart();
}

void RobeLocksMan::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

void RobeLocksMan::Execute()
{
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		stateTimer += deltaTime;

		if (stateTimer > 2.0f)
		{
			ChangeState(STATE_SWITCH);
		}

		Wander(false, true);
		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;
	case STATE_SWITCH:
		stateTimer += deltaTime;

		if (stateTimer > 1.0f)
		{
			currentWeapon = (Weapon) Random(0, 2);
			//currentWeapon = Weapon::Bomb;
			ChangeState(STATE_ATTACK);
		}

		animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		actor->SetInputFPS(false, false, false, false);
		actor->Move(deltaTime);
		break;
	case STATE_ATTACK:

		switch (currentWeapon)
		{
		case Weapon::Launcher:
			Wander(false, true);
			if (walking)
				animController->PlayExclusive(WALK_ANIM_HOLDING, 0, true, 0.2f);
			else
				animController->PlayExclusive(IDLE_ANIM_HOLDING, 0, true, 0.2f);
			break;
		case Weapon::Bomb:
			FaceTarget();
			if (shootTimer < 0.5f && weaponNode->IsEnabled() == false) 
			{
				weaponNode->SetEnabled(true);
			}
			animController->PlayExclusive(IDLE_ANIM_HOLDING, 0, true, 0.2f);
			actor->SetInputFPS(false, false, false, false);
			actor->Move(deltaTime);
			break;
		}

		shootTimer -= deltaTime;
		if (shootTimer < 0.0f)
		{
			if (Random(1.0f) > 0.5f) ChangeState(STATE_SWITCH);
			switch (currentWeapon)
			{
			case Weapon::Launcher:
				shootTimer = Random(1.0f, 5.0f);
				RobeLocksMissile::MakeRobeLocksMissile(scene, weaponNode->GetWorldPosition() + node_->GetWorldRotation() * Vector3::FORWARD * 0.4f, node_->GetWorldRotation(), node_);
				soundSource->Play("Sounds/enm_robelocksmissile.wav", true);
				break;
			case Weapon::Bomb:
			{
				shootTimer = Random(1.0f, 5.0f);
				const Vector3 aim = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
				Node* bomb = RobeLocksBomb::MakeRobeLocksBomb(scene, weaponNode->GetWorldPosition());
				RigidBody* bBody = bomb->GetComponent<RigidBody>();
				bBody->ApplyImpulse(aim * bBody->GetMass() * 10.0f);
				bBody->ApplyImpulse(Vector3::UP * bBody->GetMass() * 10.0f);
				weaponNode->SetEnabled(false);
				break;
			}
			}
		}
		
		break;
	}
}

void RobeLocksMan::Dead()
{
	Enemy::Dead();
	if (animController->IsAtEnd(REVIVE_ANIM))
	{
		ChangeState(STATE_WANDER);
	}
}

void RobeLocksMan::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	switch (newState)
	{
	case STATE_DEAD:
		animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
		animController->SetSpeed(REVIVE_ANIM, 0.0f);
		break;
	case STATE_WANDER:
		weaponNode->SetEnabled(false);
		break;
	case STATE_ATTACK:
		shootTimer = Random(1.0f, 5.0f);
		weaponNode->SetEnabled(true);
		switch (currentWeapon)
		{
		case Weapon::Launcher:
			weaponModel->SetModel(cache->GetResource<Model>("Models/robelocks_launcher.mdl"));
			weaponModel->SetMaterial(cache->GetResource<Material>("Materials/skins/robelocks_launcher_skin.xml"));
			break;
		case Weapon::Bomb:
			weaponModel->SetModel(cache->GetResource<Model>("Models/robelocks_bomb.mdl"));
			weaponModel->SetMaterial(cache->GetResource<Material>("Materials/skins/robelocks_bomb_skin.xml"));
			break;
		}
		break;
	}
}

void RobeLocksMan::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
}

RobeLocksMan::~RobeLocksMan()
{
}
