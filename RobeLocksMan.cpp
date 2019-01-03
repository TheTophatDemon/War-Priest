#include "RobeLocksMan.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Resource/XMLFile.h>
#include <iostream>

#include "RobeLocksMissile.h"
#include "RobeLocksBomb.h"
#include "Actor.h"
#include "Fireball.h"

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

#define ATTACK_THRESHOLD 30

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
			if (distanceFromPlayer < ATTACK_THRESHOLD)
			{
				currentWeapon = (Weapon)Random(0, 3);
			}
			else
			{
				currentWeapon = Weapon::Launcher;
			}
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
			if (shootTimer > 0.5f || distanceFromPlayer > ATTACK_THRESHOLD)
			{
				Wander(false, true);
				if (walking)
				{
					animController->PlayExclusive(WALK_ANIM_HOLDING, 0, true, 0.2f);
				}
				else
				{
					animController->PlayExclusive(IDLE_ANIM_HOLDING, 0, true, 0.2f);
				}
			}
			else
			{
				FaceTarget();
				animController->PlayExclusive(IDLE_ANIM_HOLDING, 0, true, 0.2f);
				actor->SetInputFPS(false, false, false, false);
				actor->Move(deltaTime);
			}
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
		case Weapon::PaintGun:
			FaceTarget();
			if (distanceFromPlayer > 15.0f)
			{
				actor->SetInputFPS(true, false, false, false);
				animController->PlayExclusive(WALK_ANIM_HOLDING, 0, true, 0.2f);
			}
			else
			{
				actor->SetInputFPS(false, false, false, false);
				animController->PlayExclusive(IDLE_ANIM_HOLDING, 0, true, 0.2f);
			}
			actor->Move(deltaTime);
			break;
		}

		shootTimer -= deltaTime;
		if (shootTimer < 0.0f)
		{
			switch (currentWeapon)
			{
			case Weapon::Launcher:
				if (distanceFromPlayer < ATTACK_THRESHOLD)
				{ 
					shootTimer = Random(0.5f, 1.0f); 
				}
				else
				{
					shootTimer = Random(1.0f, 5.0f);
				}
				RobeLocksMissile::MakeRobeLocksMissile(scene, weaponNode->GetWorldPosition() + node_->GetWorldRotation() * Vector3::FORWARD * 0.4f, node_->GetWorldRotation(), node_);
				soundSource->Play("Sounds/enm_robelocksmissile.wav", true);
				if (Random(1.0f) > 0.5f) ChangeState(STATE_SWITCH);
				break;
			case Weapon::Bomb:
			{
				shootTimer = Random(0.5f, 1.0f);
				const Vector3 aim = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
				Node* bomb = RobeLocksBomb::MakeRobeLocksBomb(scene, weaponNode->GetWorldPosition());
				RigidBody* bBody = bomb->GetComponent<RigidBody>();
				bBody->ApplyImpulse(aim * bBody->GetMass() * 10.0f);
				bBody->ApplyImpulse(Vector3::UP * bBody->GetMass() * 10.0f);
				weaponNode->SetEnabled(false);
				if (Random(1.0f) > 0.5f) ChangeState(STATE_SWITCH);
				break;
			}
			case Weapon::PaintGun:
				shootTimer = 0.25f;
				
				const Vector3 aim = (target->GetWorldPosition() + Vector3::UP * 2.0f - weaponNode->GetWorldPosition()).Normalized();

				//Raycast to determine whether the player is in sight
				PhysicsRaycastResult result;
				Ray aimRay = Ray(weaponNode->GetWorldPosition(), aim);
				physworld->RaycastSingle(result, aimRay, 100.0f, 130U); //2+128
				if (result.body_)
				{
					if (result.body_->GetCollisionLayer() & 128)
					{
						//Raycast to determine where the bullet is supposed to hit the map
						physworld->RaycastSingle(result, aimRay, 100.0f, 2U);
						//Tie the distance traveled to the bullet's life-span to ensure that it does not phase through walls
						float lifeSpan = 2.0f;
						if (result.body_)
						{
							lifeSpan = (result.distance_ / 50.0f) - 0.1f;
						}

						Quaternion aimQuat;
						aimQuat.FromLookRotation(aim);
						Fireball::MakePaintball(scene, weaponNode->GetWorldPosition() + node_->GetWorldRotation() * Vector3::FORWARD, aimQuat, node_, lifeSpan);
					}
				}
				
				if (Random(1.0f) > 0.9f) ChangeState(STATE_SWITCH);
				break;
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
		weaponNode->SetEnabled(true);
		switch (currentWeapon)
		{
		case Weapon::Launcher:
			weaponModel->SetModel(cache->GetResource<Model>("Models/robelocks_launcher.mdl"));
			weaponModel->SetMaterial(cache->GetResource<Material>("Materials/skins/robelocks_launcher_skin.xml"));
			shootTimer = Random(0.5f, 1.0f);
			break;
		case Weapon::Bomb:
			weaponModel->SetModel(cache->GetResource<Model>("Models/robelocks_bomb.mdl"));
			weaponModel->SetMaterial(cache->GetResource<Material>("Materials/skins/robelocks_bomb_skin.xml"));
			shootTimer = Random(0.5f, 1.0f);
			break;
		case Weapon::PaintGun:
			weaponModel->SetModel(cache->GetResource<Model>("Models/robelocks_paint_gun.mdl"));
			weaponModel->SetMaterial(cache->GetResource<Material>("Materials/skins/robelocks_paint_gun_skin.xml"));
			shootTimer = 0.25f;
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
