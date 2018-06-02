#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Bonus : public LogicComponent
{
	URHO3D_OBJECT(Bonus, LogicComponent);
public:
	Bonus(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	~Bonus();
};

