#pragma once
#include "CoreMinimal.h"
enum class ECharacterStat : uint8
{
	MaxHealth,
	MaxMana,
	Damage,
	Defense,
	Speed,
	IncomingDamage,
	Lifesteal
};
enum class EModifierKind : uint8
{
	Flat,
	Percent
};
enum class EStackRule : uint8
{
	RefreshSameSource,
	StackToMax,
	ConsumeOnHit
};
enum class EStatusFlag : uint8
{
	None,
	Root,
	Stun,
	Freeze
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
	int32 HitsRemaining = 0;
	EStatusFlag Flag = EStatusFlag::None;
};