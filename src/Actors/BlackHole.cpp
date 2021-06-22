/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "BlackHole.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include <Urho3D/Resource/XMLFile.h>

#include "../Gameplay.h"

BlackHole::BlackHole(Context* context) : LogicComponent(context)
{
}

void BlackHole::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	pass = node_->GetComponent<StaticModel>()->GetMaterial(0)->GetTechnique(0)->GetPass("postalpha");

	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(BlackHole, OnCollision));
}

void BlackHole::FixedUpdate(float timeStep)
{
	//Hack to make it so that it still looks black-holey when the camera is inside of the black hole model
	const float camDist = (node_->GetWorldPosition() - game->cameraNode->GetWorldPosition()).Length();
	if (camDist < node_->GetScale().x_ * 0.75f)
	{
		pass->SetDepthTestMode(CompareMode::CMP_ALWAYS);
	}
	else
	{
		pass->SetDepthTestMode(CompareMode::CMP_LESSEQUAL);
	}
}

void BlackHole::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != owner.Get() && !other->HasTag("handles_forcefields_manually") && otherBody->GetMass() > 0)
	{
		//All rigid bodies that aren't likely to have code for this interaction are pushed towards the center
		const Vector3 diff = node_->GetWorldPosition() - other->GetWorldPosition();
		otherBody->ApplyImpulse(diff.Normalized() * 2.0f * otherBody->GetMass());
	}
}

Node* BlackHole::MakeBlackHole(Scene* scene, const Vector3 position, Node* owner)
{
	Node* n = scene->InstantiateXML(scene->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/blackhole.xml")->GetRoot(),
		position, Quaternion::IDENTITY, LOCAL);
	BlackHole* bh = new BlackHole(scene->GetContext());
	bh->owner = owner;
	n->AddComponent(bh, 123, LOCAL);
	return n;
}

void BlackHole::RegisterObject(Context* context)
{
	context->RegisterFactory<BlackHole>();
}


BlackHole::~BlackHole()
{
}
