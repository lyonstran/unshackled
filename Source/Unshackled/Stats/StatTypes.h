#pragma once
#include "CoreMinimal.h"

// had a stupid problem trying to fix this ECharacterStat/EStatType thingy stupid shit... trying to make more useful comments for the future me or future person readingthis doesn't get a migraine fixing any problems that may arise lol 
// Which number a modifier changes.
enum class ECharacterStat : uint8
{
	MaxHealth,
	MaxMana,
	Damage,
	Defense,
	Speed,
	IncomingDamage,   // multiplier on damage taken, 1.0 = normal
	Lifesteal         
};
enum class EModifierKind : uint8
{
	Flat,
	Percent
};
enum class EStackRule : uint8
{
	RefreshSameSource,   // replaces the old one and restart its timer
	StackToMax,          // add one stack (up to MaxStacks) and restart the timer
	ConsumeOnHit         // like Refresh, but also removed after HitsRemaining hits
};

enum class EStatusFlag : uint8
{
	None,
	Root,    // cannot move, can still attack (if in range)
	Stun,    // cannot move or attack
	Freeze   // cannot move or attack (Kalia's time stop)
};

struct FStatModifier
{
	FName SourceId = NAME_None;
	ECharacterStat Stat = ECharacterStat::Damage;
	EModifierKind Kind = EModifierKind::Percent;
	float Amount = 0.0f;
	float Duration = 0.0f;
	float TimeLeft = 0.0f;

	EStackRule Rule = EStackRule::RefreshSameSource;
	int32 Stacks = 1;
	int32 MaxStacks = 1;

	// Only used by ConsumeOnHit.
	int32 HitsRemaining = 0;

	EStatusFlag Flag = EStatusFlag::None;
};