#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class MusicPlayer : public LogicComponent
{
	URHO3D_OBJECT(MusicPlayer, LogicComponent);
public:
	MusicPlayer(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~MusicPlayer();
protected:
	SharedPtr<SoundSource> musicSource;
};

