# Progress Check

**Lyons Tran, GSD390, Oct 14, 2026**

## at a glance...
My final project, *Unshackled*, currently is now functional as a gray box test level. A party of three fights four waves of enemies in real time, and the enemy count grows with the floor number and party size. Stats, abilities, party switching and grounding all run in C++.

The art right now is absent, and is using a placeholder (Elliott Smith's *Either/Or*, IFYKYK). Every visual is a debug shape that hides itself once a sprite is added, so I can add art later without changing any code.

If you've played *League of Legends* (got re-addicted to the game this summer break, so it's pretty fresh on my mind), I took a lot of inspiration from the depth of a some abilities, with the hope that this game makes player think a little bit more about their actions instead of button mashing, an aspect that really draws me to turn-based style games .

Something else I want to also keep in mind is how much time I'm putting into this project. I think for the last two weeks or so, I've pretty much thought about and worked on this game for a majority of my day, which although is really fun and making me quesiton if I want to explore more opportunities in game dev, it's been affecting my physical health a bit. I **will** make more of an effort to go outside and touch grass haha.

 The turn based half has not been started. All work thus far has been on the overworld aspect of the game.

| Feature Implemented | Status | Location |
| --- | --- | --- |
| Flying top down movement, and a dash that makes you immune to damage for 1 second | Done, tested | `UnshackledCharacterBase` |
| Health, damage, death and revive | Done, tested | `HealthComponent`, `UnshackledCharacterBase` |
| Stats, defense math, and buffs and debuffs that stack | Done, tested | `StatsComponent`, `StatTypes.h` |
| Aiming with the mouse or right stick | Mouse tested, stick waiting on a controller | `UnshackledPlayerController` |
| Ability system: mana, cooldowns, windups, mana regen | Done, tested | `UnshackledCharacterBase` |
| Sirius: crescent, empowered attacks and tornado, wind veil, execute | Done, tested | `SiriusCharacter`, `Abilities/`, `Projectiles/` |
| Sirius: swap veil passive | Done, Tested | `SiriusCharacter` |
| Party followers and switching leaders with Tab | Done, tested | `PartyMemberCharacter`, `UnshackledPlayerController` |
| Grounding and recalling allies (mana drain, cooldown, auto recall) | Done, tested | `PartyMemberCharacter`, `UnshackledPlayerController` |
| Enemy AI that chases and attacks | Done, tested | `EnemyCharacter` |
| Wave spawner that reads a Data Table and scales by floor and party size | Done, tested | `WaveSpawner`, `DT_Waves`, `DT_Floors` |
| HUD: health and mana bars, party card menu, Grounded tag | Done, tested | `UnshackledHUD` |
| Mal'Kholm, Kalia and Hanuboy abilities | Designed in `plans.md` | Not built |
| Retribution, items, inventory, saving | Designed in GDD | Not built |
| Turn based boss fights and telegraphs | Designed in GDD | Not built |
| Dialogue, art, audio | Designed in GDD | Not built |

Party members and enemies use the same damage, stats and ability code. That means a buff or a hit works the same way on both sides.

## Scripts

Every script below is in `Source/Unshackled/`. Each one compiles and has been tested in the editor unless I say otherwise. The game rules live in C++. Blueprints only hold tuning numbers and, later, sprites.

### Stats/StatTypes.h: the shared vocabulary

This file lists the seven stats every character has: max health, max mana, damage, defense, speed, incoming damage and lifesteal. It also defines a few other things:

* A buff or debuff can change a stat by a flat amount or by a percent.
* There are three rules for what happens when the same effect is applied again.
* There are three status effects: Root (cannot move), Stun and Freeze (cannot move or attack).

A `FStatModifier` is one buff or debuff. It says which stat it changes, by how much, for how long, and which stacking rule it uses. 
### Components/StatsComponent: buffs, debuffs and mana

This keeps a list of every active buff and debuff on a character. When the game asks for a stat, it works out the final value like this:

*Final stat = (Base + all flat bonuses) x (1 + all percent bonuses)*

The result can never go below 0, and max health can never go below 1.

* Percent bonuses add together. Two +25% buffs make +50%.
* Slows are the exception. Only the strongest slow counts, and it is capped at 80%. That way stacking slows can never freeze an enemy completely.
* When a buff is added, it follows its stacking rule:
    * *Refresh: applying it again replaces it and restarts the timer.
    * Stack to max: eac new application adds a stack, up to a limit.
    * Consume on hit: it disappears after a set number of hits.
* Every frame, timers count down. The list is rebuilt with only the buffs that are still active. 
* `CanMove()` and `CanAct()` check for Root, Stun and Freeze.
* `SpendMana` and `RestoreMana` handle mana for party members.

### Components/HealthComponent: hit points

This stores current health and max health.

* `TakeHit` subtrts damage, but never below `HealthFloor` (for planned "cannot die" effects).
* `Heal` does nothing to dead characters. Only `Revive` can bring one back, at half health.
* `SetMaxHealthFromStats` handles buffs to max health. If max health goes up, the character gains that much health too. If it goes down, current health is lowered to fit.

I'm still figuring out how revive mechanics might work, and who/what might be able to do that.

### Characters/UnshackledCharacterBase: the parent of every character

Both the party and the enemies come from this class. It builds on PaperZD's `APaperZDCharacter` and owns the Health and Stats components. It handles:

* Movement: characters use flying mode on a flat plane. The Speed stat is copied into the max fly speed every frame, so slows work right away.
* Dash:* the character launches forward, can't be hurt for 1 second, and then has a cooldown. Launching takes a character out of flying mode, so `EndDash()` puts it back.
* Attacks and abilities: before an attack or ability fires, it checks the cooldown, whether the character is dead or stunned, and whether there is enough mana. Then it calls `PerformBasicAttack`, `PerformSpecial` or `PerformUltimate`. Each party member only has to write those three functions. Windups and cooldowns use timers.
* Taking a hit (`ApplyHit`):
    1. Skip it if the character is dead or dashing.
    2. Lower the damage with defense and incoming damage.
    3. Use up any "consume on hit" buffs.
    4. Subtract health and measure how much was really lost.
    5. Check for death.
    6. Tell the attacker how much damage was dealt. This is currently used for lifesteal and Sirius's execute.

Defense lowers damage on a curve, so it never makes a character fully immune:

`Damage taken = Raw damage x 50 / (50 + Defense) x Incoming damage`

With 0 defense you take full damage. With 50 you take half, and with 150 you take a quarter. "True damage" skips the defense step. It is used for effects based on a percent of health, and for executes (currently only Sirius's).

### Characters/PartyMemberCharacter: the player's party

This adds the overhead camera and mana.

* Followers: party members the player is not controlling. Their collision is off, so they can't be hit. They walk toward the leader when they get too far away.
* Grounded allies: they stand still and can be hit. They lose 8 mana per second and are recalled automatically when their mana runs out. Any recall starts a 30 second cooldown.
* Death: when the leader dies, control moves to the next ally who is still alive. The level only restarts when the whole party is dead.
* *Swap hooks: `OnSwappedIn` and `OnSwappedOut` let each character add its own effects when the player switches characters.

### Characters/SiriusCharacter: the main character

* X, Crescent: fires a wide, short wind slash that passes through enemies.
* C, Empowered strikes: the next 3 attacks do 1.5x damage and are 2x bigger. The third one also spawns a tornado.
* V, Wnd veil: creates a veil around Sirius for 10 seconds. It hurts enemies inside, slows them by 50% and lowers their defense by 25%.
* Passive, Execute: after any hit, an enemy below 5% health gets finished off with true damage.
* Passive, Swap veil: when the player swaps away from Sirius, the ally coming in takes 25% less damage from their next hit (within 3 seconds). 


### Projectiles/AbilityProjectile: a reusable projectile

`Launch` sends it flying in a direction. Every frame it moves, checks for enemies around it, and hurts each enemy only once. It is destroyed when it reaches its max range. This one class is the crescent now. Later it will also be the air blades and the caster missiles, or any other units that may have projectiles. 

### Abilities/TornadoActor and Abilities/VeilActor

The **tornado** lasts 5 seconds. It chases the nearest living enemy and damages everything inside it every 0.5 seconds.

The **veil** follows its owner for 10 seconds. Every 0.25 seconds it gives enemies inside it a short slow and a short defense debuff. That means the debuff wears off as soon as an enemy walks out. If the owner dies, the veil ends. The tornado keeps going.

### Characters/EnemyCharacter: enemy AI

Every frame, an enemy finds the closest target. A target is the leader or any grounded ally, never a follower. The enemy walks straight toward it and attacks once it is within 90 units. For now, caster guards and floor directors use this same behavior with different stats. Their own attacks come later.

### Player/UnshackledPlayerController: input and party control

This connects all the controls: move, attack, dash, special, ultimate, aim and the party menus.

* **Aiming:** it finds where the mouse points on the game floor. If a controller is in use, it uses the right stick instead, and it switches back to the mouse when the mouse moves.
* **Party menu:** holding Tab, G or F opens the party card menu to switch, ground or recall an ally. The scroll wheel moves the selection, and letting go of the key confirms it.
* **Switching (`SwitchTo`):** the player takes control of the new leader. The old leader gets an AI controller so it can follow. Then the swap hooks run.

### Waves/WaveRow.h and Waves/WaveSpawner: waves from data

Waves are rows in a Data Table (`DT_Waves`). Each row has a floor, wave number, enemy type and base count. Because the waves are data, I can tune them without recompiling. When the player walks into the start area, the spawner reads the rows for the current wave and scales each count:

*Count = BaseCount x 1.2 to the power of (Floor minus 1) x (1 + 0.1 x (PartySize minus ExpectedPartySize))*, rounded up

Finally, as of right now, minibosses do not scale. Enemies should spawn one at a time, with at most 8 alive at once and a 2.5 second break between waves. After the last wave, dead allies come back and the floor is cleared.

### UI/UnshackledHUD: what the player sees

It draws:

* Health bars over every living character (red for enemies, green for the party)
* Blue mana bars for the party
* A green "Grounded" tag over grounded allies
* The party card menu. Each card has a portrait, a name, a status (Leading, Grounded, Down or Cooldown), a mana bar and a health bar. The selected card gets a gold border.

### Game/UnshackledGameMode

This sets which player controller and HUD the game uses. Later it will handle winning, losing and moving to the boss maps.

## Blueprints, data and input assets

They only hold tuning numbers, so the logic stays in code and I can retune relatively easily. It would suck to have to recompile the code each time I wanted to tune something.

| Asset | What it is |
| --- | --- |
| `BP_PartyMember` | Built on `ASiriusCharacter`. The leader plus two follower copies are placed in the level. |
| `BP_BasicGuard` | Melee guard with 60 HP, 10 damage and 20 defense |
| `BP_CasterGuard`, `BP_FloorDirector` | Enemy versions with their own stats and color (their own attacks still to come) |
| `BP_GameMode`, `BP_PlayerController` | Connect the C++ game mode, HUD and controls together |
| `DT_Waves`, `DT_Floors` | Data Tables for Floor 1's four waves and the expected party size on each floor |
| `IMC_Overworld` and 11 Input Actions | Keyboard, mouse and controller bindings |
| `Level_Prototype` | Gray box Floor 1 test level with the wave spawner and spawn points |
| `Art/Placeholder` | A placeholder sprite and animation used until real art is ready |

Floor 1's waves come from `DT_Waves`, and I checked them in play: *(these are just rough numbers for playtesting at the moment)*

* Wave 1: 4 guards
* Wave 2: 3 guards and 2 casters
* Wave 3: the floor director miniboss, 2 guards and 1 caster
* Wave 4: 4 guards and 3 casters

Two design documents guide the build. The GDD (`GDD.md` and `GDD.pdf`) covers *roughly* the whole game. **`plans.md`** covers each character's basic attack, ability, ultimate, Retribution, passive and grounded behavior, plus every enemy type (need to expand on enemy types (include hounds)).

Decisions I made while building, which add to the GDD:
* Defense works on a curve (formula above), so no amount of defense makes a character immune.
* Only the strongest slow counts, capped at 80%, so slows together cannot lock enemies in place. It'd be kinda lame to do that in my opinion. 
* Waves scale with floor and party size from one table written for the expected party size.
* Followers can't be hit, but grounded allies can, which hopefully  makes grounding a real risk and reward choice.
* Art can be added at any time. I wanted gameplay to not depend as much on sprites. This is so that adding/removing art later needs no code changes (should be easier).

## Next steps

1. Finish Sirius
2. Build Mal'Kholm, Kalia and Hanuboy from `plans.md`. Then build the enemy attacks (ranged casters, wardens, floor director types).
3. By roughly ~Nov 8-9th: saving progress between maps, items and inventory, ability cooldowns on the HUD, and the first turn based boss fight.
4. Run a prototype playtest with friends, using a packaged Windows build and a short feedback form.
5. Add sprites, portraits, music and sound effects sometime in Novemember.
6. Finalize story + cutscenes (I fear this may take a BIG chunk of time)

*Currently, I'm running into a lot of issues with development on Linux. It's a WIP, but I hope to have this game be compatible with Linux platforms as well.*