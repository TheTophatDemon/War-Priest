#include "SoundSounder.h"

#include "Urho3D/Core/Context.h"
#include "Gameplay.h"
#include "GunPriest.h"
#include "Settings.h"
#include <iostream>

SoundSounder::SoundSounder(Context* context) : LogicComponent(context)
{
	
}

void SoundSounder::RegisterObject(Context* context)
{
	context->RegisterFactory<SoundSounder>();
}

void SoundSounder::Start()
{
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		sources[i] = node_->CreateComponent<SoundSource3D>();
		sources[i]->SetSoundType("GAMEPLAY");
	}
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
}

void SoundSounder::FixedUpdate(float timeStep)
{
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		sources[i]->SetGain(Settings::GetSoundVolume());
	}
}

void SoundSounder::Play(const String path, const bool variate)
{
	float freq = 44100.0f;
	if (variate) freq += Random(-1500.0f, 1500.0f);
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		if (sources[i]->IsPlaying() == false)
		{
			sources[i]->Play(cache->GetResource<Sound>(path), freq);
			return;
		}
	}
	sources[0]->Play(cache->GetResource<Sound>(path), freq);
}

void SoundSounder::SetDistanceAttenuation(const float nearDist, const float farDist, const float rollOff)
{
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		sources[i]->SetDistanceAttenuation(nearDist, farDist, rollOff);
	}
}

SoundSounder::~SoundSounder()
{
}
