#include "KilledKaaba.h"

#include "Gameplay.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include "Settings.h"
#include "Blackstone.h"

#define STATE_DORMANT 0
#define STATE_RISE 1
#define STATE_FLY 2
#define STATE_BALLATTACK 3

#define HEIGHT_FROM_BOTTOM 9.0f
#define RISE_SPEED 500.0f

KilledKaaba::KilledKaaba(Context* context) : LogicComponent(context),
	distanceFromPlayer(0.0f),
	stateTimer(0.0f),
	state(-1),
	moveSpeed(1.0f),
	direction(1.0f, 0.0f, 0.0f),
	targetHeight(0.0f),
	lastState(-1),
	attackTimer(0.0f),
	shootTimer(0.0f)
{
}

void KilledKaaba::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	moveSpeed = 50.0f * Settings::GetDifficulty();
}

void KilledKaaba::Start()
{
	moveSpeed = 50.0f * Settings::GetDifficulty();

	SetUpdateEventMask(USE_FIXEDUPDATE);
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	
	body = node_->GetComponent<RigidBody>();
	body->SetLinearVelocity(Vector3::ZERO);

	targetHeight = node_->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;

	glowNode = node_->CreateChild();
	StaticModel* sm = glowNode->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/fireglow.xml"));

	areas = Vector<SharedPtr<Node>>();
	PODVector<Node*> areas_ptrs = scene->GetChildrenWithTag("kaaba_area", true);
	for (Node* n : areas_ptrs)
	{
		n->RemoveComponent<StaticModel>();
		areas.Push(SharedPtr<Node>(n));
		SubscribeToEvent(n, E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnAreaCollision));
	}

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(KilledKaaba, OnSettingsChange));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnCollision));

	ChangeState(STATE_DORMANT);
}

void KilledKaaba::FixedUpdate(float timeStep)
{
	Vector3 plyPos = game->playerNode->GetWorldPosition(); 
	plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); 
	ourPos.y_ = 0.0f;
	distanceFromPlayer = (ourPos - plyPos).Length();

	const float hDiff = targetHeight - node_->GetWorldPosition().y_;
	const float currentSpeed = body->GetLinearVelocity().Length();
	const int spinDir = Sign(body->GetAngularVelocity().y_);

	stateTimer += timeStep;
	switch (state)
	{
	case STATE_DORMANT:
		body->SetLinearVelocity(Vector3::ZERO);
		if (distanceFromPlayer < 40.0f)
		{
			ChangeState(STATE_RISE);
		}
		break;
	case STATE_RISE:
		if (Abs(hDiff) < 1.0f) //Decelerate the spinning before resuming activity
		{
			body->SetLinearVelocity(Vector3::ZERO);
			body->ApplyTorqueImpulse(Vector3(0.0f, -spinDir * timeStep * 500.0f, 0.0f));
			if (body->GetAngularVelocity().Length() < 1.0f)
			{
				body->SetAngularVelocity(Vector3::ZERO);
				ChangeState(STATE_FLY);
			}
		}
		else
		{
			if (lastState == STATE_DORMANT)
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff) * 0.1f, 0.0f));
			else
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff), 0.0f));
			body->ApplyTorqueImpulse(Vector3(0.0f, 500.0f * timeStep, 0.0f));
		}
		break;
	case STATE_FLY:
		node_->Rotate(Quaternion(timeStep * 100.0f, Vector3::UP), TS_LOCAL);
		if (currentSpeed < moveSpeed) 
		{
			body->ApplyForce(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
		}
		else if (currentSpeed > moveSpeed)
		{
			body->SetLinearVelocity(body->GetLinearVelocity().Normalized() * moveSpeed);
		}

		attackTimer -= timeStep;
		if (attackTimer < 0.0f)
		{
			const int attack = Random(0, 5);
			switch (attack)
			{
			default:
			{
				int count = floorf(Settings::ScaleWithDifficulty(8.0f, 12.0f, 16.0));
				for (int i = 0; i < count; ++i)
				{
					float degree = (360.0f / count) * i;
					BouncyFireball::MakeBouncyFireball(scene, node_->GetWorldPosition() + Vector3(0.0f, -4.0f, 0.0f), Quaternion(degree, Vector3::UP), node_);
				}
				attackTimer = Random(Settings::ScaleWithDifficulty(8.0f, 5.0f, 3.0f), 10.0f);
				break;
			}
			}
			//ChangeState(STATE_BALLATTACK);
		}
		
		if (hDiff < -1.0f) 
		{
			const int buffer = body->GetCollisionLayer();
			body->SetCollisionLayer(0);

			PhysicsRaycastResult result;
			physworld->SphereCast(result, Ray(node_->GetWorldPosition(), Vector3::DOWN), 8.0f, 10.0f, 2U);
			if (result.body_ == nullptr || result.body_ == body) ChangeState(STATE_RISE);

			body->SetCollisionLayer(buffer);
		}
		else if (hDiff > 1.0f)
		{
			ChangeState(STATE_RISE);
		}
		break;
	case STATE_BALLATTACK:
		body->SetLinearVelocity(Vector3::ZERO);
		if (stateTimer < 4.0f) 
		{
			body->ApplyTorqueImpulse(Vector3(0.0f, timeStep * 500.0f, 0.0f));
		}
		else //Decelerate spinning before going back to normal
		{
			body->ApplyTorqueImpulse(Vector3(0.0f, -spinDir * timeStep * 500.0f, 0.0f));
			if (body->GetAngularVelocity().Length() < 1.0f)
			{
				body->SetAngularVelocity(Vector3::ZERO);
				ChangeState(STATE_FLY);
			}
		}
		shootTimer += timeStep;
		if (shootTimer > Settings::ScaleWithDifficulty(1.0f, 0.5f, 0.5f))
		{
			shootTimer = 0.0f;
			Vector3 shootDirection = Vector3(0.0f, 16.0f, 0.0f);
			shootDirection.x_ = Random(100.0f, 500.0f) * Sign(Random(-1.0f, 1.0f));
			shootDirection.z_ = Random(100.0f, 500.0f) * Sign(Random(-1.0f, 1.0f));
			Blackstone::MakeBlackstone(scene, node_->GetWorldPosition() + Vector3(0.0f, 12.0f, 0.0f), shootDirection, node_);
		}
		break;
	}
}

void KilledKaaba::OnAreaCollision(StringHash eventType, VariantMap& eventData)
{
	RigidBody* body = (RigidBody*)eventData["Body"].GetPtr();
	Node* area = body->GetNode();
	Node* otherNode = (Node*)eventData["OtherNode"].GetPtr();
	if (otherNode->GetName() == "player")
	{
		for (SharedPtr<Node> a : areas)
		{
			if (a.Get() == area)
			{
				targetHeight = a->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;
				break;
			}
		}
	}
}

void KilledKaaba::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 2 || 
		(otherBody->IsTrigger() && other->HasTag("enemy_barrier")) )
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof()) 
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			direction += normal * -Sign(distance);
		}
		direction.y_ = 0.0f;
		direction.Normalize();
		body->ApplyImpulse(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
	}
}

void KilledKaaba::ChangeState(const int newState)
{
	if (newState != state)
	{
		LeaveState(state);
		EnterState(newState);
		lastState = state;
		stateTimer = 0.0f;
	}
	state = newState;
}

void KilledKaaba::EnterState(const int newState)
{
	switch (newState)
	{
		case STATE_DORMANT:
			glowNode->SetEnabled(false);
			glowNode->SetScale(0.0f);
			break;
		case STATE_FLY:
			direction = Vector3(Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f));
			direction.Normalize();
			attackTimer = Random(Settings::ScaleWithDifficulty(8.0f, 5.0f, 3.0f), 10.0f);
			break;
		case STATE_RISE:
			body->SetAngularFactor(Vector3::UP);
			body->SetLinearFactor(Vector3::ONE);
			break;
		case STATE_BALLATTACK:
			body->SetAngularFactor(Vector3::UP);
			body->SetLinearFactor(Vector3::ONE);
			shootTimer = 0.0f;
			break;
	}
}

void KilledKaaba::LeaveState(const int oldState)
{
	switch(oldState) 
	{
	case STATE_DORMANT:
	{
		glowNode->SetEnabled(true);
		glowNode->SetScale(8.25f);

		SharedPtr<ValueAnimation> valAnim(new ValueAnimation(context_));
		valAnim->SetKeyFrame(0.0f, Vector3(8.5f, 8.5f, 8.5f));
		valAnim->SetKeyFrame(0.5f, Vector3(9.0f, 9.0f, 9.0f));
		valAnim->SetKeyFrame(1.0f, Vector3(8.5f, 8.5f, 8.5f));

		glowNode->SetAttributeAnimation("Scale", valAnim, WM_LOOP, 1.0f);

		game->FlashScreen(Color::WHITE, 0.01f);
		break;
	}
	case STATE_RISE:
		body->SetAngularFactor(Vector3::ZERO);
		body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
		break;
	case STATE_BALLATTACK:
		body->SetAngularFactor(Vector3::ZERO);
		body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
		break;
	}
}

KilledKaaba::~KilledKaaba()
{
}

void KilledKaaba::RegisterObject(Context* context)
{
	context->RegisterFactory<KilledKaaba>();
}