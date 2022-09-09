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