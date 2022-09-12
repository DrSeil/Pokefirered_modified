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

Route1_EventScript_TryGetPackage::
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 1
	giveitem_msg ViridianCity_Mart_Text_ReceivedOaksParcelFromClerk, ITEM_OAKS_PARCEL, 1, MUS_OBTAIN_KEY_ITEM
	closemessage
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 5
	end

Route1_Warp::
	warp MAP_ROUTE1, 255, 12, 34
	waitstate
	release
	end

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
