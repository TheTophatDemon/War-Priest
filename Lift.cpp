#include "Lift.h"

#include <Urho3D/Core/Context.h>

#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Bullet\BulletDynamics\Dynamics\btRigidBody.h>
#include <iostream>

#define STICK_TIME 0.05f

#define STATE_REST 0
#define STATE_MOVE 1

SharedPtr<ValueAnimation> Lift::blinkAnimation;

Lift::Lift(Context* context) : LogicComponent(context), 
	restTime(1.0f), speed(1.0f), activeRadius(0.0f),
	rotateSpeed(0.0f), wait(false), state(STATE_REST),
	stateTimer(0.0f), pointA(Vector3::ZERO), pointB(Vector3::ZERO),
	rotateDirection(1), moveDir(Vector3::ZERO), target(Vector3::ZERO),
	childCache(Vector<Pair<WeakPtr<Node>,float>>()),
	oldTransform(Matrix3x4::IDENTITY)
{
}

void Lift::RegisterObject(Context* context)
{
	context->RegisterFactory<Lift>();
	blinkAnimation = SharedPtr<ValueAnimation>(new ValueAnimation(context));
	const Color col0 = Color(0.0f, 0.5f, 0.9f, 0.3f);
	const Color col1 = Color(0.0f, 0.5f, 0.9f, 0.0f);
	blinkAnimation->SetKeyFrame(0.0f, col1);
	blinkAnimation->SetKeyFrame(0.5f, col1);
	blinkAnimation->SetKeyFrame(0.75f, col0);
	blinkAnimation->SetKeyFrame(1.0f, col1);
}

void Lift::Start()
{
	game = GetScene()->GetComponent<Gameplay>();

	oldTransform = node_->GetWorldTransform();
	
	if (!node_->HasComponent<RigidBody>())
	{
		body = node_->CreateComponent<RigidBody>();
	}
	else
	{
		body = node_->GetComponent<RigidBody>();
	}
	body->SetMass(0);
	body->SetUseGravity(false);
	body->SetCollisionLayer(3);
	body->SetCollisionEventMode(COLLISION_ALWAYS);
	body->SetFriction(0.0f);
	body->SetAngularFactor(Vector3::ZERO);
	body->SetLinearFactor(Vector3::ZERO);
	body->SetKinematic(true);

	PhysicsWorld* physworld = body->GetPhysicsWorld();

	if (wait)
	{
		glowNode = node_->CreateChild();
		StaticModel* model = node_->GetComponent<StaticModel>();
		StaticModel* glowModel = dynamic_cast<StaticModel*>(glowNode->CloneComponent(model));
		assert(glowModel);
		glowMat = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/waiting_haze.xml")->Clone();
		for (int i = 0; i < glowModel->GetNumGeometries(); ++i)
		{
			glowModel->SetMaterial(i, glowMat);
		}
		glowMat->SetShaderParameterAnimation("MatDiffColor", blinkAnimation, WM_LOOP, 1.0f);
	}

	SetTarget(pointA);

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Lift, OnCollision));
	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Lift, OnCollisionStart));
}

void Lift::FixedUpdate(float timeStep)
{
	//Movement
	stateTimer += timeStep;
	const float playerDistance = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).Length();
	if (activeRadius == 0.0f || playerDistance < activeRadius)
	{
		if (state == STATE_MOVE) 
		{
			float damping = 1.0f;
			const float targetDistance = (target - node_->GetWorldPosition()).Length();
			if (targetDistance < 1.0f) damping = Max(0.1f, targetDistance);
			node_->Translate(timeStep * moveDir * speed * damping, TS_WORLD);
			if (Abs(targetDistance) < 0.033f * speed * damping) //If it's less than a thirtieth of a second away...
			{
				node_->SetWorldPosition(target);
				ChangeState(STATE_REST);
			}
		}
	}
	if (state == STATE_REST)
	{
		//In rest mode, the "target" indicates where the platform was heading before it stopped.
		if ((!wait || target == pointB) && stateTimer > restTime)
		{
			if (target == pointA)
			{
				SetTarget(pointB);
			}
			else 
			{
				SetTarget(pointA);
			}
			ChangeState(STATE_MOVE);
		}
	}
	if (rotateSpeed != 0.0f)
	{
		node_->Rotate(Quaternion(timeStep * rotateSpeed * rotateDirection, Vector3::UP), TS_WORLD);
	}

	//All bodies standing on the lift get moved
	const Matrix3x4 deltaTransform = node_->GetWorldTransform() * oldTransform.Inverse();
	bool foundPlayer = false;
	for (Pair<WeakPtr<Node>, float>& pair : childCache)
	{
		if (pair.first_.Get())
		{
			if (pair.first_->GetName() == "player") foundPlayer = true;

			const Matrix3x4 diddly = deltaTransform * pair.first_->GetWorldTransform();
			pair.first_->SetWorldTransform(diddly.Translation(), diddly.Rotation());
			pair.first_->GetComponent<RigidBody>()->SetTransform(diddly.Translation(), diddly.Rotation());
		}
		pair.second_ -= timeStep;
		if (pair.second_ <= 0.0f)
		{
			childCache.Remove(pair);
		}
	}
	if (wait)
	{
		//The glow blinks only when nobody stands upon it
		if (foundPlayer)
		{
			glowMat->SetShaderParameterAnimationSpeed("MatDiffColor", 0.0f);
		}
		else
		{
			glowMat->SetShaderParameterAnimationSpeed("MatDiffColor", 1.0f);
		}
	}

	body->SetTransform(node_->GetWorldTransform().Translation(), node_->GetWorldTransform().Rotation());
	oldTransform = node_->GetWorldTransform();
}

void Lift::ChangeState(const int newState)
{
	if (state != newState) stateTimer = 0.0f;
	state = newState;
}

void Lift::SetTarget(Vector3& targ)
{
	target = targ;
	moveDir = targ - node_->GetWorldPosition();
	moveDir.Normalize();
}

void Lift::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData[NodeCollision::P_OTHERNODE].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData[NodeCollision::P_OTHERBODY].GetPtr();
	if (otherBody->GetCollisionLayer() & 128 || otherBody->GetCollisionLayer() & 64 || other->GetName() == "player")
	{
		VectorBuffer contacts = eventData[NodeCollision::P_CONTACTS].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			//if (normal.y_ == -666.0f) std::cout << "GOTCHAA!" << std::endl;
			if (normal.y_ < -0.1f) //When something is on top
			{
				bool found = false;
				for (Pair<WeakPtr<Node>, float>& pair : childCache)
				{
					if (pair.first_.Get() == other)
					{
						pair.second_ = STICK_TIME;
						found = true;
						break;
					}
				}
				if (!found) childCache.Push(Pair<WeakPtr<Node>, float>(WeakPtr<Node>(other), STICK_TIME)); 
				break;
			}
		}
	}
}

void Lift::OnCollisionStart(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 128 || otherBody->GetCollisionLayer() & 64 || other->GetName() == "player") //KNOCK KNOCK ITS ME
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			//std::cout << normal.ToString().CString() << std::endl;
			//normal y is -1 when on top
			if (normal.y_ >= -0.1f)
			{
				if (rotateSpeed != 0.0f) rotateDirection = -rotateDirection;
				if (state == STATE_MOVE) 
				{
					Vector3 contactDir = position - node_->GetWorldPosition();
					contactDir.Normalize();
					const float facing = contactDir.DotProduct(moveDir);
					if (facing > 0.0f) 
					{
						if (target == pointA)
						{
							SetTarget(pointB);
						}
						else
						{
							SetTarget(pointA);
						}
					}
				}
				return;
			}
			else if (other->GetName() == "player" //The player should be on top in this case
				&& wait && state == STATE_REST && target == pointA)
			{
				SetTarget(pointB);
				ChangeState(STATE_MOVE);
			}
		}
	}
}

Lift::~Lift()
{
}
