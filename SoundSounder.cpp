#include "SoundSounder.h"

#include "Urho3D/Core/Context.h"
#include "Gameplay.h"
#include "GunPriest.h"
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
		sources[i]->SetSoundType("ALL");
	}
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
}

void SoundSounder::FixedUpdate(float timeStep)
{
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		sources[i]->SetGain(game->gunPriest->sSoundVolume);
	}
}

void SoundSounder::Play(String path)
{
	for (int i = 0; i < MAX_INSTANCE; ++i)
	{
		if (sources[i]->IsPlaying() == false)
		{
			sources[i]->Play(cache->GetResource<Sound>(path));
			return;
		}
	}
	sources[0]->Play(cache->GetResource<Sound>(path));
}

SoundSounder::~SoundSounder()
{
}
