# Core game systems

* Let all entities possess Health stat,
"Defense" stat (Sort of like a resistance stat that mitigates damage), damage stat(affects all things that do damage), and speed stat (how fast enetities move). Mana should also exist (but is exclusive to player party).
* there will also be items that can affect health, mana, defense and damage positive or negative. Skills can also affect these stats as well
* of course, retribution as well as grounded ally interactions can also affect these stats as well. I also want characters to have passive traits as well that can affect stats too.
* Below are a general overview of overworld stuff:

  * Sirius:

    * Basic Attack: Summons a cresent shaped swipe of air that slashes through enemies caught.
    * Basic Ability: The next three basic attacks are much bigger and the third and last hit summons a tornado that chases enemies, doing over-time damage for 5 seconds.
    * Ultimate ability: Summons a veil on themselves, and any allies that are grounded. This veil lasts for 10 seconds, and does damage to enemies within the veil radius, as well as slowing them for 50%, and reducing defense by 25%
    * Retribution: Sirius occupies the floor gusts of strong wind torrents. For 10 seconds, all enemies present lose 50% of defenses, and are slowed by 25%. All of sirius's attacks now also restore health based off 50% of the damage they deal.
    * Passive: Execute enemies below 5% health, and allies that swap in from them receive a veil of wind magic that reduces the damage on the next attack dealt to them by 25%; this veil expires after 3 seconds.
    * Grounded: while grounded, Sirius launches air blades at enemies. Air blades that hit allies slow their speed by 10% non-stacking.
  * Mal'Kholm Ecks:

    * Basic Attack: Swings chain around in a circle alternating clockwise and counterclockwise direction. Every other attack increases Mal'Kholm's defense by 2% percent, up to 10%, for up to 7.5 seconds. Enemies hit by the edge of attack every other hit lose 2% damage, up to 10%, and are slowed by 15% for 1 second.
    * Basic Ability: After winding up for 1.5 seconds, he slams both chains down in front him. Enemies hit are slowed by 50% for 1.5 seconds, and enemies hit at the tip of the attack are rooted (0% speed)  for 2 seconds.
    * Ultimate ability: Mal'kholm yanks himself to the highest health enemy in range. He then gains 100% of their max health, 50% of their defense, and his basic ability no longer has a cooldown. This effect lasts for 10 seconds.
    * Retribution: Temporarily reduce current health by 35%. For 10 seconds, Mal'Kohlm does 25% increased damage, and his basic attack, basic ability, and ultimate ability range is doubled. After the duration is over, heal back 25% missing health + 20% max health.
    * Passive: The first time Mal'kohlm Ecks would die, he insteads heals back to 100% max health + heal 10% from damage dealt, but has 100% decreased defense. These effects last 15 seconds. This passive has a cooldown of 180 seconds.
    * Grounded: While grounded, Mal'Kohlm periodically whips his chains out in a circle that knocks back enemies. If ally reaches 10 percent health, Mal'kholm will yank them to him and automatically swap in with them, rejoining the party. Grounding cooldown is activated.
  * Kalia:

    * Basic attack: Kalia slashes with her long dagger.
    * Basic ability: Kalia lines up a knife in a direction where after 1.2 seconds passes, the dagger flies in that direction. If it hit an enemy, she vaults blinks behind them, reduces their defense by 50%, and does 200% damamge to them. If this knock the target below 10% health, the cooldown on this ability is refreshed.
    * Ultimate ability: Kalia into freezes time, and her dagger transforms into a magic blade, and is covered in dark armor. All enemies and allies are frozen. For 12 seconds, her basic ability is replaced with a blink with no cool down, and costs 0 mana to cast. Every blink moves her to a random enemy or ally (if they are grounded). Allies that are blinked to are healed for 12% max health, and enemies blinked take 12% missing health damage. After 12 seconds are up, all damage is repeated twice for 12% effectiveness. Each second she is in her ultimate, she loses 12% current health down to 1.2% max health. She cannot die or go below 1.2% max health during her ultimate. Recasting the ultimate ends it early.
    * Retribution: For 12 seconds, Kalia gains 12% speed, 12% defense, 12% attack, and heals back 1.2% of damage done.
    * Passive: Enemies that die from Kalia's attacks explode and freeze nearby enemies for 1.2 seconds.
    * Grounded: Within a radius, she dashes around slashing enemies.
  * Hanuboy:

    * Basic attack: Does a front flip which launches his big chain ball at enemies forward and backward. Enemies hit are knocked back.
    * Basic ability: Summons a giant flower that emits a healing zone, healing for 2.5% max health per second, and restoring 2.5% max mana per second.  This flower can be used by allies and expires after 15 seconds, or until it is destroyed (enemies can target this flower).
    * Ultimate: Summons a tree that fires rays of sun energy at enemies, dealing 10% missing health damage. Enemies that die within 3 seconds of being hit turn into the same flowers as the ones in Hanuboy's basic abilities. This tree expires after 30 seconds and fires a shot every 1.5 seconds.
    * Retribution: Hanuboy makes it rain petals. These flowers can be abosrbed by allies later to heal them for 2.5% missing health, and restore 2.5% missing mana. These petals expire after 10 seconds.
    * Passive: Hanuboy periodically throws out flower bombs that explode when enemies step on them. Enemies lose 50% defense, do 50% less damage, and are slowed for 50% when hit by the explosion.
    * Grounded: Hanuboy becomes very territorial. He goes around bashing enemies into the ground with their chain ball, rooting them for 2 seconds. The space Hanuboy can occupy grows the closer allies are to him (grounded or nearby).
  * Basic guard:

    * Basic attack: When in range, slashes their sword at player or ground ally, which ever is closer.
  * Wardens:

    * Basic attack: When in range, they slam their maces down. Allies hit are rooted for 0.5 seconds, and lose 25% defense for 3 seconds.
    * When they are low on health, they raise their shield up, boosting defense for 25% until death.
  * Caster guards:

    * Basic attack: runs into range of player or grounded ally, where they then targetted magic missles.
    * If player is in range of them, they will run away until ready to fire again
  * Caster wardens:

    * Basic attack: after winding up for 2 seconds, they shoot a cluster of magic comets at either the player or a grounded ally (which ever is closer). Allies hit lose 10% current health, and lose 50% defense.
    * Their are surrounded by a magic circle. Allies in this circle lose 10% of their defense. They have infinite range.
  * Floor director (physical):

    * basic attack: after winding up ram for 3 seconds, they charge at the player. If it hands the player, player loses 25% max health, and are stunned for 1 second.
    * nearby enemies are rallied, gaining 25% defense, and 10% attack
  * Floor director (magic):

    * Basic attack: After winding up for 2 seconds, they shoot a continuous laser beam for 1 second. Allies that are caught in it lose 50% defense, and take 15% missing health damage.
    * they will periodically summon basic guards, or caster guards, 2 at a time, alternating
    * They are surrounded by a magic circle that reduce player defenses by 15% if in it. They have infinite range.
  * Floor director (hybrid):

    * Basic attack: After winding up ram for 3 seconds, they charge at the player.  If it hands the player, player loses 25% max health, and are stunned for 1 second. After, they stand still and wind up for 3 seconds, where afterwards, they shoot a laser beam following the player for 3 seconds. Allies caught take up to 25% max health damage based on duration exposed, and are slowed for 15%.
    * they will periodically summon basic guards, or caster guards, 2 at a time, alternating
    * They are surrounded by a magic circle that reduce player defenses by 15% if in it. They have infinite range.
    * nearby enemies are rallied, gaining 25% defense, and 10% attack.

I do need to add base stats to abilities on top of the other scalings present in the existing abilities <- applies to both overworld and turn-based stuff

# Core turn based stuff

Every character will have a basic attack, block (damage reduction by like 50% maybe?), option to use items, and .... pending anything else that comes to mind. Actual mana costs of abailites are yet to be determined... also, not all abilities will be avaliable at the start, as the player progresses, abilities unlock respectively for each character. Note: turns and rounds are used interchangebly here. 

* Sirius

  * passive: attacks and abilities apply stacks to enemies. Each stack reduces enemy damage by 1% percent. At 25 stacks, enemies are are executed by Sirius's attacks if they are below 5% health.
  * basic attack: an option for the player to go for a simple attack (similar to the overworld basic attack), except it only locks onto one target. sirius targets an enemy and slices them with a blade of wind magic.
  * Specific moves:

    * gale: a strong blast of wind magic. Single-target.
    * veil: surrounds allies, providing 15% damage reduction, and reflects back 10% of damage back towards enemies. This effect lasts 3 turns.
    * winds unbound: raise attack by 15%. This effect lasts 3 turns. This effect does not stack.
    * gale unleased: inflicts powerful tornado(AOE) onto all enemies, where they take initial flat damage and for the next three turns, enemies take weaker flat damage again (maybe like 10% of what the initial tornado damage was)
    * wind wall: sets up a shield that consumes the next damaging attack from enemy (directed towards sirius). 20% of that damage is restored as mana. Subsequent use reduces chance of successful abilitiy cast (-10%) for the rest of the battle, starting at 100% successs rate
    * winds of war: raise attack stat of allies and enemies by 10%
    * submission: Sirius lowers an enemies attack, and defense by 15%. This debuff lasts for 3 turns. 
  * retribution: sirius's next damaging ability restores 25% max health.
* Mal'Kholm Ecks

  * passive: The first time Mal'Kholm would die, he instead survives at 1 hp, his defense is increased by 50%, any and all sources of healing or shielding(if implemented) are doubled. This effect lasts for 3 turns. Also, the lower Mal'Kholm's health is, the stronger he gets (+0.25% attack per 1% missing health)
  * basic attack: Mal'Kholm strikes twice, and is able to select up to two targets; if both attacks are directed at one target, he gains 1.5% defense -> this increase is permanent for the remainder of the battle
  * Specific moves;

    * revolution: at the start of the following round, all healing, shielding, damage, and buffs in the previous round is repeated at 25% effectiveness (would have to define what that looks like for each kind of staat alteration) -> allies only. This move is disabled for one round afterwards.
    * sacrifice: forfeit 5% current health, and the following turn, Mal'Kholm is granted an additional turn.
    * Might: Mal'Kholm slams both chains down onto an enemy. Single-target.
    * Freedom: grants an ally +7.5% defense for 3 turns. This effect can stack up to 3 times. Stacks refresh turn duration.
    * Confrontation: Mal'Kohlm hooks onto an enemy and takes 15% of their defense as his own, heals himself for 50% of enemy current hp, and deals 10%  of his missing health + 5% max enemy health as damage.
    * power: Mal'Kholm grants allies 5% of his defense + 5% of his missing health as a shield to all allies. This shielf can stack (shields are essentially health with 0 defense).
    * unity: Mal'Kholm recycles the retribution cycle to start on him. Each successful pass between allies grant all allies 5% max health shield. If retribution is successful, reduce enemy damage by 10%. 
  * Retribution:

    * Player gains an additional turn (able to reselect another character after making a move on Mal'Kholm).
* Hanuboy

  * passive: Every round, an enemy or ally has a flower bomb dropped onto them, where in the following round, they either take 5% current health damage (enemy) or heal for 10% current health (ally). In addition, every  spawn fielded gives Hanuboy +5% mana restoration. The flower bomb counts as a spawn.
  * basic attack: hanuboy smacks his chain ball onto an enemy. This has a 10% of skipping a turn from the boss. 
  * Specific moves: 

    * sprout: hanuboy summons a flower. Every round, the flower restores 2% max health to all allies. This flower expires after 5 turns, or until destroyed (takes one hit to be destroyed). This flower is stackable up to 2 times.
    * blossom: hanuboy summons a tree. On spawn, this tree deals 5% max health damage to all enemies. For the next 3 rounds, enemies take 2.5% max health damage. Enemies cannot destroy this tree. In addition, any debuffs to allies to halvened. 
    * harvest: hanuuboy destroys all spawns. For each destroyed spawn, Hanuboy gains 3% damage. This effect lasts for 3 turns.
    * territory: hanuboy summons a mushroom. Every round, the mushroom restores a flat \_\_\_ to all party members mana. The mushroom lasts for 3 rounds, and is stackable up to 2 times. Destroyed by one hit. 
    * squeak: Hanuboy raises defense of one ally by 15% for 3 rounds. 
    * super loud squeak: Hanuboy restores health 5-10% max health to all party members (randomized)
    * shelter: select an ally; If that ally takes damage from an enemy in the next turn, Hanuboy splits that damage with them and himself.
  * Retribution: everyone gets dropped a flower bomb immediately. 
* Kalia

  * passive: Every other turn, Kalia is taken over by her sickle (yes I'm changing her weapon to be a sickle, and this "take over" is a lore thing that I want to make apparent when Kalia ults in the overworld as well). The entity that takes over her is named Proxy. Anyways, every other turn, Proxy takes over and suggests an action at random (I want it to be skewed towards using an item or blocking, even if it's not neccessitated given player situation -> this quest should be hard to fulfill). If Kalia (player) chooses said action, Kalia gains a stack. Each stack grants Kalia +1.2% damage. On collection of 12 stacks, Proxy takes over Kalia, and for the next 12 moves (essentially 3 rounds where the boss should be able to move) from the boss are fully negated (they do not work -> there will be a dialogue/sprite alteration that indicates boss was frozen in time). Everyround thereafter, the boss loses 1.2% of their defense and attack. 
  * basic attack: Kalia slices an enemy with her sickle. 
  * Specific moves:

    * Chaos: A random ally gains a random boost (1.2-12%) to a random stat (attack or defense) for a random amount of turns (1-5). 
    * Sever: Kalia throws her sickle which lands the following turn. it has a 12% chance of missing. If it lands, Kalia dashes to the blade, and strikes at the targetted enemy. The enemy loses 12% defense and 12% attack, lasting for 3 turns. This debuff cannot stack. They also take 1.2% max health damage + 12% missing health damage. Following uses of this move increase the chance of failure up to 96% for the rest of the battle, but damage of the attack is increased by +1.2% max health and + 12% missing health damage each cast of this ability, regardless of whether or not the ability lands or not. 
    * Battlesong: Kalia boosts the damage of an ally's next attack by 12%. 
    * Riposte: Kalia readies a parry. If an the enemy damages her, she reduces the damage by 50%, sends 50% of that damage back at the enemy, and repels any potential debuffs that would've otherwise been inflicted upon herself.
    * Barrage: Kalia strikes multiple times at an enemy. 
    * Concentration: Kalia's next attack reduces targetted enemy defense by 12%. 
    * slayer: Kalia stabs an enemy. every time it's the enemies turn to attack, they take 1.2% missing health damage. This debuff cannot go away (unless enemy has a cleansing ability). 
  * Retribution: Kalia cleanses all debuffs inflicted on ally's, excluding herself.

