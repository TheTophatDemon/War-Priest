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

#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
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
	void Play(const String path, bool variate = false);
	void SetDistanceAttenuation(const float nearDist, const float farDist, float rollOff = 2.0f);
	void StopPlaying();
	~SoundSounder();
protected:
	SharedPtr<SoundSource3D> sources[MAX_INSTANCE];
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
};

