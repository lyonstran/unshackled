# Unshackled
### GSD390 Final Project in Unreal Engine 5 with C++
**Author:** Lyons Tran

**Version:** 1.0

---

## Table of Contents

1. [Title and Overview](#1-title-and-overview)
2. [Gameplay Mechanics](#2-gameplay-mechanics)
3. [Technical Requirements](#3-technical-requirements)
4. [Level Design and World Structure](#4-level-design-and-world-structure)
5. [Visual and Audio Design](#5-visual-and-audio-design)
6. [Story and Narrative](#6-story-and-narrative)
7. [User Interface (UI) and HUD](#7-user-interface-ui-and-hud)
8. [Testing and Iteration Plan](#8-testing-and-iteration-plan)
9. [Project Timeline](#9-project-timeline)

---

## 1. Title and Overview

### 1.1 Game Title
The working title of this game is *Unshackled*. 

### 1.2 Game Genre
Mystery/Fantasy RPG. Turn-based + A-RPG

### 1.3 Target Audience

| Attribute | Details |
|---|---|
| Age Group / Rating | ESRB T, 13 and up |
| Platform | Windows, Linux |
| Player Demographic | Catered to players who enjoy turn-based combat and beat-em ups, as well as following a story |
| Play Session Length | 3-4 hours |

### 1.4 Game Summary

Taking place in a dungeon, the game aims to curate an experience that combines both turn-based combat mechanics and A-RPG style beat-em ups as a means to progress through the game, while also following a visual-novel based story. Players will progress through the game fighting waves of enemy per level, unlocking story-content along the way, and engaging in turn-based combat with mini-boss and boss-level entities. 

The story follows Sirius. Plagued with amnesia, they can't figure out why they are locked in a dungeon, better yet, why they are in the depths of said dungeon housing the most dangerous of the dangerous, all while missing their left arm. In finding out the injustice that takes place in the dungeon, they set off to break out of the dungeon, rallying allies to join their party along the way. Can Sirius break out of the unforgiving hellscape that surrounds them and their allies, and can they remember why they are there in the first place? 

### 1.5 Unique Selling Points
- Floors are real-time A-RPG fights against waves of enemies, and bosses are turn-based. How the player plays decides the Retribution bonuses and starting conditions for the boss fight.
- Grounding lets the player drop allies around the floor as semi-independent units that cast their own spells and interact with each other. This turns wave combat into positioning and area control and rewards player for strategizing instead of button-mashing.
- The last fight is against Sirius, using every skill the player unlocked. Upgrading your character over the whole game is also building your final opponent, so the twist hits both in the story and in the gameplay.
- Story pieces unlock floor by floor, so the player's progress in the escape is also their progress in finding out who Sirius is and why they were locked up at the bottom of the dungeon.

---

## 2. Gameplay Mechanics

### 2.1 Core Mechanics

**Core Loop:** 

- Defeat waves of enemies 

- Gain EXP + items

- Upgrade party members 

- Unlock story content 

- Engage in turn-based combat with boss 

- Repeat

| Mechanic | Description | Purpose in Gameplay |
|---|---|---|
| Basic movement | Simple navigation around levels | Movement will be a core part of the overworld aspect of the game. Mobs of enemies will chase the player and party around the level in which the player's party needs to be able to manuever strategically around to defeat enemies efficiently |
| Grounding allies | Deploy members of party around the level to automatically interact with enemies.  Party members will have unique interactions with enemies and allies when "grounded" (ex. casting specific spells or skills). | This serves as a way for the player to strategize throughout the level |
| Retribution | Sequence of party member order will affect turn-based combat by giving player's party additional bonuses (damage boosts, additional shields/heals, enemy debuffs, etc) | To reward players for strategy and planning when fighting enemies |
| Leader | Party members can switch a "leader" of party to start combat. This has unique effects and interactions with enemies and allies | This has unique effects and interactions with enemies and allies, both negative and positive |

**Objectives:** 

- Successfully escaping the dungeon.
- Recruiting new party members to aid in their dungeon escape. 
- Regaining the lost memories of the protagonist.

**Challenges:** 

- Enemy mobs (dungeon guards, wardens, dungeon floor bosses)
- Dungeon terrain 

### 2.2 Controls and Player Input
**Input Method(s):** Keyboard & mouse, controller (Xbox / Playstation)

**Input System:** Unreal's Enhanced Input System, with two Input Mapping Contexts: `IMC_Overworld` for real-time floors and `IMC_TurnBased` for boss fights. The Player Controller can swap between them when combat changes mode. 

*'O' indicates overworld mechanics and 'T' indicates turn-based combat mechanics. Otherwise, action is a system-wide mechanic.*

| Action | Keyboard/Mouse | Controller |
|---|---|---|
| O-Move | WASD | Left Stick |
| O-Look | Move mouse | Right Stick |
| O-Jump | Space | Cross / A | 
| Pause | Esc | Menu/Options |
| O-Physical Skill | X | X / Square |
| O-Special Skill | C | Y / Triangle |
| O-Ultimate Skill | V | RT / R2 |
| Use item | Q | D-Pad up |
| O-Tackle | Left shift | B / Circle |
| O-Ground party member | G | LT / L2 |
| Interact | Left mouse click | LB / L1 |
| Open inventory | R | View / Touchpad |
| O-Switch leader | Tab + Scroll wheel (to cycle) | RB / R1  + D-pad (to select)|
| T-Toggle skills | Arrow keys | D-pad |
| T-Use skill | Space | Cross / A |
| T-Hover party member/Hover enemy | WASD | Left Stick |

### 2.3 Player Progression

| Stage / Level | New Ability or Mechanic | Challenge Introduced |
|---|---|---|
| Floor 1 | Movement, Grounding, Retribution, Leader + all other core game mechanics + first party member | Waves of enemy mobs + floor boss + story mystery |
| Floor 2 | Second party member + story progression | mobs + boss + terrain |
| Floor 3 | Third party member + story progression | mobs + boss + terrain |
| Floor 4 | fourth and final party member + story progression | mobs + boss + terrain |
| Floor 5 | story progression | mobs + 2 boss fights |
| Escape | finale of story | betrayal (final boss fight) |

### 2.4 AI Behavior

| Enemy / NPC | Movement | Attacks / Actions | Decision-Making |
|---|---|---|---|
| Basic guard | Running (slightly faster than player's party) | basic melee attacks | Chases at start of level, does not yield until death |
| Wardens | slow walk | Slower but stronger melee attacks, can buff enemies | Spawns in a bit later, chases throughout level, does not yield until death |
| Caster guards | levitation (slightly slower than player's party) | magic spells (can apply debbuffs) | Follows player party until they are in range, runs away if player party is too close, stops after death |
| Caster wardens | levitation | very far ranged attacks (applies debuffs), can buff enemies | Casts spells towards player's party after spawning in, does not stop until defeated |
| Hounds | Sprints (much faster than player's party) | small ranged, powerful melee attacks | slowly follows player party trail(s) until spotted in vision, where it then hunts player down until death |
| Floor director (miniboss) | dependent on floor (each director is unique) | dependent on floor, miniboss will either be physical (short range melee), magic (long range spells), or both | dependent on miniboss, but they do not yield until defeated |
| Floor boss (Goat, serpent, lion, dragon, Chimera) | no movement  | dependent on floor, each boss is unique | bosses will have turns equal to number of party members, and will choose most optimal moves given circumstances; stops once defeated |
| S. Ravana + Chimera | no movement | Final boss is our main protagonist, Sirius; will have all skills player unlocked that can be used to attack player party | Given three turns (the total number of remaining party members) and will choose most optimal moves given circumstances; stops once defeated |

---

## 3. Technical Requirements

### 3.1 Unreal Engine 5.8 Features
*Unshackled* is top-down 2D pixel art with illustrated dialogue portraits. Nanite and Lumen are left off, since they don't help a sprite-based game.

| Feature / Tool | Use |
|---|---|
| C++ | Core systems: characters, combat, stats, turn-based battles, spawning, NPC, dialogue, saving |
| Blueprints | Character/enemy setup, level scripting, UI layout, tuning |
| Paper2D | Sprites, Flipbooks (animation), and Tile Maps (dungeon floors) |
| PaperZD | Animation state machines with directional facing; frame-based events for hit timing |
| Orthographic Camera | Top-down follow camera at pixel scale |
| Enhanced Input | All controls, with overworld and turn-based mapping contexts |
| AI (Behavior Trees, Perception, NavMesh) | Enemy and grounded-ally behavior, sight detection, pathfinding |
| Gameplay Tags | Skill types, damage types, and status effects |
| Data Assets / Data Tables | Stats, skills, enemies, waves, EXP curves, and dialogue scripts |
| UMG + Common UI | HUD, menus, inventory, battle UI, dialogue box; controller menu navigation |
| UMG Animations | Portrait slide-in/out, speaker highlighting, and small reactions |
| Niagara | Sprite-based spell, hit, and ambient effects |
| Level Sequencer | Short story scenes and boss intros |
| MetaSounds / Sound Classes | Music transitions and volume sliders |
| SaveGame | Progress, party, inventory, story flags |
| Lighting | Lit sprites with point lights; Lumen and Nanite disabled |
| Chaos Physics | Collision/hitboxes and Tackle knockback only |
| Texture Settings | Pixel art uses nearest-neighbor filtering, no mipmaps; portraits use UI filtering. Anti-aliasing, motion blur, and auto-exposure off |

### 3.2 Programming Requirements (C++)

| C++ Class | Parent Class | Responsibility |
|---|---|---|
| `AUnshackledCharacterBase` | `APaperZDCharacter` | Shared base for party and enemies |
| `APartyMemberCharacter` | `AUnshackledCharacterBase` | Leader effects, grounding, following |
| `AEnemyCharacter` | `AUnshackledCharacterBase` | Drops, AI setup, enemy type data |
| `AUnshackledPlayerController` | `APlayerController` | Input contexts, leader switching, grounding, menus |
| `AUnshackledGameMode` | `AGameModeBase` | Waves, win/lose, switching to boss fights |
| `UUnshackledGameInstance` | `UGameInstance` | Party, inventory, and story flags across floors |
| `AWaveSpawner` | `AActor` | Spawns enemy waves from Data Tables |
| `ATurnBattleManager` | `AActor` | Turn order, Retribution bonuses, skill resolution |
| `UBossDecisionComponent` | `UActorComponent` | Scores and picks the best boss move |
| `AEnemyAIController` | `AAIController` | Behavior Trees and Perception |
| `UBTTask_*` / `UBTService_*` | `UBTTaskNode` / `UBTService` | Custom AI actions (keep distance, track, buff) |
| `UHealthComponent` | `UActorComponent` | HP, shields, damage, death |
| `UStatsComponent` | `UActorComponent` | Level, EXP, stats |
| `USkillComponent` | `UActorComponent` | Skills and cooldowns in both combat modes |
| `UStatusEffectComponent` | `UActorComponent` | Buffs and debuffs |
| `UInventoryComponent` | `UActorComponent` | Items |
| `USkillData` / `UCharacterData` | `UPrimaryDataAsset` | Skill and character data, including portrait sets |
| `UDialogueManager` | `UGameInstanceSubsystem` | Runs dialogue from Data Tables, sets story flags |
| `UDialogueWidget` | `UUserWidget` | Text reveal, portrait expressions, speaker highlight |
| `UUnshackledSaveGame` | `USaveGame` | Save data |
| `UUnshackledHUDWidget` / `UBattleWidget` | `UUserWidget` | HUD and battle UI data |

**Key Systems to Implement:**
- [ ] 2D top-down character controller
- [ ] Real-time combat (hitboxes, damage, knockback, cooldowns)
- [ ] Grounding and party following
- [ ] Leader switching
- [ ] Wave spawning and floor progression
- [ ] Turn-based battles with Retribution
- [ ] Enemy AI and boss AI
- [ ] Buffs/debuffs
- [ ] EXP and upgrades
- [ ] Inventory
- [ ] Visual novel dialogue
- [ ] HUD, battle UI, menus
- [ ] Save/load

**C++ vs. Blueprint Split:** Game rules and systems are in C++, so they're fast, easier to debug (in my opinion), and should be able to merge cleanly between machines. Blueprints can handle the presentation side of things with character setup, animations, UI layout, level scripting, and tuning. Stats, skills, and dialogue will live in Data Tables so changes don't need a recompile.

### 3.3 Assets and Tools

| Asset Type | Source | Custom / Premade | Notes |
|---|---|---|---|
| Sprites & Animations (Pixel) | Aseprite; itch.io / Fab packs | Mix | Main characters custom; generic enemies edited from packs |
| Tilesets (Pixel) | itch.io / Fab | Mostly premade | Custom tiles for hazards and floor features |
| Dialogue Portraits (Illustrated) | Krita | Custom | Bust portraits with multiple expressions |
| UI (Pixel) | Aseprite | Custom | HUD, icons, menus, dialogue frame |
| VFX Textures | FX packs + custom | Mix | Used in Niagara |
| Sound Effects | Freesound.org / Fab / itch.io | Premade | Slashes, spells, hits, footsteps, UI |
| Music | Waveform Free / Ardour + guitar | Custom | Original soundtrack featuring recorded guitar |
| Fonts | Free pixel fonts | Premade | Chosen for readability |

**Development Environment (Windows 11 + Fedora Linux):**

| Tool | Windows 11 | Fedora |
|---|---|---|
| Unreal Engine 5.8 | Epic Games Launcher | Epic's Linux build |
| IDE | Rider | Rider |
| Compiler | MSVC | Clang (UE toolchain) |
| Graphics API | DirectX 12 | Vulkan |
| Pixel Art / Illustration | Aseprite / Krita | Aseprite / Krita |
| Audio | Waveform Free, Audacity | Ardour, Audacity |
| Version Control | Git + GitHub + LFS | Git + GitHub + LFS |

### 3.4 Target Platform
These are only estimates and need to be confirmed by profiling builds on both OSes.

| Requirement | Minimum | Recommended |
|---|---|---|
| OS | Windows 10 / 64-bit Linux | Windows 11 / current 64-bit Linux |
| CPU | i5-6400 / Ryzen 3 1200 | i5-8400 / Ryzen 5 3600 |
| GPU | GTX 960 / RX 470 (2 GB) | GTX 1060 / RX 580 (4 GB+) |
| RAM | 8 GB | 16 GB |
| Storage | 5 GB | 5 GB (SSD) |
| Graphics API | DX12 / Vulkan | DX12 / Vulkan |
| Input | Keyboard & mouse | Controller |

**Target Performance:** 60 FPS at 1080p (recommended), 60 FPS at 720p (minimum).

---

## 4. Level Design and World Structure

### 4.1 World Layout
The overall world/setting of the game will be a multi-floor dungeon. The dungeon is deep underground, and is shaped like an upside-down trapezoidal prism (floors get bigger and longer as the player and party progresses upward). 

| Level / Area | Description | Key Features |
|---|---|---|
| Main dungeon space | Any space that is not a cell or safe spot on a given floor | This is where enemies will spawn into (mobs, miniboss, floor boss) |
| Cells | Cell to house prisoners | Can include loot, NPC interaction (where recruiting party members can take place, as well as basic dialogue with other NPCs) |
| Safe spots | Small, guard-free rooms tucked into each floor (old storage rooms, collapsed cells) | Save points, healing, party management (upgrades, leader/order changes), and optional dialogue with allies |

**Example Floor Layout:**

### 4.2 Environmental Features
- Terrain: Can be setup in a way to challenge player movement; there will also be pillars that can be broken in a way to have floors collapse 
- Buildings / Structures: 
- Interactive Elements: doors, switches for certain gates or doors, pickups like items or keys
- Hazards: falling platforms (collapsed floors), enemy altered terrain 

### 4.3 Flow and Pacing
As the game progresses, the game will become harder in a roughly linear manner. Floors progressively challenge player mechanics and strategy with enemies growing in power, and skills. 

Example Difficulty Curve:
```
Difficulty

   ^
   |
   |                                                                ^
   |                                                        _______/
   |                                                      /
   |                                                     /
   |                                                    /
   |                                            _______/
   |                                    _______/
   |                                  /
   |                         ________/
   |                        /
   |              /\______ /     
   |      /\     /  
   |  ___/  \___/
   +--------------------------------------------------------------------> Time
    Tutorial  Floor 1   Floor 2   Floor 3   Floor 4   Floor 5   Escape
```

---

## 5. Visual and Audio Design

### 5.1 Art Style
- **Graphical Style:** 2d, 16-bit spirtes
- **Color Palette:** Grey, brown, black, orange, yellow, earthy
- **Lighting & Mood:** Darker and broody 
- **Influences / References:** *Persona* series, *The Mageseeker: A League of Legends Story*, *The Legend of Zelda: Majora's Mask*, *Slay the Spire 1 & 2*, *Genshin Impact*, *Puzzle & Dragons*

### 5.2 Character Design

| Character | Appearance | Key Animations | Behavior / Personality |
|---|---|---|---|
| Sirius | Disheveled, long ivory hair, bandages covering torso, beat-up capri-like pants, barefoot. Red eyes.  | Idle, Running, Jump, Attack, Death, leader, retribution, grounded | Because player plays as Sirius, dialogue options will reflect player's interpretation of Sirius's personality, with choices ranging from pessimistic, optimistic, and neutral. Sirius at the end of the day is charismatic and good at rallying his allies together. Throughout the story, he is constantly juxtaposed with comparisons to a demon, though him nor his allies fully understand why; this in turn slowly makes him doubt his innocence. |
| Mal'Kholm Ecks | Tall, big, muscular, bald, and shirtless, with a brown, braided beard. Tan skin and yellow eyes. Big chains latches to both wrists, with a shunk of stone connected on each end of the chains. Short 5-inch inseam shorts, big brolic legs and barefoot | Idle, Running, Jump, Attack, Death, leader, retribution, grounded | He is a gentle, thoughtful, and patient man. Although portrayed as a bloodthirsty savage, he remains resolute in breaking out of the dungeon and reuniting with the revolution |
| Kalia | Short, black hair pixie cut. She is tall and lean. Green eyes. Dark skin complexion, she wears a cape over her baggy prison attire.  | Idle, Running, Jump, Attack, Death, leader, retribution, grounded | She is passionate, prideful, and bold in all regard. She deeply cares for her allies, and is the type to bleed for them. |
| Hanuboy | A small pika. They are shackled with a big leg iron at least three-times are big as he is. | Idle, Running, Jump, Attack, Death, leader, retribution, grounded | Very sassy. Often frustrated that his only means of communicating is through squeaks, despite fully understanding human language. |
| Basic guard | Basic, plated attire, similar to knights from the medieval times. All wield a basic broadsword. | Running, attack, death | No distinct personality other than to take down player's party. |
| Wardens | heavily armored in a rustic gold. Built similar to conventional 'juggernaught' archetypes. All wield a shield and heavy mace. | Walking, attack, death | No distinct personality other than to take down player's party. |
| Caster guards | Draped with a brown robe that covers everything but their glowing yellow eyes. All wield a magic staff. | Running, attack, death | Cowardly in nature, becomes confident when out of range of player's party. |
| Caster wardens | Draped with a navy robe, they encompass themselves in magic circles. Their face and body are also hidden, other than their gross grin. | Wind up, attack, death | They are maniacal and will anything to utterly defeat the player's party |
| Floor director (floor miniboss) | three types: physical damage based ones have a lot of armor, more than wardens, and ride rams into combat. Magic based ones are enamored with magic circles, a and robe that fully covers their sillhoutte. Hybrid ones (magic + physical) ride battle rams and are drenched in a magic circles and a cloak. | riding, wind up, attack, death | Almost robotic. No feelings and thoughts, just there to take down the player and party. |
| Hounds | Wolves who jaws are replaced with giant serrated ones. Hounds are black outside of their distinctly blue eyes and chrome/white jaws. | sprints, sniffing, attack, death | A ruthless beast, whos consumed only by bloodlust |
| Floor boss | In order, the bosses will have traditional goat, serpent, lion, dragon, and finally a chimera (the four come together and fuse) features, with them all having slight variations in design to reflect their abilities. | Idle, attack, death | All bosses are stoic, and calm. Their only goal is to serve their master (revealed to be S. Ravana) and will not stop at any cost.  |
| S. Ravana (formerly Sirius) | Clean, upkept. Slick back hair, with white-glowing tribal markings all over his body. His left arm is one composed of magic. Dressed in a robe. Their eyes are blindfolded, though their red eyes shine right through. Veiled by powerful wind magic. Despite wearing sandals, they only float. | Idle, attack, death | Arrogant, cunning, charismatic, and ruthless. Their former memories with their former allies breaking out of the dungeon does nothing but digusts them. |

### 5.3 Audio Design
- **Sound Effects:** Attacks (spells, physical attacks, buffs, debuffs), picking up items, using items, death, certain doors/gates, switches, retribution, grounding, clicks, more to come... 
- **Music:** Combat music and passive music throughout game aims to be opposites of each other, with calming ambient music during non-combat scenes, and much more intense and thrilling sonic landscape during combat.
- **Voice Acting:** No voice acting will be in this game. 
- **How Audio Enhances Gameplay:** Audio changes based on stage of game: default battle music, changes to enhanced battle music when mini-boss spawns, changes to floor boss music, and "lore" music plays when story progresses. Final fight will have it's own unique theme. Music in this game will try to invoke the feelings of the party. Sound effects will also be distinct, and almost jarring to the music, to better highlight it's desired effect.

---

## 6. Story and Narrative

### 6.1 Plot Summary
- **Setting:** Inside a 5 floor dungeon.
- **Main Plot:** 
- **Key Characters:** Sirius, Mal'Kholm Ecks, Kalia, Hanuboy

### 6.2 Objectives
- **Main Objective:** To break out of dungeon rooted in corruption and cruelty, and to change the system that is complacent in this mistreatment
- **Player Motivation:** 
- **Secondary Objectives:** Learn the mystery behind Sirius's identiy

### 6.3 Dialogue and Cutscenes
- **Dialogue System:** Visual novel text boxes with illustrated portraits and expressions. Sirius has dialogue choices (optimistic / neutral / pessimistic) that change ally reactions but not the main plot.
- **Cutscenes:** Short in-engine scenes made with Level Sequencer for major story beats; all other story moments use the dialogue system.

| Cutscene / Story Beat | Trigger | Content |
|---|---|---|
| Awakening | Game start | Sirius wakes in a cell at the bottom of the dungeon, missing an arm, with no memories |
| Recruitment scenes | Entering an ally's cell (Floors 1–3) | Each ally is introduced and joins the party |
| Boss intros | Entering a boss chamber | Each boss appears and pledges loyalty to "the master" |
| Memory flashes | Defeating each floor boss | Short, cryptic flashbacks from Sirius's past |
| The name | Floor 4 | The party learns the name S. Ravana. Clues from throughout the game should start to make sense for the party |
| The chimera | Floor 5 | The four bosses fuse into the chimera |
| Betrayal | Reaching the exit | Sirius's memories return; he becomes S. Ravana and turns on the party, and revives Chimera to fight the party  |
| Ending | Defeating S. Ravana + Chimera | The party escapes with Mal'Kholm Ecks and heads toward the reuniting with the revolution |

---

## 7. User Interface (UI) and HUD

### 7.1 UI Elements

**Menus:**
- **Main Menu:** New game, load game, settings, quit
- **Pause Menu:** Save, quit, settings
- **Settings Menu:** Audio, graphics, controls
- **Game Over / Victory Screen:** Rewards (exp gained, items, skills unlocked) <- if floor cleared, otherwise, a menacing game-over screen that gives the player an option to return to last save, restart floor, or quit to menu. 

**HUD:**
*'O' indicates overworld elements and 'T' indicates turn-based combat elements.*

| HUD Element | Screen Position | Information Shown |
|---|---|---|
| O-Party Health Bar | Above player party (hovers above heads regardless of position) | current health / total combined health |
| O-HUD bar | Bottom middle of screen | Character picture, skills & cooldowns, item equipped |
| O-Current character leading picture | Left side inside HUD bar | Picture of current leader of party |
| O-Ability / Ability cooldowns | right side inside HUD bar | skills equiped and cooldowns if used (picture of skill will be greyed out with timer until back up) |
| O-Half-circle mini-map | Top center of screen | portion of map party is in, mobs indicated by red marker |
| O-Grounded allies list | bottom left corner | Current health and resource bar |
| O-Retribution bar | Bottom right corner | Sequence order in character switches need to happen |
| T-Character health bars | hover above character position on screen | Current health / Total health |
| T-Character resource bars | hover above character position on screen, right below health bar | Current resource / Total resource |
| T-Hud bar | bottom middle of screen | options to open skill menu, block, or open item menu |
| T-retribution progress | bottom right of screen | Progression for retribution and next order sequence |

### 7.2 UX Design

UI aims to keep information crucial to player success compact and easy to glance at while in combat, and thorough and in-depth during turn-based combat. This allows for hack-n-slash sequences to be smooth and turn-based combat satisfying to players. HUD elements will also be customizable to players as they will be able to expand or reduce sizing to their liking. There will also be clear and distinct visual effects like damage numbers and small shakes (ability to toggle will be avaliable) to indiciate severity of damage, as well as status conditions having clear contrastable colors. Interactables will also be clearly highlighted on the map. 

For example red = damage, green = heals, blue = shields, purple/pink = debuffs.

---

## 8. Testing and Iteration Plan

### 8.1 Testing Strategy

| Test Type | Method | Frequency |
|---|---|---|
| Functional / Bug Testing | Test each feature in a small test map right after implementation | Every feature |
| Automated Tests | Unreal Automation Tests for C++ logic (damage math, status effects, Retribution bonuses, turn order and etc.) | Every build |
| Cross-Platform Builds | Package and run on both Windows 11 and Fedora | Weekly |
| Performance Testing | Unreal Insights, with max possible waves of enemies on screen | During every milestones |
| Playtesting | friends play a build and fill out a short feedback form | Three rounds that being the prototype, alpha, and beta stages |
| Balance Testing | Adjust stats and enemy waves in Data Tables based on the playtesting results | After each of the playtests |

Bug Tracking: Cna use GitHub Issues, which is labeled by severity and platform the Windows / Linux

### 8.2 Iteration Plan
After each playtest, feedback is sorted into bugs, balance, and design changes. Crashes and blockers are fixed first, then balance through data tables/sheets, then any design changes. If time runs short, content can always be cut before the systems, so the build always stays complete and playable.

**Scope Tiers:**
- **Must Have (MVP):** Floors 1–2 fully playable; Sirius and 2 allies; grounding, leader, and Retribution; 3 enemy types; 2 turn-based boss fights; dialogue system; HUD and menus; save/load
- Should Have: Floors 3–5, all the possible enemy types, the chimera fight, all 3 allies, full soundtrack
- Nice to Have: Final S. Ravana fight, dialogue choices that affect ally reactions, packaged Linux release

---

## 9. Project Timeline

### 9.1 Development Phases

| Phase | Dates | Tasks / Milestones | Deliverable |
|---|---|---|---|
| Setup & Prototyping | Sep 30 – Oct 14 | Repo & cross-platform setup, movement, real-time combat, one enemy with AI, grounding, using placeholder art | Playable gray-box floor (should be done by Oct 18) |
| Core Systems | Oct 14 – Nov 8 | Turn-based battle handler, Retribution implementation, leader, enemy waves, stat + EXP, dialogue, HUD | Alpha: Floor 1 complete (Nov 8) |
| Content | Nov 9 – Nov 25 | Floor 2+, more enemies and bosses, sprites, portraits, music | Beta: MVP content (Nov 25) |
| Polish & Testing | Nov 26 – Dec 7 | Playtest fixes, balance, VFX/SFX, optimization (will have a lighter load week over Thanksgiving just so I can enjoy some Turkey heheh) | Release candidate (Dec 7) |
| Final Build | Dec 10 – Dec 14 | Packaged Windows + Linux builds, final bug fixes, submission | Final build (Dec 14) |

**Playtest Rounds:** Oct 18 (prototype), Nov 8 (alpha), Nov 25 (beta)

### Task Allocation
Solo project; all programming, art, audio, and design by me, Lyons Tran B-)

## Final note
I know that this is a very ambitious project, and doing so by myself is certainly going to be hard given I've never engaged in extensive game development. However, I am confident in my abilities to finish most of my intended vision by the final deadline, December 14th. I have and extensive coding background, so I believe feature implementation and debugging is an area I'm very comfortable in, and any areas I'm lacking in, I am good at looking through documentation. More than that, I've had a deep appriciation of video games since I can remember, and given the freedom I have this semester (I have a very light load), I am planning on going all out into this project.

 I am very excited and I can't wait for you to monitor my progression, and try out my game when the time comes. 

---
