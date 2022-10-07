.equ LOCALID_MOM, 1

PalletTown_MomToOak::
	lock
	faceplayer
	goto_if_set FLAG_BEAT_RIVAL_IN_OAKS_LAB, PalletTown_PlayersHouse_1F_EventScript_MomHeal
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouFemale
	closemessage
	applymovement LOCALID_MOM, Common_Movement_FaceOriginalDirection
	waitmovement 0
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 1
	clearflag FLAG_HIDE_OAK_IN_HIS_LAB
	setvar VAR_MAP_SCENE_PALLET_TOWN_OAK, 1
	setflag FLAG_HIDE_OAK_IN_PALLET_TOWN
	setflag FLAG_DONT_TRANSITION_MUSIC
	warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, 12
	clearflag FLAG_0x0AF 
	waitstate
	return

PalletTown_FastStarterScene::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_PROF_OAK, PalletTown_ProfessorOaksLab_Movement_OakEnter
	waitmovement 0
	removeobject LOCALID_PROF_OAK
	setobjectxyperm LOCALID_PROF_OAK, 6, 3
	setobjectmovementtype LOCALID_PROF_OAK, MOVEMENT_TYPE_FACE_DOWN
	clearflag FLAG_HIDE_OAK_IN_HIS_LAB
	applymovement OBJ_EVENT_ID_PLAYER, PalletTown_ProfessorOaksLab_Movement_PlayerEnter
	waitmovement 0
	clearflag FLAG_DONT_TRANSITION_MUSIC
	savebgm MUS_DUMMY
	fadedefaultbgm
	msgbox PalletTown_Text_ChatPicked
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 2
	releaseall
	end

PalletTown_Text_ChatPicked::
	.string "Hope chat picked correctly!$"

PalletTown_ReturnPackage::
	message PalletTown_ProfessorOaksLab_Text_DeliveredOaksParcel
	waitmessage
	removeitem ITEM_OAKS_PARCEL
	message PalletTown_ProfessorOaksLab_Text_ReceivedPokedexFromOak
	waitmessage
	setflag FLAG_SYS_POKEDEX_GET
	special SetUnlockedPokedexFlags
	setvar VAR_MAP_SCENE_POKEMON_CENTER_TEALA, 1
	giveitem_msg PalletTown_ProfessorOaksLab_Text_ReceivedFivePokeBalls, ITEM_POKE_BALL, 5
	famechecker FAMECHECKER_OAK, 1
	closemessage
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 2
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 1
	setvar VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 1
	setvar VAR_MAP_SCENE_ROUTE22, 1
	release
	end

EventScript_NameStarterIllegalRun::
	setvar VAR_0x8004, 0
	special NicknameIllegalRun
	goto PalletTown_ProfessorOaksLab_EventScript_RivalPicksStarter
	end

Route1_EventScript_TryGetPackage::
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 1
	giveitem_msg Route16_House_Text_ReceivedHM02FromGirl, ITEM_HM02

	giveitem_msg ViridianCity_Mart_Text_ReceivedOaksParcelFromClerk, ITEM_OAKS_PARCEL, 1, MUS_OBTAIN_KEY_ITEM
	closemessage
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 5
	end

Route1_Warp::
	warp MAP_ROUTE1, 255, 12, 34
	waitstate
	release
	end

ViridianCity_EventScript_DoRandomTutorialBattle::
	msgbox ViridianCity_Text_ShowYouHowToCatchMons
	closemessage
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	random 2
	copyvar VAR_0x8008, VAR_RESULT
	special StartOldManTutorialBattle
	waitstate
	lock
	faceplayer
	call_if_eq VAR_0x8008, 0, ViridianCity_EventScript_Educational
	call_if_eq VAR_0x8008, 1, ViridanCity_EventScript_HmmmYouGetTheIdea
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2
	giveitem ITEM_TEACHY_TV
	msgbox ViridianCity_Text_WatchThatToLearnBasics
	return

ViridianCity_EventScript_Educational::
	msgbox ViridianCity_Text_ThatWasEducationalTakeThis
	return

ViridanCity_EventScript_HmmmYouGetTheIdea::
	msgbox ViridianCity_Text_HmmYouGetTheIdea
	return

ViridianCity_Text_HmmYouGetTheIdea::
    .string "Hmmm... Well you get the idea!\n"
    .string "And here, take this, too.$"


EventScript_ReuseRepel::
	msgbox Text_RepelWoreOffUseAnother, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_TryUseRepel
	closemessage
	end

EventScript_TryUseRepel::
	removeitem ITEM_MAX_REPEL 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_FinishUseMaxRepel
	removeitem ITEM_SUPER_REPEL 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_FinishUseSuperRepel
	removeitem ITEM_REPEL 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_FinishUseRepel
	msgbox Text_OutOfRepels, MSGBOX_SIGN
	closemessage
	end
	
EventScript_FinishUseMaxRepel::
	setvar VAR_REPEL_STEP_COUNT, 250
	msgbox Text_UsedMaxRepel, MSGBOX_SIGN
	closemessage
	end

EventScript_FinishUseSuperRepel::
	setvar VAR_REPEL_STEP_COUNT, 200
	msgbox Text_UsedSuperRepel, MSGBOX_SIGN
	closemessage
	end

EventScript_FinishUseRepel::
	setvar VAR_REPEL_STEP_COUNT, 100
	msgbox Text_UsedRepel, MSGBOX_SIGN
	closemessage
	end

Text_RepelWoreOffUseAnother::
	.string "REPEL's effect wore off…\n"
	.string "Use another?$"

Text_OutOfRepels::
	.string "You are out of REPELs!$"

Text_UsedRepel::
    .string "A REPEL was used.$"

Text_UsedSuperRepel::
    .string "A SUPER REPEL was used.$"

Text_UsedMaxRepel::
    .string "A MAX REPEL was used.$"

kaizo_PokemonLeague_LoreleisRoom_Text_Intro::
    .string "Welcome to the IRONMON LEAGUE.\p"
    .string "As you know, I am LADYPOO, your wife.\p"
    .string "No one can best me when it comes\n"
    .string "to IRONMON battles!\p"
    .string "I've been practicing.\p"
    .string "You've received enough healing snacks,\n"
    .string "now taste DEFEAT!\p"
    .string "Hahaha!\n"
    .string "Are you ready?{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_LoreleisRoom_Text_RematchIntro::
    .string "Welcome to the IRONMON LEAGUE.\p"
    .string "As you know, I am LADYPOO, your wife.\p"
    .string "No one can best me when it comes\n"
    .string "to IRONMON battles!\p"
    .string "I've been practicing.\p"
    .string "You've received enough healing snacks,\n"
    .string "now taste DEFEAT!\p"
    .string "Hahaha!\n"
    .string "Are you ready?{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_LoreleisRoom_Text_Defeat::
    .string "…Make your own dinner!$"

kaizo_PokemonLeague_LoreleisRoom_Text_PostBattle::
    .string "You're better than I thought.\n"
    .string "Go on ahead or...\p"
    .string "If you've been streaming a long time\n"
    .string "your family misses you.$"

kaizo_PokemonLeague_BrunosRoom_Text_Intro::
    .string "I am LASERBELCH of the ELITE FOUR!\p"
    .string "Through rigorous training, people\n"
    .string "and POKéMON can become stronger\l"
    .string "without limit.\p"
    .string "Do you see this headband? See these\n"
    .string "GUNS!? Calendar worthy!\l"
    .string "And that will never change!\p"
    .string "{PLAYER}!\p"
    .string "Wanna get ripped? Get with\n"
    .string "that grind!\p"
    .string "Let's Go!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_BrunosRoom_Text_RematchIntro::
    .string "I am LASERBELCH of the ELITE FOUR!\p"
    .string "Through rigorous training, people\n"
    .string "and POKéMON can become stronger\l"
    .string "without limit.\p"
    .string "Do you see this headband? See these\n"
    .string "GUNS!? Calendar worthy!\l"
    .string "And that will never change!\p"
    .string "{PLAYER}!\p"
    .string "Wanna get ripped? Get with\n"
    .string "that grind!\p"
    .string "Let's Go!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_BrunosRoom_Text_Defeat::
    .string "What? All these gains…\n"
    .string "How could I lose?$"

kaizo_PokemonLeague_BrunosRoom_Text_PostBattle::
    .string "Gotta get some more reps in.\n"
    .string "Go face your next challenge.$"

kaizo_PokemonLeague_AgathasRoom_Text_Intro::
    .string "I am SKYBILZ of the ELITE FOUR.\p"
    .string "I hear OAK's taken a lot of\n"
    .string "interest in your mom.\p"
    .string "You might as well go home\n"
    .string "and meet your new dad\p"
    .string "because you're not going to beat me!\n"
    .string "I've already cleared this CHALLENGE!\p"
    .string "Then decided to do it again…\n"
    .string "for FUN!\p"
    .string "What hope of victory do you have?\n"
    .string "Let's get you back to the lab already\p"
    .string "{PLAYER}! Repping Oxygen Esports,\n"
    .string "let's get this over with!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_AgathasRoom_Text_RematchIntro::
    .string "I am SKYBILZ of the ELITE FOUR.\p"
    .string "I hear OAK's taken a lot of\n"
    .string "interest in your mom.\p"
    .string "You might as well go home\n"
    .string "and meet your new dad\p"
    .string "because you're not going to beat me!\n"
    .string "I've already cleared this challenge!\p"
    .string "Then decided to do it again…\n"
    .string "for FUN!\p"
    .string "What hope of victory do you have?\n"
    .string "Let's get you back to the lab already\p"
    .string "{PLAYER}! Repping Oxygen Esports,\n"
    .string "let's get this over with!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_AgathasRoom_Text_Defeat::
    .string "Well, I'll be!\n"
    .string "Maybe you do have what it takes!$"

kaizo_PokemonLeague_AgathasRoom_Text_PostBattle::
    .string "You've won!\p"
    .string "But you've got more\n"
    .string "battles ahead.\p"
    .string "If you don't have any cute otter\n"
    .string "or raccoon videos, get outta my sight!$"

kaizo_PokemonLeague_LancesRoom_Text_Intro::
    .string "Ah! Another one, huh? Your name is…\n"
    .string "{PLAYER}?\p"
    .string "I lead the ELITE FOUR.\p"
    .string "But more importantly, I'm the reason you're here\n"
    .string "TRAINER.\p"
    .string "I am IATEYOURPIE, creator of\n"
    .string "the IRONMON CHALLENGE!\p"
    .string "It's hard to beat,\n"
    .string "but you managed to make it here.\p"
    .string "Too bad your journey is\n"
    .string "about to come to a painful end.\p"
    .string "Well, are you ready to lose?\p"
    .string "Your IRONMON CHALLENGE ends w- wait\n"
    .string "My PIZZA! Not again!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_LancesRoom_Text_RematchIntro::
    .string "Ah! Back again, huh\n"
    .string "{PLAYER}?\p"
    .string "I lead the ELITE FOUR.\p"
    .string "But more importantly, I'm the reason you're here\n"
    .string "TRAINER.\p"
    .string "I am IATEYOURPIE, creator of\n"
    .string "the IRONMON CHALLENGE!\p"
    .string "It's hard to beat,\n"
    .string "but you managed to make it here.\p"
    .string "Too bad your journey is\n"
    .string "about to come to a painful end.\p"
    .string "Well, are you ready to lose?\p"
    .string "Your IRONMON CHALLENGE ends w- wait\n"
    .string "My PIZZA! Not again!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

kaizo_PokemonLeague_LancesRoom_Text_Defeat::
    .string "Oh my gosh!\p"
    .string "Do you think there's any way to\n"
    .string "salvage this PIZZA?$"

kaizo_PokemonLeague_LancesRoom_Text_PostBattle::
    .string "Look, I've gotta go put this\n"
    .string "burned PIZZA in the trash, {PLAYER}.\p"
    .string "but I guess you're the IRONMON\n"
    .string "CHAMPION now!\p"
    .string "…Or, you would have been, but\n"
    .string "someone actually beat you to it.\p"
    .string "There is one more TRAINER to face!\n"
    .string "His name is…\p"
    .string "{RIVAL}!\p"
    .string "He beat the ELITE FOUR before\n"
    .string "you. Somehow. Time dilation?\p"
    .string "He is the real IRONMON\n"
    .string "CHAMPION.$"

kaizo_PokemonLeague_ChampionsRoom_Text_Intro::
    .string "{RIVAL}: Hey, {PLAYER}!\p"
    .string "Was starting to wonder if you'd\n"
    .string "ever show up, {PLAYER}!\p"
    .string "My rival should be strong to keep\n"
    .string "me sharp.\p"
    .string "While building up my TWITCH, I\n"
    .string "looked all over for RIVALS.\p"
    .string "Since you're the only one who responded,\n"
    .string "that means it was meant to be you!\p"
    .string "And now that\p"
    .string "I'm the IRONMON CHAMPION!\p"
    .string "{PLAYER}!\n"
    .string "Do you know what that means?\p"
    .string "I'll tell you.\p"
    .string "I am the most powerful STREAMER in\n"
    .string "the world!$"

kaizo_PokemonLeague_ChampionsRoom_Text_RematchIntro::
    .string "{RIVAL}: Hey, {PLAYER}!\p"
    .string "Was starting to wonder if you'd\n"
    .string "ever show up, {PLAYER}!\p"
    .string "My rival should be strong to keep\n"
    .string "me sharp.\p"
    .string "While building up my TWITCH, I\n"
    .string "looked all over for RIVALS.\p"
    .string "Since you're the only one who responded,\n"
    .string "that means it was meant to be you!\p"
    .string "And now that\p"
    .string "I'm the IRONMON CHAMPION!\p"
    .string "{PLAYER}!\n"
    .string "Do you know what that means?\p"
    .string "I'll tell you.\p"
    .string "I am the most powerful STREAMER in\n"
    .string "the world!$"

kaizo_PokemonLeague_ChampionsRoom_Text_Defeat::
    .string "NO!\n"
    .string "That can't be!\l"
    .string "You beat me at my best!\p"
    .string "After all that work to become the \n"
    .string "VERY BEST? Like no one ever was?\p"
    .string "My reign is over already?\n"
    .string "It's not fair!$"

kaizo_PokemonLeague_ChampionsRoom_Text_Victory::
    .string "はーはッ！\n"
    .string "かった！　かった！　かった！\p"
    .string "{PLAYER}に　まける　ような\n"
    .string "おれさま　では　なーい！\p"
    .string "ま！　ポケモンの\n"
    .string "てんさい　{RIVAL}さま　あいてに\l"
    .string "ここまで　よく　がんばった！\p"
    .string "ほめて　つかわす！\n"
    .string "はーッ！　はーはッはッ！$"

kaizo_PokemonLeague_ChampionsRoom_Text_PostBattle::
    .string "Why?\n"
    .string "Why did I lose?\p"
    .string "I never made any mistakes raising\n"
    .string "my POKéMON…\p"
    .string "Darn it! You're the newest\n"
    .string "IRONMON CHAMPION!\p"
    .string "Although I don't like to admit it…$"

kaizo_PokemonLeague_ChampionsRoom_Text_OakPlayer::
    .string "OAK: {PLAYER}!$"

kaizo_PokemonLeague_ChampionsRoom_Text_OakCongratulations::
    .string "OAK: So, you've won!\n"
    .string "Sincerely, congratulations!\p"
    .string "You're the new IRONMON CHAMPION!\n"
    .string "Honestly, I didn't think it was possible.\p"
    .string "You've grown up so much since you first\n"
    .string "left the LAB with {STR_VAR_1} at the start\l"
    .string "of this IRONMON CHALLENGE seed.\p"
    .string "{PLAYER}, you have really done it!$"

kaizo_PokemonLeague_ChampionsRoom_Text_OakImDisappointedRival::
    .string "OAK: {RIVAL}…\n"
    .string "I'm disappointed in you.\p"
    .string "I left {PLAYER}'s Mom's house when I\n"
    .string "heard you'd beaten the ELITE FOUR.\p"
    .string "But, when I got here, you had\n"
    .string "already lost!\p"
    .string "{RIVAL}, do you understand why\n"
    .string "that's upsetting to me?\p"
    .string "You have forgotten that I'm the sole\n"
    .string "patriarch of PALLET TOWN.\p"
    .string "Without me, the town will never\n"
    .string "produce CHILDREN again!$"

kaizo_PokemonLeague_ChampionsRoom_Text_OakPlayerComeWithMe::
    .string "OAK: {PLAYER}.\p"
    .string "You understand that your victory\n"
    .string "was not just your own doing.\p"
    .string "The bond you share with your\n"
    .string "Chat is marvelous.\p"
    .string "{PLAYER}!\n"
    .string "Come with me!$"

kaizo_PokemonLeague_HallOfFame_Text_OakCongratulations::
    .string "OAK: Er-hem!\n"
    .string "Congratulations, {PLAYER}!\p"
    .string "This floor is the IRONMON HALL OF\n"
    .string "FAME.\p"
    .string "IRONMON LEAGUE CHAMPIONS are\n"
    .string "honored for their exploits here.\p"
    .string "Their POKéMON are also recorded in\n"
    .string "the HALL OF FAME.\p"
    .string "{PLAYER}!\p"
    .string "You have worked hard to become\n"
    .string "the new PEOPLE'S CHAMPION.\p"
    .string "Congratulations, {PLAYER}, you and\n"
    .string "your POKéMON are HALL OF FAMERS!$"

kaizo_Route25_SeaCottage_Text_ImBillHelpMeOutPal::
    .string "I turned myself into a MARIO!\n"
    .string "Boom! Big reveal: I'm a MARIO.\p"
    .string "What do you think about that?\n"
    .string "I turned myself into a MARIO!\p"
    .string "W-what are you just staring at me for?\n"
    .string "I turned myself into a MARIO!\p"
    .string "I'm MARIO BILL!\p"
    .string "This reference is tired and I think\n"
    .string "I'm actually over it…\p"
    .string "So, how about it?\n"
    .string "Help me out here!$"

kaizo_Route25_SeaCottage_Text_ImBillHelpMeOutLady::
    .string "I turned myself into a MARIO!\n"
    .string "Boom! Big reveal: I'm a MARIO.\p"
    .string "What do you think about that?\n"
    .string "I turned myself into a MARIO!\p"
    .string "W-what are you just staring at me for?\n"
    .string "I turned myself into a MARIO!\p"
    .string "I'm MARIO BILL!\p"
    .string "This reference is tired and I think\n"
    .string "I'm actually over it…\p"
    .string "So, how about it?\n"
    .string "Help me out here!$"

kaizo_Route25_SeaCottage_Text_RunCellSeparationOnPC::
    .string "Wait till I get inside the\n"
    .string "MARIO UN-MAKER.\p"
    .string "When I do, go to my PC and run\n"
    .string "the No Mo' Bros Protocol.$"

kaizo_Route25_SeaCottage_Text_NoPleaseChief::
    .string "No!?\n"
    .string "Now don't be so cold!\p"
    .string "Come on, you gotta help a guy in\n"
    .string "deep, deep trouble!\p"
    .string "What do you say, chief?\n"
    .string "Please?\l"
    .string "Okay?\l"
    .string "All right!$"

kaizo_Route25_SeaCottage_Text_NoPleaseBeautiful::
    .string "No!?\n"
    .string "Now don't be so cold!\p"
    .string "Come on, you gotta help a guy in\n"
    .string "deep, deep trouble!\p"
    .string "What do you say, beautiful?\n"
    .string "Please?\l"
    .string "Okay?\l"
    .string "All right!$"

kaizo_Route25_SeaCottage_Text_ThanksBudTakeThis::
    .string "BILL: Yeehah!\n"
    .string "Thanks, bud! I owe you one!\p"
    .string "So, did you come to see my\n"
    .string "POKéMON collection?\p"
    .string "You didn't?\n"
    .string "That's a bummer.\p"
    .string "I've got to thank you…\n"
    .string "Oh, here, maybe this'll do.$"

kaizo_Route25_SeaCottage_Text_ThanksLadyTakeThis::
    .string "BILL: Yeehah!\n"
    .string "Thanks, lady! I owe you one!\p"
    .string "So, did you come to see my\n"
    .string "POKéMON collection?\p"
    .string "You didn't?\n"
    .string "That's a bummer.\p"
    .string "I've got to thank you…\n"
    .string "Oh, here, maybe this'll do.$"

kaizo_Route25_SeaCottage_Text_ReceivedSSTicketFromBill::
    .string "{PLAYER} received an S.S. TICKET\n"
    .string "from BILL.$"

kaizo_Route25_SeaCottage_Text_YouveGotTooMuchStuff::
    .string "You've got too much stuff!$"

kaizo_Route25_SeaCottage_Text_SSAnnePartyYouGoInstead::
    .string "That cruise ship S.S. ANNE is in\n"
    .string "VERMILION CITY.\p"
    .string "I hear there are lots of TRAINERS\n"
    .string "on board, too.\p"
    .string "They invited me to their party, but\n"
    .string "I can't stand fancy do's.\p"
    .string "Why don't you go instead of me?\n"
    .string "Go on and have a good time.$"

kaizo_Route25_SeaCottage_Text_CheckOutRareMonsOnPC::
    .string "BILL: Feel like checking out some\n"
    .string "of my rare POKéMON on my PC?\p"
    .string "Go on, check out my PC.$"

kaizo_Route25_SeaCottage_Text_TeleporterIsDisplayed::
    .string "MARIO UN-MAKER is displayed on the PC\n"
    .string "monitor.$"

kaizo_Route25_SeaCottage_Text_InitiatedTeleportersCellSeparator::
    .string "{PLAYER} initiated the MARIO UN-MAKER's\n"
    .string "No Mo' Bros Protocol.$"

kaizo_Route25_SeaCottage_Text_BillsFavoriteMonList::
    .string "BILL's favorite POKéMON list!$"

kaizo_Route25_SeaCottage_Text_SeeWhichMon::
    .string "Which POKéMON do you want to see?$"

kaizo_chad_text::
	.string "How many times do I have to\n"
	.string "teach you this lesson old man.$"

kaizo_Text_WelcomeWantToHealPkmn::
    .string "Welcome to our Healing KITCHEN!\p"
    .string "Rough journey? Y'all hungry?\n"
    .string "I've got NUGS and mini TACOS!$"

kaizo_Text_TakeYourPkmnForFewSeconds::
    .string "Okay, I'll take your POKéMON for a\n"
    .string "few seconds. Eat up!$"

kaizo_Text_WeHopeToSeeYouAgain::
    .string "Please don't stream too long!$"

kaizo_Text_RestoredPkmnToFullHealth::
    .string "Thank you for waiting.\n"
    .string "We've restored you both to\l"
    .string "full nourishment.$"

