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
				newMessage += Settings::GetAction(ActionType::FORWARD).binding->verboseName;
			else if (token == "%BKEY")
				newMessage += Settings::GetAction(ActionType::BACK).binding->verboseName;
			else if (token == "%LKEY")
				newMessage += Settings::GetAction(ActionType::LEFT).binding->verboseName;
			else if (token == "%RKEY")
				newMessage += Settings::GetAction(ActionType::RIGHT).binding->verboseName;
			else if (token == "%JKEY")
				newMessage += Settings::GetAction(ActionType::JUMP).binding->verboseName;
			else if (token == "%RVKEY")
				newMessage += Settings::GetAction(ActionType::REVIVE).binding->verboseName;
			else if (token == "%SKEY")
				newMessage += Settings::GetAction(ActionType::SLIDE).binding->verboseName;
		}
		else
		{
			newMessage += token + " ";
		}
	}
	return newMessage;
}

Sign::~Sign() {}
