#include "ChaosCaliph.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Actor.h"
#include "Fireball.h"
#include "Projectile.h"
#include "WeakChild.h"
#include "Settings.h"

#define SPIN_RANGE 7.0f

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SHOOT 32
#define STATE_SPIN 33

#define IDLE_ANIM "Models/enemy/chaoscaliph_idle.ani"
#define REVIVE_ANIM "Models/enemy/chaoscaliph_revive.ani"
#define WALK_ANIM "Models/enemy/chaoscaliph_walk.ani"
#define SHOOT_ANIM "Models/enemy/chaoscaliph_shoot.ani"
#define SPIN_ANIM "Models/enemy/chaoscaliph_spin.ani"

#define SOUND_LASER "Sounds/enm_laser.wav"
#define SOUND_SHOCK "Sounds/enm_shock.wav"

ChaosCaliph::ChaosCaliph(Context* context) : Enemy(context), 
	shot(false),
	lastState(STATE_DEAD)
{
}

void ChaosCaliph::RegisterObject(Context* context)
{
	context->RegisterFactory<ChaosCaliph>();
}

void ChaosCaliph::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	actor->maxspeed = 15.5f + Settings::ScaleWithDifficulty(-1.0f, 0.0f, 1.5f);
}

void ChaosCaliph::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 15.5f + Settings::ScaleWithDifficulty(-1.0f, 0.0f, 1.5f);
	actor->acceleration = 50.0f;
	actor->friction = 0.5f;
	actor->maxfall = 15.0f;

	sparkChild = node_->GetChild("spark");
	sparkChild->SetParent(scene);
	WeakChild::MakeWeakChild(sparkChild, node_);

	modelNode->SetParent(scene);
	WeakChild::MakeWeakChild(modelNode, node_);

	emitter = sparkChild->GetComponent<ParticleEmitter>();
	emitter->SetEmitting(false);

	animModel = modelNode->GetComponent<AnimatedModel>();
	boringMaterial = animModel->GetMaterial();
	glowyMaterial = cache->GetResource<Material>("Materials/enemy/chaoscaliph_skin_glowy.xml");

	SharedPtr<ValueAnimation> glowyThrob(new ValueAnimation(context_));
	glowyThrob->SetKeyFrame(0.0f, Color(0.0f, 0.0f, 0.0f));
	glowyThrob->SetKeyFrame(0.5f, Color(0.0f, 0.0f, 1.0f));
	glowyThrob->SetKeyFrame(1.0f, Color(0.0f, 0.0f, 0.0f));
	glowyMaterial->SetShaderParameterAnimation("MatEmissiveColor", glowyThrob, WM_LOOP, 1.0f);

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(ChaosCaliph, OnSettingsChange));
}

void ChaosCaliph::FixedUpdate(float timeStep)
{
	modelNode->SetWorldPosition(node_->GetWorldPosition());
	modelNode->SetWorldRotation(node_->GetWorldRotation());
	modelNode->Rotate(Quaternion(-90.0f, Vector3::UP));

	Enemy::FixedUpdate(timeStep);
}

void ChaosCaliph::Execute()
{
	Vector3 aimVec = Vector3::ZERO;
	float targetDistance;
	if (target.Get())
	{
		const Vector3 tarDiff = (target->GetWorldPosition() - node_->GetWorldPosition());
		targetDistance = tarDiff.Length();
		aimVec = tarDiff.Normalized();
	}

	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		stateTimer += deltaTime;

		Wander(false, false);
		
		if (targetDistance < SPIN_RANGE && lastState != STATE_DEAD)
			ChangeState(STATE_SPIN);

		if (stateTimer > 1.0f)
		{
			//Check if player is within aim
			PhysicsRaycastResult result;
			physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aimVec), 60.0f, 130);//128+2
			if (result.body_)
			{
				if (result.body_->GetCollisionLayer() & 128)
				{
					ChangeState(STATE_SHOOT);
				}
			}
			stateTimer = 0.0f;
		}

		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);

		break;
	case STATE_SHOOT:
		FaceTarget();
		
		animController->PlayExclusive(SHOOT_ANIM, 0, false, 0.2f);

		stateTimer += deltaTime;
		if (stateTimer > 0.25f)
		{
			if (!shot)
			{
				shot = true;
				for (int i = -1; i <= 1; ++i)
				{
					if (i == 0 && Settings::GetDifficulty() < Settings::UNHOLY_THRESHOLD) continue;
					Fireball::MakeBlueFireball(scene, 
						node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f) + (node_->GetWorldDirection() * 2.0f),
						Quaternion((i*30.0f) + node_->GetWorldRotation().EulerAngles().y_, Vector3::UP),
						node_);
					soundSource->Play(SOUND_LASER, true);
				}
			}
			else
			{
				if (stateTimer > 0.5f) ChangeState(STATE_WANDER);
			}
		}
		
		actor->SetMovement(0.0f, 0.0f);
		actor->Move(deltaTime);
		break;
	case STATE_SPIN:
		stateTimer += deltaTime;

		animController->PlayExclusive(SPIN_ANIM, 0, true, 0.2f);

		if (target.Get()) 
		{
			if (targetDistance > SPIN_RANGE)
			{
				ChangeState(STATE_WANDER);
			}
		} 
		
		modelNode->SetWorldRotation(Quaternion(stateTimer * 1080.0f, Vector3::UP));
		newRotation.FromLookRotation(Vector3(aimVec.x_, 0.0f, aimVec.z_), Vector3::UP);
		//FaceTarget();

		if (!CheckCliff(false))
		{
			actor->SetMovement(true, false, false, false);
		}
		else 
		{
			actor->SetMovement(false, false, false, false);
		}
		actor->Move(deltaTime);
		break;
	}

	if (sparkChild.Get()) sparkChild->SetWorldPosition(node_->GetWorldPosition());
}

void ChaosCaliph::EnterState(const int newState)
{
	lastState = state;
	Enemy::EnterState(newState);
	if (newState == STATE_SHOOT)
	{
		shot = false;
	}
	else if (newState == STATE_SPIN)
	{
		animModel->SetMaterial(glowyMaterial);
		emitter->SetEmitting(true);
		spinSpeed = 0.0f;
		actor->forward = 0.0f;
		actor->acceleration = 50.0f;
		soundSource->Play(SOUND_SHOCK, true);
	}
}

void ChaosCaliph::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_SPIN)
	{
		animModel->SetMaterial(boringMaterial);
		emitter->SetEmitting(false);
		actor->acceleration = 50.0f;
		soundSource->StopPlaying();
	}
}

void ChaosCaliph::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Enemy::OnCollision(eventType, eventData);
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	if ((other == target.Get() || other->HasTag("statue")) && state == STATE_SPIN) 
	{
		VariantMap map = VariantMap();
		map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, node_));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, Variant(other)));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, 12));
		SendEvent(Projectile::E_PROJECTILEHIT, map);
	}
}

void ChaosCaliph::Dead()
{
	Enemy::Dead();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
	if (animController->GetTime(REVIVE_ANIM) >= animController->GetLength(REVIVE_ANIM) * 0.9f)
	{
		ChangeState(STATE_WANDER);
	}
}

void ChaosCaliph::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

ChaosCaliph::~ChaosCaliph()
{
}
