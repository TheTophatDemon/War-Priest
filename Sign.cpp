#include "Sign.h"

#include <Urho3D/Core/Context.h>

#include "Gameplay.h"
#include "Settings.h"

#define READ_DISTANCE 8 * 8

using namespace GP;

Sign::Sign(Context* context) : LogicComponent(context){}

void Sign::RegisterObject(Context* context)
{
	context->RegisterFactory<Sign>();
}

void Sign::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	input = GetSubsystem<Input>();
}

void Sign::FixedUpdate(float timeStep)
{
	const float distSquared = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).LengthSquared();
	if (distSquared < READ_DISTANCE)
	{
		game->DisplayMessage(ParseMessage(node_->GetVar("message").GetString()), Color::WHITE, 0.5f, 5);
	}
}

String Sign::ParseMessage(const String& message)
{
	String newMessage = "";
	StringVector tokens = message.Split(' ', true);
	for (String& token : tokens)
	{
		if (token.StartsWith("%"))
		{
			if (token == "%FKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetForwardKey());
			else if (token == "%BKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetBackwardKey());
			else if (token == "%LKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetLeftKey());
			else if (token == "%RKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetRightKey());
			else if (token == "%JKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetJumpKey());
			else if (token == "%RKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetReviveKey());
			else if (token == "%SKEY")
				newMessage += Settings::GetKeyName(input, Settings::GetSlideKey());
		}
		else
		{
			newMessage += token + " ";
		}
	}
	return newMessage;
}

Sign::~Sign() {}
