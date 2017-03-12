#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/StaticModel.h>

using namespace Urho3D;

class Gameplay;
class Boulder : public LogicComponent
{
	URHO3D_OBJECT(Boulder, LogicComponent);
public:
	Boulder(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	void Flash();
	virtual void FixedUpdate(float timeStep);
	~Boulder();
protected:
	SharedPtr<ValueAnimation> flashAnim;
	SharedPtr<StaticModel> model;
};

