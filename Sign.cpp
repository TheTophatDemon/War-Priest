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
				newMessage += Settings::GetForwardKey()->name;
			else if (token == "%BKEY")
				newMessage += Settings::GetBackwardKey()->name;
			else if (token == "%LKEY")
				newMessage += Settings::GetLeftKey()->name;
			else if (token == "%RKEY")
				newMessage += Settings::GetRightKey()->name;
			else if (token == "%JKEY")
				newMessage += Settings::GetJumpKey()->name;
			else if (token == "%RVKEY")
				newMessage += Settings::GetReviveKey()->name;
			else if (token == "%SKEY")
				newMessage += Settings::GetSlideKey()->name;
		}
		else
		{
			newMessage += token + " ";
		}
	}
	return newMessage;
}

Sign::~Sign() {}
