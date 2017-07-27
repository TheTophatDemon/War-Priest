#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>

using namespace Urho3D;

#define MAX_INSTANCE 8
class Gameplay;
class SoundSounder : public LogicComponent //Since sound sources can only play one sound at a time...
{
	URHO3D_OBJECT(SoundSounder, LogicComponent);
public:
	SoundSounder(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void Play(const String path, const bool variate = false);
	~SoundSounder();
protected:
	SharedPtr<SoundSource3D> sources[MAX_INSTANCE];
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
};

