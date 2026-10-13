# Core game systems

 - Let all entities possess Health stat, 
    "Defense" stat (Sort of like a resistance stat that mitigates damage), damage stat(affects all things that do damage), and speed stat (how fast enetities move). Mana should also exist (but is exclusive to player party). 
 - there will also be items that can affect health, mana, defense and damage positive or negative. Skills can also affect these stats as well 
 - of course, retribution as well as grounded ally interactions can also affect these stats as well. I also want characters to have passive traits as well that can affect stats too. 
 - Below are a general overview of overworld stuff:
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