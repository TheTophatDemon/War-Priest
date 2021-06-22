#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

class Gameplay;
namespace GP 
{
	class Sign : public LogicComponent
	{
		URHO3D_OBJECT(Sign, LogicComponent);
	public:
		Sign(Context* context);
		static void RegisterObject(Context* context);
		virtual void Start() override;
		virtual void FixedUpdate(float timeStep) override;
		~Sign();
	protected:
		String ParseMessage(const String& message);
		WeakPtr<Gameplay> game;
		Input* input;
	};
}

