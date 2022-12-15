#include "global.h"
#include "gflib.h"
#include "data.h"
#include "decompress.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "graphics.h"
#include "help_system.h"
#include "item.h"
#include "item_menu.h"
#include "mail_data.h"
#include "menu.h"
#include "naming_screen.h"
#include "new_menu_helpers.h"
#include "pc_screen_effect.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system_internal.h"
#include "pokemon_summary_screen.h"
#include "quest_log.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "tilemap_util.h"
#include "trig.h"
#include "constants/items.h"
#include "constants/help_system.h"
#include "constants/songs.h"
#include "random.h"

EWRAM_DATA struct PokemonStorageSystemData *gStorage = NULL;
static EWRAM_DATA bool8 sInPartyMenu = 0;
static EWRAM_DATA u8 sCurrentBoxOption = 0;
static EWRAM_DATA u8 sDepositBoxId = 0;
static EWRAM_DATA u8 sWhichToReshow = 0;
static EWRAM_DATA u8 sLastUsedBox = 0;
static EWRAM_DATA u8 sReleaseText = 0;
static EWRAM_DATA u8 sReleasedelay = 0;
static EWRAM_DATA u8 sPivoted = 0;
static EWRAM_DATA u16 sMovingItemId = ITEM_NONE;

static void Task_InitPokeStorage(u8 taskId);
static void Task_ShowPokeStorage(u8 taskId);
static void Task_ReshowPokeStorage(u8 taskId);
static void Task_PokeStorageMain(u8 taskId);
static void Task_ShowPartyPokemon(u8 taskId);
static void Task_HidePartyPokemon(u8 taskId);
static void Task_OnSelectedMon(u8 taskId);
static void Task_MoveMon(u8 taskId);
static void Task_PlaceMon(u8 taskId);
static void Task_ShiftMon(u8 taskId);
static void Task_WithdrawMon(u8 taskId);
static void Task_DepositMenu(u8 taskId);
static void Task_ReleaseMon(u8 taskId);
static void Task_ShowMarkMenu(u8 taskId);
static void Task_TakeItemForMoving(u8 taskId);
static void Task_GiveMovingItemToMon(u8 taskId);
static void Task_ItemToBag(u8 taskId);
static void Task_SwitchSelectedItem(u8 taskId);
static void Task_ShowItemInfo(u8 taskId);
static void Task_HandleMovingMonFromParty(u8 taskId);
static void Task_PrintCantStoreMail(u8 taskId);
static void Task_HandleBoxOptions(u8 taskId);
static void Task_HandleWallpapers(u8 taskId);
static void Task_JumpBox(u8 taskId);
static void Task_NameBox(u8 taskId);
static void Task_ShowMonSummary(u8 taskId);
static void Task_GiveItemFromBag(u8 taskId);
static void Task_OnCloseBoxPressed(u8 taskId);
static void Task_OnBPressed(u8 taskId);
static void Task_ChangeScreen(u8 taskId);
static void GiveChosenBagItem(void);
static void FreePokeStorageData(void);
static void SetScrollingBackground(void);
static void ScrollBackground(void);
static void LoadPokeStorageMenuGfx(void);
static bool8 InitPokeStorageWindows(void);
static void LoadsMiscSpritePalette(void);
static void InitPalettesAndSprites(void);
static void CreateMarkingComboSprite(void);
static void CreateWaveformSprites(void);
static void RefreshDisplayMonData(void);
static void StartDisplayMonMosaic(void);
static void SpriteCB_DisplayMonMosaic(struct Sprite *sprite);
static bool8 IsDisplayMonMosaicActive(void);
static void CreateDisplayMonSprite(void);
static void LoadDisplayMonGfx(u16 species, u32 personality);
static void PrintDisplayMonInfo(void);
static void UpdateWaveformAnimation(void);
static void InitSupplementalTilemaps(void);
static void SetUpHidePartyMenu(void);
static bool8 HidePartyMenu(void);
static void UpdateCloseBoxButtonTilemap(bool8 normal);
static void StartFlashingCloseBoxButton(void);
static void StopFlashingCloseBoxButton(void);
static void UpdateCloseBoxButtonFlash(void);
static void SetPartySlotTilemaps(void);
static void SetPartySlotTilemap(u8 pos, bool8 isPartyMon);
static void UpdatePartySlotColors(void);
static void SetUpDoShowPartyMenu(void);
static bool8 DoShowPartyMenu(void);
static void InitPokeStorageBg0(void);
static void PrintStorageMessage(u8 textId);
static void PrintReleaseMessage(u8 textId);
static void ShowYesNoWindow(s8 cursorPos);
static void ClearBottomWindow(void);
static void AddWallpaperSetsMenu(void);
static void AddWallpapersMenu(u8 wallpaperSet);
static void InitCursorItemIcon(void);
static void SetPokeStorageQuestLogEvent(u8 species);
static void UpdateBoxToSendMons(void);

enum {
    TILEMAP_PKMN_DATA, // The "Pkmn Data" text at the top of the display
    TILEMAP_PARTY_MENU,
    TILEMAP_CLOSE_BUTTON,
    TILEMAP_COUNT
};

enum
{
    SCREEN_CHANGE_EXIT_BOX,
    SCREEN_CHANGE_SUMMARY_SCREEN,
    SCREEN_CHANGE_NAME_BOX,
    SCREEN_CHANGE_ITEM_FROM_BAG,
};

enum
{
    MSG_EXIT_BOX,
    MSG_WHAT_YOU_DO,
    MSG_PICK_A_THEME,
    MSG_PICK_A_WALLPAPER,
    MSG_IS_SELECTED,
    MSG_JUMP_TO_WHICH_BOX,
    MSG_DEPOSIT_IN_WHICH_BOX,
    MSG_WAS_DEPOSITED,
    MSG_BOX_IS_FULL,
    MSG_RELEASE_POKE,
    MSG_WAS_RELEASED,
    MSG_BYE_BYE,
    MSG_MARK_POKE,
    MSG_LAST_POKE,
    MSG_PARTY_FULL,
    MSG_HOLDING_POKE,
    MSG_WHICH_ONE_WILL_TAKE,
    MSG_CANT_RELEASE_EGG,
    MSG_CONTINUE_BOX,
    MSG_CAME_BACK,
    MSG_WORRIED,
    MSG_SURPRISE,
    MSG_PLEASE_REMOVE_MAIL,
    MSG_IS_SELECTED2,
    MSG_GIVE_TO_MON,
    MSG_PLACED_IN_BAG,
    MSG_BAG_FULL,
    MSG_PUT_IN_BAG,
    MSG_ITEM_IS_HELD,
    MSG_CHANGED_TO_ITEM,
    MSG_CANT_STORE_MAIL,
};

enum
{
    MSG_FMT_NONE,
    MSG_FMT_MON_NAME_1,
    MSG_FMT_MON_NAME_2,
    MSG_FMT_MON_NAME_3,
    MSG_FMT_RELEASE_MON_1,
    MSG_FMT_RELEASE_MON_2,
    MSG_FMT_RELEASE_MON_3,
    MSG_FMT_ITEM_NAME,
};

static const u32 sScrollingBg_Gfx[]     = INCBIN_U32("graphics/pokemon_storage/scrolling_bg.4bpp.lz");
static const u32 sScrollingBg_Tilemap[] = INCBIN_U32("graphics/pokemon_storage/scrolling_bg.bin.lz");

// Unused
static const u16 sMenu_Pal[] = INCBIN_U16("graphics/pokemon_storage/menu.gbapal");

static const u32 sMenu_Tilemap[]             = INCBIN_U32("graphics/pokemon_storage/menu.bin.lz");
static const u16 sPkmnData_Tilemap[]         = INCBIN_U16("graphics/pokemon_storage/pkmn_data.bin");
static const u16 sScrollingBg_Pal[]          = INCBIN_U16("graphics/pokemon_storage/scrolling_bg.gbapal");
static const u16 sScrollingBgMoveItems_Pal[] = INCBIN_U16("graphics/pokemon_storage/scrolling_bg_move_items.gbapal");
static const u16 sCloseBoxButton_Tilemap[]   = INCBIN_U16("graphics/pokemon_storage/close_box_button.bin");
static const u16 sPartySlotFilled_Tilemap[]  = INCBIN_U16("graphics/pokemon_storage/party_slot_filled.bin");
static const u16 sPartySlotEmpty_Tilemap[]   = INCBIN_U16("graphics/pokemon_storage/party_slot_empty.bin");
static const u16 sPokeStorageMisc2Pal[]      = INCBIN_U16("graphics/pokemon_storage/misc2.gbapal");
static const u16 sWaveform_Gfx[]             = INCBIN_U16("graphics/pokemon_storage/waveform.4bpp");

// Unused
static const u16 sUnused_Pal[] = INCBIN_U16("graphics/pokemon_storage/unused.gbapal");

static const u16 sItemInfoFrame_Pal[] = INCBIN_U16("graphics/pokemon_storage/item_info_frame.gbapal");

static const struct WindowTemplate sWindowTemplates[] = {
    {
        .bg = 1,
        .tilemapLeft = 0,
        .tilemapTop = 11,
        .width = 9,
        .height = 7,
        .paletteNum = 3,
        .baseBlock = 0x0c0
    }, {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 17,
        .width = 18,
        .height = 2,
        .paletteNum = 13,
        .baseBlock = 0x014
    }, {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 12,
        .width = 25,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 0x014
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgTemplates[] = {
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0x000
    }, {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0x100
    }, {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 27,
        .screenSize = 1,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0x000
    }, {
        .bg = 3,
        .charBaseIndex = 3,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0x000
    }
};

static const struct SpritePalette sMiscSpritePalette = {
    sPokeStorageMisc2Pal, PALTAG_MISC_2
};

static const struct SpriteSheet sWaveformSpriteSheet = {
    sWaveform_Gfx, 0x01C0, GFXTAG_WAVEFORM
};

static const struct OamData sOamData_DisplayMon;

static const struct SpriteTemplate sSpriteTemplate_DisplayMon = {
    .tileTag = GFXTAG_DISPLAY_MON,
    .paletteTag = PALTAG_DISPLAY_MON,
    .oam = &sOamData_DisplayMon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct StorageMessage sMessages[] = {
    [MSG_EXIT_BOX]             = {gText_ExitFromBox,             MSG_FMT_NONE},
    [MSG_WHAT_YOU_DO]          = {gText_WhatDoYouWantToDo,       MSG_FMT_NONE},
    [MSG_PICK_A_THEME]         = {gText_PleasePickATheme,        MSG_FMT_NONE},
    [MSG_PICK_A_WALLPAPER]     = {gText_PickTheWallpaper,        MSG_FMT_NONE},
    [MSG_IS_SELECTED]          = {gText_PkmnIsSelected,          MSG_FMT_MON_NAME_1},
    [MSG_JUMP_TO_WHICH_BOX]    = {gText_JumpToWhichBox,          MSG_FMT_NONE},
    [MSG_DEPOSIT_IN_WHICH_BOX] = {gText_DepositInWhichBox,       MSG_FMT_NONE},
    [MSG_WAS_DEPOSITED]        = {gText_PkmnWasDeposited,        MSG_FMT_MON_NAME_1},
    [MSG_BOX_IS_FULL]          = {gText_BoxIsFull2,              MSG_FMT_NONE},
    [MSG_RELEASE_POKE]         = {gText_ReleaseThisPokemon,      MSG_FMT_NONE},
    [MSG_WAS_RELEASED]         = {gText_PkmnWasReleased,         MSG_FMT_RELEASE_MON_1},
    [MSG_BYE_BYE]              = {gText_ByeByePkmn,              MSG_FMT_RELEASE_MON_3},
    [MSG_MARK_POKE]            = {gText_MarkYourPkmn,            MSG_FMT_NONE},
    [MSG_LAST_POKE]            = {gText_ThatsYourLastPkmn,       MSG_FMT_NONE},
    [MSG_PARTY_FULL]           = {gText_YourPartysFull,          MSG_FMT_NONE},
    [MSG_HOLDING_POKE]         = {gText_YoureHoldingAPkmn,       MSG_FMT_NONE},
    [MSG_WHICH_ONE_WILL_TAKE]  = {gText_WhichOneWillYouTake,     MSG_FMT_NONE},
    [MSG_CANT_RELEASE_EGG]     = {gText_YouCantReleaseAnEgg,     MSG_FMT_NONE},
    [MSG_CONTINUE_BOX]         = {gText_ContinueBoxOperations,   MSG_FMT_NONE},
    [MSG_CAME_BACK]            = {gText_PkmnCameBack,            MSG_FMT_MON_NAME_1},
    [MSG_WORRIED]              = {gText_WasItWorriedAboutYou,    MSG_FMT_NONE},
    [MSG_SURPRISE]             = {gText_FourEllipsesExclamation, MSG_FMT_NONE},
    [MSG_PLEASE_REMOVE_MAIL]   = {gText_PleaseRemoveTheMail,     MSG_FMT_NONE},
    [MSG_IS_SELECTED2]         = {gText_PkmnIsSelected,          MSG_FMT_ITEM_NAME},
    [MSG_GIVE_TO_MON]          = {gText_GiveToAPkmn,             MSG_FMT_NONE},
    [MSG_PLACED_IN_BAG]        = {gText_PlacedItemInBag,         MSG_FMT_ITEM_NAME},
    [MSG_BAG_FULL]             = {gText_BagIsFull2,              MSG_FMT_NONE},
    [MSG_PUT_IN_BAG]           = {gText_PutItemInBag,            MSG_FMT_NONE},
    [MSG_ITEM_IS_HELD]         = {gText_ItemIsNowHeld,           MSG_FMT_ITEM_NAME},
    [MSG_CHANGED_TO_ITEM]      = {gText_ChangedToNewItem,        MSG_FMT_ITEM_NAME},
    [MSG_CANT_STORE_MAIL]      = {gText_MailCantBeStored,        MSG_FMT_NONE},
};

static const struct StorageMessage sReleaseMessages[] = {
[0] = {gText_Release1, MSG_FMT_RELEASE_MON_3},
[1] = {gText_Release2, MSG_FMT_RELEASE_MON_3},
[2] = {gText_Release3, MSG_FMT_RELEASE_MON_3},
[3] = {gText_Release4, MSG_FMT_RELEASE_MON_3},
[4] = {gText_Release5, MSG_FMT_RELEASE_MON_3},
[5] = {gText_Release6, MSG_FMT_RELEASE_MON_3},
[6] = {gText_Release7, MSG_FMT_RELEASE_MON_3},
[7] = {gText_Release8, MSG_FMT_RELEASE_MON_3},
[8] = {gText_Release9, MSG_FMT_RELEASE_MON_3},
[9] = {gText_Release10, MSG_FMT_RELEASE_MON_3},
[10] = {gText_Release11, MSG_FMT_RELEASE_MON_3},
[11] = {gText_Release12, MSG_FMT_RELEASE_MON_3},
[12] = {gText_Release13, MSG_FMT_RELEASE_MON_3},
[13] = {gText_Release14, MSG_FMT_RELEASE_MON_3},
[14] = {gText_Release15, MSG_FMT_RELEASE_MON_3},
[15] = {gText_Release16, MSG_FMT_RELEASE_MON_3},
[16] = {gText_Release17, MSG_FMT_RELEASE_MON_3},
[17] = {gText_Release18, MSG_FMT_RELEASE_MON_3},
[18] = {gText_Release19, MSG_FMT_RELEASE_MON_3},
[19] = {gText_Release20, MSG_FMT_RELEASE_MON_3},
[20] = {gText_Release21, MSG_FMT_RELEASE_MON_3},
[21] = {gText_Release22, MSG_FMT_RELEASE_MON_3},
[22] = {gText_Release23, MSG_FMT_RELEASE_MON_3},
[23] = {gText_Release24, MSG_FMT_RELEASE_MON_3},
[24] = {gText_Release25, MSG_FMT_RELEASE_MON_3},
[25] = {gText_Release26, MSG_FMT_RELEASE_MON_3},
[26] = {gText_Release27, MSG_FMT_RELEASE_MON_3},
[27] = {gText_Release28, MSG_FMT_RELEASE_MON_3},
[28] = {gText_Release29, MSG_FMT_RELEASE_MON_3},
[29] = {gText_Release30, MSG_FMT_RELEASE_MON_3},
[30] = {gText_Release31, MSG_FMT_RELEASE_MON_3},
[31] = {gText_Release32, MSG_FMT_RELEASE_MON_3},
[32] = {gText_Release33, MSG_FMT_RELEASE_MON_3},
[33] = {gText_Release34, MSG_FMT_RELEASE_MON_3},
[34] = {gText_Release35, MSG_FMT_RELEASE_MON_3},
[35] = {gText_Release36, MSG_FMT_RELEASE_MON_3},
[36] = {gText_Release37, MSG_FMT_RELEASE_MON_3},
[37] = {gText_Release38, MSG_FMT_RELEASE_MON_3},
[38] = {gText_Release39, MSG_FMT_RELEASE_MON_3},
[39] = {gText_Release40, MSG_FMT_RELEASE_MON_3},
[40] = {gText_Release41, MSG_FMT_RELEASE_MON_3},
[41] = {gText_Release42, MSG_FMT_RELEASE_MON_3},
[42] = {gText_Release43, MSG_FMT_RELEASE_MON_3},
[43] = {gText_Release44, MSG_FMT_RELEASE_MON_3},
[44] = {gText_Release45, MSG_FMT_RELEASE_MON_3},
[45] = {gText_Release46, MSG_FMT_RELEASE_MON_3},
[46] = {gText_Release47, MSG_FMT_RELEASE_MON_3},
[47] = {gText_Release48, MSG_FMT_RELEASE_MON_3},
[48] = {gText_Release49, MSG_FMT_RELEASE_MON_3},
[49] = {gText_Release50, MSG_FMT_RELEASE_MON_3},
[50] = {gText_Release51, MSG_FMT_RELEASE_MON_3},
[51] = {gText_Release52, MSG_FMT_RELEASE_MON_3},
[52] = {gText_Release53, MSG_FMT_RELEASE_MON_3},
[53] = {gText_Release54, MSG_FMT_RELEASE_MON_3},
[54] = {gText_Release55, MSG_FMT_RELEASE_MON_3},
[55] = {gText_Release56, MSG_FMT_RELEASE_MON_3},
[56] = {gText_Release57, MSG_FMT_RELEASE_MON_3},
[57] = {gText_Release58, MSG_FMT_RELEASE_MON_3},
[58] = {gText_Release59, MSG_FMT_RELEASE_MON_3},
[59] = {gText_Release60, MSG_FMT_RELEASE_MON_3},
[60] = {gText_Release61, MSG_FMT_RELEASE_MON_3},
[61] = {gText_Release62, MSG_FMT_RELEASE_MON_3},
[62] = {gText_Release63, MSG_FMT_RELEASE_MON_3},
[63] = {gText_Release64, MSG_FMT_RELEASE_MON_3},
[64] = {gText_Release65, MSG_FMT_RELEASE_MON_3},
[65] = {gText_Release66, MSG_FMT_RELEASE_MON_3},
[66] = {gText_Release67, MSG_FMT_RELEASE_MON_3},
[67] = {gText_Release68, MSG_FMT_RELEASE_MON_3},
[68] = {gText_Release69, MSG_FMT_RELEASE_MON_3},
[69] = {gText_Release70, MSG_FMT_RELEASE_MON_3},
[70] = {gText_Release71, MSG_FMT_RELEASE_MON_3},
[71] = {gText_Release72, MSG_FMT_RELEASE_MON_3},
[72] = {gText_Release73, MSG_FMT_RELEASE_MON_3},
[73] = {gText_Release74, MSG_FMT_RELEASE_MON_3},
[74] = {gText_Release75, MSG_FMT_RELEASE_MON_3},
[75] = {gText_Release76, MSG_FMT_RELEASE_MON_3},
[76] = {gText_Release77, MSG_FMT_RELEASE_MON_3},
[77] = {gText_Release78, MSG_FMT_RELEASE_MON_3},
[78] = {gText_Release79, MSG_FMT_RELEASE_MON_3},
[79] = {gText_Release80, MSG_FMT_RELEASE_MON_3},
[80] = {gText_Release81, MSG_FMT_RELEASE_MON_3},
[81] = {gText_Release82, MSG_FMT_RELEASE_MON_3},
[82] = {gText_Release83, MSG_FMT_RELEASE_MON_3},
[83] = {gText_Release84, MSG_FMT_RELEASE_MON_3},
[84] = {gText_Release85, MSG_FMT_RELEASE_MON_3},
[85] = {gText_Release86, MSG_FMT_RELEASE_MON_3},
[86] = {gText_Release87, MSG_FMT_RELEASE_MON_3},
[87] = {gText_Release88, MSG_FMT_RELEASE_MON_3},
[88] = {gText_Release89, MSG_FMT_RELEASE_MON_3},
[89] = {gText_Release90, MSG_FMT_RELEASE_MON_3},
[90] = {gText_Release91, MSG_FMT_RELEASE_MON_3},
[91] = {gText_Release92, MSG_FMT_RELEASE_MON_3},
[92] = {gText_Release93, MSG_FMT_RELEASE_MON_3},
[93] = {gText_Release94, MSG_FMT_RELEASE_MON_3},
[94] = {gText_Release95, MSG_FMT_RELEASE_MON_3},
[95] = {gText_Release96, MSG_FMT_RELEASE_MON_3},
[96] = {gText_Release97, MSG_FMT_RELEASE_MON_3},
[97] = {gText_Release98, MSG_FMT_RELEASE_MON_3},
[98] = {gText_Release99, MSG_FMT_RELEASE_MON_3},
[99] = {gText_Release100, MSG_FMT_RELEASE_MON_3},
[100] = {gText_Release101, MSG_FMT_RELEASE_MON_3},
[101] = {gText_Release102, MSG_FMT_RELEASE_MON_3},
[102] = {gText_Release103, MSG_FMT_RELEASE_MON_3},
[103] = {gText_Release104, MSG_FMT_RELEASE_MON_3},
[104] = {gText_Release105, MSG_FMT_RELEASE_MON_3},
[105] = {gText_Release106, MSG_FMT_RELEASE_MON_3},
[106] = {gText_Release107, MSG_FMT_RELEASE_MON_3},
[107] = {gText_Release108, MSG_FMT_RELEASE_MON_3},
[108] = {gText_Release109, MSG_FMT_RELEASE_MON_3},
[109] = {gText_Release110, MSG_FMT_RELEASE_MON_3},
[110] = {gText_Release111, MSG_FMT_RELEASE_MON_3},
[111] = {gText_Release112, MSG_FMT_RELEASE_MON_3},
[112] = {gText_Release113, MSG_FMT_RELEASE_MON_3},
[113] = {gText_Release114, MSG_FMT_RELEASE_MON_3},
[114] = {gText_Release115, MSG_FMT_RELEASE_MON_3},
[115] = {gText_Release116, MSG_FMT_RELEASE_MON_3},
[116] = {gText_Release117, MSG_FMT_RELEASE_MON_3},
[117] = {gText_Release118, MSG_FMT_RELEASE_MON_3},
[118] = {gText_Release119, MSG_FMT_RELEASE_MON_3},
[119] = {gText_Release120, MSG_FMT_RELEASE_MON_3},
[120] = {gText_Release121, MSG_FMT_RELEASE_MON_3},
[121] = {gText_Release122, MSG_FMT_RELEASE_MON_3},
[122] = {gText_Release123, MSG_FMT_RELEASE_MON_3},
[123] = {gText_Release124, MSG_FMT_RELEASE_MON_3},
[124] = {gText_Release125, MSG_FMT_RELEASE_MON_3},
[125] = {gText_Release126, MSG_FMT_RELEASE_MON_3},
[126] = {gText_Release127, MSG_FMT_RELEASE_MON_3},
[127] = {gText_Release128, MSG_FMT_RELEASE_MON_3},
[128] = {gText_Release129, MSG_FMT_RELEASE_MON_3},
[129] = {gText_Release130, MSG_FMT_RELEASE_MON_3},
[130] = {gText_Release131, MSG_FMT_RELEASE_MON_3},
[131] = {gText_Release132, MSG_FMT_RELEASE_MON_3},
[132] = {gText_Release133, MSG_FMT_RELEASE_MON_3},
[133] = {gText_Release134, MSG_FMT_RELEASE_MON_3},
[134] = {gText_Release135, MSG_FMT_RELEASE_MON_3},
[135] = {gText_Release136, MSG_FMT_RELEASE_MON_3},
[136] = {gText_Release137, MSG_FMT_RELEASE_MON_3},
[137] = {gText_Release138, MSG_FMT_RELEASE_MON_3},
[138] = {gText_Release139, MSG_FMT_RELEASE_MON_3},
[139] = {gText_Release140, MSG_FMT_RELEASE_MON_3},
[140] = {gText_Release141, MSG_FMT_RELEASE_MON_3},
[141] = {gText_Release142, MSG_FMT_RELEASE_MON_3},
[142] = {gText_Release143, MSG_FMT_RELEASE_MON_3},
[143] = {gText_Release144, MSG_FMT_RELEASE_MON_3},
[144] = {gText_Release145, MSG_FMT_RELEASE_MON_3},
[145] = {gText_Release146, MSG_FMT_RELEASE_MON_3},
[146] = {gText_Release147, MSG_FMT_RELEASE_MON_3},
[147] = {gText_Release148, MSG_FMT_RELEASE_MON_3},
[148] = {gText_Release149, MSG_FMT_RELEASE_MON_3},
[149] = {gText_Release150, MSG_FMT_RELEASE_MON_3},
[150] = {gText_Release151, MSG_FMT_RELEASE_MON_3},
[151] = {gText_Release152, MSG_FMT_RELEASE_MON_3},
[152] = {gText_Release153, MSG_FMT_RELEASE_MON_3},
[153] = {gText_Release154, MSG_FMT_RELEASE_MON_3},
[154] = {gText_Release155, MSG_FMT_RELEASE_MON_3},
[155] = {gText_Release156, MSG_FMT_RELEASE_MON_3},
[156] = {gText_Release157, MSG_FMT_RELEASE_MON_3},
[157] = {gText_Release158, MSG_FMT_RELEASE_MON_3},
[158] = {gText_Release159, MSG_FMT_RELEASE_MON_3},
[159] = {gText_Release160, MSG_FMT_RELEASE_MON_3},
[160] = {gText_Release161, MSG_FMT_RELEASE_MON_3},
[161] = {gText_Release162, MSG_FMT_RELEASE_MON_3},
[162] = {gText_Release163, MSG_FMT_RELEASE_MON_3},
[163] = {gText_Release164, MSG_FMT_RELEASE_MON_3},
[164] = {gText_Release165, MSG_FMT_RELEASE_MON_3},
};

static const struct WindowTemplate sYesNoWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 24,
    .tilemapTop = 11,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x05c
};

static const struct OamData sOamData_DisplayMon = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0x000,
    .priority = 0,
    .paletteNum = 0
};

// Waveform

static const struct OamData sOamData_Waveform = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x8),
    .tileNum = 0x000,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sAnim_Waveform_LeftOff[] = {
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_Waveform_LeftOn[] = {
    ANIMCMD_FRAME(2, 8),
    ANIMCMD_FRAME(4, 8),
    ANIMCMD_FRAME(6, 8),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd sAnim_Waveform_RightOff[] = {
    ANIMCMD_FRAME(8, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnim_Waveform_RightOn[] = {
    ANIMCMD_FRAME(10, 8),
    ANIMCMD_FRAME(4, 8),
    ANIMCMD_FRAME(12, 8),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sAnims_Waveform[] = {
    sAnim_Waveform_LeftOff,
    sAnim_Waveform_LeftOn,
    sAnim_Waveform_RightOff,
    sAnim_Waveform_RightOn
};

static const struct SpriteTemplate sSpriteTemplate_Waveform = {
    .tileTag = GFXTAG_WAVEFORM,
    .paletteTag = PALTAG_MISC_2,
    .oam = &sOamData_Waveform,
    .anims = sAnims_Waveform,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static void VBlankCB_PokeStorage(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    UnkUtil_Run();
    TransferPlttBuffer();
    SetGpuReg(REG_OFFSET_BG2HOFS, gStorage->bg2_X);
}

static void CB2_PokeStorage(void)
{
    RunTasks();
    DoScheduledBgTilemapCopiesToVram();
    ScrollBackground();
    UpdateCloseBoxButtonFlash();
    AnimateSprites();
    BuildOamBuffer();
}

void EnterPokeStorage(u8 boxOption)
{
    ResetTasks();
    sCurrentBoxOption = boxOption;
    gStorage = Alloc(sizeof(struct PokemonStorageSystemData));
    if (gStorage == NULL)
        SetMainCallback2(CB2_ExitPokeStorage);
    else
    {
        gStorage->boxOption = boxOption;
        gStorage->isReopening = FALSE;
        sMovingItemId = 0;
        gStorage->state = 0;
        gStorage->taskId = CreateTask(Task_InitPokeStorage, 3);
        SetHelpContext(HELPCONTEXT_BILLS_PC);
        sLastUsedBox = StorageGetCurrentBox();
        SetMainCallback2(CB2_PokeStorage);
    }
}

void CB2_ReturnToPokeStorage(void)
{
    ResetTasks();
    gStorage = Alloc(sizeof(struct PokemonStorageSystemData));
    if (gStorage == NULL)
        SetMainCallback2(CB2_ExitPokeStorage);
    else
    {
        gStorage->boxOption = sCurrentBoxOption;
        gStorage->isReopening = TRUE;
        gStorage->state = 0;
        gStorage->taskId = CreateTask(Task_InitPokeStorage, 3);
        SetHelpContext(HELPCONTEXT_BILLS_PC);
        SetMainCallback2(CB2_PokeStorage);
    }
}

static void ResetAllBgCoords(void)
{
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
}

static void ResetForPokeStorage(void)
{
    ResetPaletteFade();
    ResetSpriteData();
    FreeSpriteTileRanges();
    FreeAllSpritePalettes();
    ClearDma3Requests();
    gReservedSpriteTileCount = 0x280;
    UnkUtil_Init(&gStorage->unkUtil, gStorage->unkUtilData, ARRAY_COUNT(gStorage->unkUtilData));
    gKeyRepeatStartDelay = 20;
    ClearScheduledBgCopiesToVram();
    TilemapUtil_Init(TILEMAP_COUNT);
    TilemapUtil_SetTilemap(TILEMAP_PKMN_DATA, 1, sPkmnData_Tilemap, 8, 4);
    TilemapUtil_SetPos(TILEMAP_PKMN_DATA, 1, 0);
    gStorage->closeBoxFlashing = FALSE;
}

static void InitStartingPosData(void)
{
    ClearSavedCursorPos();
    sInPartyMenu = gStorage->boxOption == OPTION_DEPOSIT;
    sDepositBoxId = 0;
}

static void SetMonIconTransparency(void)
{
    if (gStorage->boxOption == OPTION_MOVE_ITEMS)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL);
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(7, 11));
    }
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP);
}

static void SetPokeStorageTask(TaskFunc newFunc)
{
    gTasks[gStorage->taskId].func = newFunc;
    gStorage->state = 0;
}

static void Task_InitPokeStorage(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetForPokeStorage();
        if (gStorage->isReopening)
        {
            switch (sWhichToReshow)
            {
            case SCREEN_CHANGE_NAME_BOX - 1:
                LoadSavedMovingMon();
                break;
            case SCREEN_CHANGE_SUMMARY_SCREEN - 1:
                SetSelectionAfterSummaryScreen();
                break;
            case SCREEN_CHANGE_ITEM_FROM_BAG - 1:
                GiveChosenBagItem();
                break;
            }
        }
        LoadPokeStorageMenuGfx();
        LoadsMiscSpritePalette();
        break;
    case 1:
        if (!InitPokeStorageWindows())
        {
            SetPokeStorageTask(Task_ChangeScreen);
            return;
        }
        break;
    case 2:
        PutWindowTilemap(0);
        ClearWindowTilemap(1);
        CpuFill32(0, (void *)VRAM, 0x200);
        TextWindow_SetUserSelectedFrame(1, 0xB, 0xE0);
        break;
    case 3:
        ResetAllBgCoords();
        if (!gStorage->isReopening)
            InitStartingPosData();
        break;
    case 4:
        InitMonIconFields();
        if (!gStorage->isReopening)
            InitCursor();
        else
            InitCursorOnReopen();
        break;
    case 5:
        if (!MultiMove_Init())
        {
            SetPokeStorageTask(Task_ChangeScreen);
            return;
        }
        else
        {
            SetScrollingBackground();
            InitPokeStorageBg0();
        }
        break;
    case 6:
        InitPalettesAndSprites();
        break;
    case 7:
        InitSupplementalTilemaps();
        break;
    case 8:
        CreateInitBoxTask(StorageGetCurrentBox());
        break;
    case 9:
        if (IsInitBoxActive())
            return;

        if (gStorage->boxOption != OPTION_MOVE_ITEMS)
        {
            gStorage->markMenu.baseTileTag = GFXTAG_MARKING_MENU;
            gStorage->markMenu.basePaletteTag = PALTAG_MARKING_MENU;
            InitMonMarkingsMenu(&gStorage->markMenu);
            BufferMonMarkingsMenuTiles();
        }
        else
        {
            CreateItemIconSprites();
            InitCursorItemIcon();
        }
        break;
    case 10:
        SetMonIconTransparency();
        if (!gStorage->isReopening)
        {
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
            SetPokeStorageTask(Task_ShowPokeStorage);
        }
        else
        {
            BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
            SetPokeStorageTask(Task_ReshowPokeStorage);
        }
        SetVBlankCallback(VBlankCB_PokeStorage);
        return;
    default:
        return;
    }

    gStorage->state++;
}

static void Task_ShowPokeStorage(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PlaySE(SE_PC_LOGIN);
        BeginPCScreenEffect_TurnOn(20, 0, 1);
        gStorage->state++;
        break;
    case 1:
        if (!IsPCScreenEffectRunning_TurnOn())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ReshowPokeStorage(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        BeginNormalPaletteFade(PALETTES_ALL, -1, 0x10, 0, RGB_BLACK);
        gStorage->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_PokeStorageMain(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        switch (HandleInput())
        {
        case INPUT_MOVE_CURSOR:
            PlaySE(SE_SELECT);
            gStorage->state = 1;
            break;
        case INPUT_SHOW_PARTY:
            if (gStorage->boxOption != OPTION_MOVE_MONS && gStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                PrintStorageMessage(MSG_WHICH_ONE_WILL_TAKE);
                gStorage->state = 3;
            }
            else
            {
                ClearSavedCursorPos();
                SetPokeStorageTask(Task_ShowPartyPokemon);
            }
            break;
        case INPUT_HIDE_PARTY:
            if (gStorage->boxOption == OPTION_MOVE_MONS)
            {
                if (IsMonBeingMoved() && ItemIsMail(gStorage->displayMonItemId))
                    gStorage->state = 5;
                else
                    SetPokeStorageTask(Task_HidePartyPokemon);
            }
            else if (gStorage->boxOption == OPTION_MOVE_ITEMS)
                SetPokeStorageTask(Task_HidePartyPokemon);
            break;
        case INPUT_CLOSE_BOX:
            SetPokeStorageTask(Task_OnCloseBoxPressed);
            break;
        case INPUT_PRESSED_B:
            SetPokeStorageTask(Task_OnBPressed);
            break;
        case INPUT_BOX_OPTIONS:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_HandleBoxOptions);
            break;
        case INPUT_IN_MENU:
            SetPokeStorageTask(Task_OnSelectedMon);
            break;
        case INPUT_SCROLL_RIGHT:
            PlaySE(SE_SELECT);
            gStorage->newCurrBoxId = StorageGetCurrentBox() + 1;
            if (gStorage->newCurrBoxId >= TOTAL_BOXES_COUNT)
                gStorage->newCurrBoxId = 0;
            if (gStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                SetUpScrollToBox(gStorage->newCurrBoxId);
                gStorage->state = 2;
            }
            else
            {
                TryHideItemAtCursor();
                gStorage->state = 10;
            }
            break;
        case INPUT_SCROLL_LEFT:
            PlaySE(SE_SELECT);
            gStorage->newCurrBoxId = StorageGetCurrentBox() - 1;
            if (gStorage->newCurrBoxId < 0)
                gStorage->newCurrBoxId = TOTAL_BOXES_COUNT - 1;
            if (gStorage->boxOption != OPTION_MOVE_ITEMS)
            {
                SetUpScrollToBox(gStorage->newCurrBoxId);
                gStorage->state = 2;
            }
            else
            {
                TryHideItemAtCursor();
                gStorage->state = 10;
            }
            break;
        case INPUT_DEPOSIT:
            if (!CanMovePartyMon())
            {
                if (ItemIsMail(gStorage->displayMonItemId))
                    gStorage->state = 5;
                else
                {
                    PlaySE(SE_SELECT);
                    SetPokeStorageTask(Task_DepositMenu);
                }
            }
            else
                gStorage->state = 4;
            break;
        case INPUT_MOVE_MON:
            if (CanMovePartyMon())
                gStorage->state = 4;

            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_MoveMon);
            }
            break;
        case INPUT_SHIFT_MON:
            if (!CanShiftMon())
                gStorage->state = 4;
            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_ShiftMon);
            }
            break;
        case INPUT_WITHDRAW:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_WithdrawMon);
            break;
        case INPUT_PLACE_MON:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_PlaceMon);
            break;
        case INPUT_TAKE_ITEM:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_TakeItemForMoving);
            break;
        case INPUT_GIVE_ITEM:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_GiveMovingItemToMon);
            break;
        case INPUT_SWITCH_ITEMS:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_SwitchSelectedItem);
            break;
        case INPUT_MULTIMOVE_START:
            PlaySE(SE_SELECT);
            MultiMove_SetFunction(MULTIMOVE_START);
            gStorage->state = 7;
            break;
        case INPUT_MULTIMOVE_SINGLE:
            MultiMove_SetFunction(MULTIMOVE_SINGLE);
            gStorage->state = 8;
            break;
        case INPUT_MULTIMOVE_CHANGE_SELECTION:
            PlaySE(SE_SELECT);
            MultiMove_SetFunction(MULTIMOVE_CHANGE_SELECTION);
            gStorage->state = 9;
            break;
        case INPUT_MULTIMOVE_GRAB_SELECTION:
            MultiMove_SetFunction(MULTIMOVE_GRAB_SELECTION);
            gStorage->state = 7;
            break;
        case INPUT_MULTIMOVE_MOVE_MONS:
            PlaySE(SE_SELECT);
            MultiMove_SetFunction(MULTIMOVE_MOVE_MONS);
            gStorage->state = 9;
            break;
        case INPUT_MULTIMOVE_PLACE_MONS:
            PlaySE(SE_SELECT);
            SetPokeStorageQuestLogEvent(3);
            MultiMove_SetFunction(MULTIMOVE_PLACE_MONS);
            gStorage->state = 7;
            break;
        case INPUT_MULTIMOVE_UNABLE:
            PlaySE(SE_FAILURE);
            break;
        }
        break;
    case 1:
        if (!UpdateCursorPos())
        {
            if (IsCursorOnCloseBox())
                StartFlashingCloseBoxButton();
            else
                StopFlashingCloseBoxButton();

            if (gStorage->setMosaic)
                StartDisplayMonMosaic();
            gStorage->state = 0;
        }
        break;
    case 2:
        if (!ScrollToBox())
        {
            SetCurrentBox(gStorage->newCurrBoxId);
            if (!sInPartyMenu && !IsMonBeingMoved())
            {
                DoTrySetDisplayMonData();
                StartDisplayMonMosaic();
            }

            if (gStorage->boxOption == OPTION_MOVE_ITEMS)
            {
                TryShowItemAtCursor();
                gStorage->state = 11;
            }
            else
                gStorage->state = 0;
        }
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            gStorage->state = 0;
        }
        break;
    case 4:
        PlaySE(SE_FAILURE);
        PrintStorageMessage(MSG_LAST_POKE);
        gStorage->state = 6;
        break;
    case 5:
        PlaySE(SE_FAILURE);
        PrintStorageMessage(MSG_PLEASE_REMOVE_MAIL);
        gStorage->state = 6;
        break;
    case 6:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 7:
        if (!MultiMove_RunFunction())
            gStorage->state = 0;
        break;
    case 8:
        if (!MultiMove_RunFunction())
            SetPokeStorageTask(Task_MoveMon);
        break;
    case 9:
        if (!MultiMove_RunFunction())
        {
            if (gStorage->setMosaic)
                StartDisplayMonMosaic();
            gStorage->state = 0;
        }
        break;
    case 10:
        if (!IsItemIconAnimActive())
        {
            SetUpScrollToBox(gStorage->newCurrBoxId);
            gStorage->state = 2;
        }
        break;
    case 11:
        if (!IsItemIconAnimActive())
            gStorage->state = 0;
        break;
    }
}

static void Task_ShowPartyPokemon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        SetUpDoShowPartyMenu();
        gStorage->state++;
        break;
    case 1:
        if (!DoShowPartyMenu())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_HidePartyPokemon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PlaySE(SE_SELECT);
        SetUpHidePartyMenu();
        gStorage->state++;
        break;
    case 1:
        if (!HidePartyMenu())
        {
            SetCursorBoxPosition(GetSavedCursorPos());
            gStorage->state++;
        }
        break;
    case 2:
        if (!UpdateCursorPos())
        {
            if (gStorage->setMosaic)
                StartDisplayMonMosaic();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_OnSelectedMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (!IsDisplayMonMosaicActive())
        {
            PlaySE(SE_SELECT);
            if (gStorage->boxOption != OPTION_MOVE_ITEMS)
                PrintStorageMessage(MSG_IS_SELECTED);
            else if (IsActiveItemMoving() || gStorage->displayMonItemId != 0)
                PrintStorageMessage(MSG_IS_SELECTED2);
            else
                PrintStorageMessage(MSG_GIVE_TO_MON);

            AddMenu();
            gStorage->state = 1;
        }
        break;
    case 1:
        if (!IsMenuLoading())
            gStorage->state = 2;
        break;
    case 2:
        switch (HandleMenuInput())
        {
        case MENU_B_PRESSED:
        case MENU_TEXT_CANCEL:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case MENU_TEXT_MOVE:
            if (CanMovePartyMon())
                gStorage->state = 3;
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_MoveMon);
            }
            break;
        case MENU_TEXT_PLACE:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_PlaceMon);
            break;
        case MENU_TEXT_SHIFT:
            if (!CanShiftMon())
                gStorage->state = 3;
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_ShiftMon);
            }
            break;
        case MENU_TEXT_WITHDRAW:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_WithdrawMon);
            break;
        case MENU_TEXT_STORE:
            if (CanMovePartyMon())
                gStorage->state = 3;
            else if (ItemIsMail(gStorage->displayMonItemId))
                gStorage->state = 4;
            else
            {
                PlaySE(SE_SELECT);
                ClearBottomWindow();
                SetPokeStorageTask(Task_DepositMenu);
            }
            break;
        case MENU_TEXT_RELEASE:
            if (CanMovePartyMon())
                gStorage->state = 3;
            else if (gStorage->displayMonIsEgg)
                gStorage->state = 5;
            else if (ItemIsMail(gStorage->displayMonItemId))
                gStorage->state = 4;
            else
            {
                PlaySE(SE_SELECT);
                SetPokeStorageTask(Task_ReleaseMon);
            }
            break;
        case MENU_TEXT_SUMMARY:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_ShowMonSummary);
            break;
        case MENU_TEXT_MARK:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_ShowMarkMenu);
            break;
        case MENU_TEXT_TAKE:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_TakeItemForMoving);
            break;
        case MENU_TEXT_GIVE:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_GiveMovingItemToMon);
            break;
        case MENU_TEXT_BAG:
            SetPokeStorageTask(Task_ItemToBag);
            break;
        case MENU_TEXT_SWITCH:
            SetPokeStorageTask(Task_SwitchSelectedItem);
            break;
        case MENU_TEXT_GIVE2:
            SetPokeStorageTask(Task_GiveItemFromBag);
            break;
        case MENU_TEXT_INFO:
            SetPokeStorageTask(Task_ShowItemInfo);
            break;
        }
        break;
    case 3:
        PlaySE(SE_FAILURE);
        PrintStorageMessage(MSG_LAST_POKE);
        gStorage->state = 6;
        break;
    case 5:
        PlaySE(SE_FAILURE);
        PrintStorageMessage(MSG_CANT_RELEASE_EGG);
        gStorage->state = 6;
        break;
    case 4:
        PlaySE(SE_FAILURE);
        PrintStorageMessage(MSG_PLEASE_REMOVE_MAIL);
        gStorage->state = 6;
        break;
    case 6:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_MoveMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        InitMonPlaceChange(CHANGE_GRAB);
        gStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            if (sInPartyMenu)
                SetPokeStorageTask(Task_HandleMovingMonFromParty);
            else
                SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_PlaceMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        SetPokeStorageQuestLogEvent(1);
        InitMonPlaceChange(CHANGE_PLACE);
        gStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            if (sInPartyMenu)
                SetPokeStorageTask(Task_HandleMovingMonFromParty);
            else
                SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_ShiftMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        SetPokeStorageQuestLogEvent(0);
        InitMonPlaceChange(CHANGE_SHIFT);
        gStorage->state++;
        break;
    case 1:
        if (!DoMonPlaceChange())
        {
            StartDisplayMonMosaic();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_WithdrawMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (CalculatePlayerPartyCount() == PARTY_SIZE)
        {
            PrintStorageMessage(MSG_PARTY_FULL);
            gStorage->state = 1;
        }
        else
        {
            SaveCursorPos();
            InitMonPlaceChange(CHANGE_GRAB);
            gStorage->state = 2;
        }
        break;
    case 1:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 2:
        if (!DoMonPlaceChange())
        {
            SetMovingMonPriority(1);
            SetUpDoShowPartyMenu();
            gStorage->state++;
        }
        break;
    case 3:
        if (!DoShowPartyMenu())
        {
            SetPokeStorageQuestLogEvent(1);
            InitMonPlaceChange(CHANGE_PLACE);
            gStorage->state++;
        }
        break;
    case 4:
        if (!DoMonPlaceChange())
        {
            UpdatePartySlotColors();
            gStorage->state++;
        }
        break;
    case 5:
        SetPokeStorageTask(Task_HidePartyPokemon);
        break;
    }
}

static void Task_DepositMenu(u8 taskId)
{
    u8 boxId;

    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_DEPOSIT_IN_WHICH_BOX);
        LoadChooseBoxMenuGfx(&gStorage->chooseBoxMenu, GFXTAG_CHOOSE_BOX_MENU_CENTER, PALTAG_MISC_1, 3, FALSE);
        CreateChooseBoxMenuSprites(sDepositBoxId);
        gStorage->state++;
        break;
    case 1:
        boxId = HandleBoxChooseSelectionInput();
        switch (boxId)
        {
        case BOXID_NONE_CHOSEN:
            break;
        case BOXID_CANCELED:
            ClearBottomWindow();
            DestroyChooseBoxMenuSprites();
            FreeBoxSelectionPopupSpriteGfx();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        default:
            if (TryStorePartyMonInBox(boxId))
            {
                sDepositBoxId = boxId;
                SetPokeStorageQuestLogEvent(2);
                ClearBottomWindow();
                DestroyChooseBoxMenuSprites();
                FreeBoxSelectionPopupSpriteGfx();
                gStorage->state = 2;
            }
            else
            {
                PrintStorageMessage(MSG_BOX_IS_FULL);
                gStorage->state = 4;
            }
            break;
        }
        break;
    case 2:
        CompactPartySlots();
        CompactPartySprites();
        gStorage->state++;
        break;
    case 3:
        if (GetNumPartySpritesCompacting() == 0)
        {
            ResetSelectionAfterDeposit();
            StartDisplayMonMosaic();
            UpdatePartySlotColors();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 4:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintStorageMessage(MSG_DEPOSIT_IN_WHICH_BOX);
            gStorage->state = 1;
        }
        break;
    }
}

static void Task_ReleaseMon(u8 taskId)
{
    if(sReleasedelay !=0)
    {
        sReleasedelay--;
        return;
    }
    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_RELEASE_POKE);
        ShowYesNoWindow(1);
        gStorage->state++;
        // fallthrough
    case 1:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case MENU_B_PRESSED:
        case 1:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 0:
            ClearBottomWindow();
            InitCanReleaseMonVars();
            InitReleaseMon();
            gStorage->state++;
            break;
        }
        break;
    case 2:
        RunCanReleaseMon();
        if (!TryHideReleaseMon())
        {
            while (TRUE)
            {
                // keep checking until status is no longer RELEASE_MON_UNDETERMINED
                s8 canReleaseStatus = RunCanReleaseMon();
                if (canReleaseStatus == RELEASE_MON_ALLOWED)
                {
                    gStorage->state++;
                    break;
                }
                else if (canReleaseStatus == RELEASE_MON_NOT_ALLOWED)
                {
                    gStorage->state = 9; // Can't release the mon.
                    break;
                }
            }
        }
        break;
    case 3:
        ReleaseMon();
        RefreshDisplayMonData();
        sReleaseText = Random()>>10;
        while (sReleaseText > 55) {
            sReleaseText = Random()>>10;
        }
        sReleaseText *=3;
        PrintReleaseMessage(sReleaseText);
        sReleaseText++;
        gStorage->state++;
        sReleasedelay = 120;
        if(sPivoted <=3)
        {
            FlagClear(FLAG_HIDE_BLOOD1+sPivoted);
            sPivoted++;
        }
        break;
    case 4:
        if(StringLength(sReleaseMessages[sReleaseText].text)==0)
        {
            gStorage->state++;
            break;
        }
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintReleaseMessage(sReleaseText);
            sReleaseText++;
            gStorage->state++;
            sReleasedelay = 120;
        }
        break;
    case 5:
        if(StringLength(sReleaseMessages[sReleaseText].text)==0)
        {
            gStorage->state++;
            break;
        }
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintReleaseMessage(sReleaseText);
            sReleaseText++;
            gStorage->state++;
            sReleasedelay = 120; 
        }
        break;
    case 6:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            if (sInPartyMenu)
            {
                CompactPartySlots();
                CompactPartySprites();
                gStorage->state++;
            }
            else
                gStorage->state = 8;
        }
        break;
    case 7:
        if (GetNumPartySpritesCompacting() == 0)
        {
            DoTrySetDisplayMonData();
            StartDisplayMonMosaic();
            UpdatePartySlotColors();
            gStorage->state++;
        }
        break;
    case 8:
        SetPokeStorageTask(Task_PokeStorageMain);
        break;
    case 9:
        // Start "can't release" sequence
        PrintStorageMessage(MSG_WAS_RELEASED);
        gStorage->state++;
        break;
    case 10:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintStorageMessage(MSG_SURPRISE);
            gStorage->state++;
        }
        break;
    case 11:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            DoReleaseMonComeBackAnim();
            gStorage->state++;
        }
        break;
    case 12:
        if (!ResetReleaseMonSpritePtr())
        {
            TrySetCursorFistAnim();
            PrintStorageMessage(MSG_CAME_BACK);
            gStorage->state++;
        }
        break;
    case 13:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PrintStorageMessage(MSG_WORRIED);
            gStorage->state++;
        }
        break;
    case 14:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_ShowMarkMenu(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_MARK_POKE);
        gStorage->markMenu.markings = gStorage->displayMonMarkings;
        OpenMonMarkingsMenu(gStorage->displayMonMarkings, 176, 16);
        gStorage->state++;
        break;
    case 1:
        if (!HandleMonMarkingsMenuInput())
        {
            FreeMonMarkingsMenu();
            ClearBottomWindow();
            SetMonMarkings(gStorage->markMenu.markings);
            RefreshDisplayMonData();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_TakeItemForMoving(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (!ItemIsMail(gStorage->displayMonItemId))
        {
            ClearBottomWindow();
            gStorage->state++;
        }
        else
            SetPokeStorageTask(Task_PrintCantStoreMail);
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        Item_FromMonToMoving(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetBoxCursorPosition());
        gStorage->state++;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_FIST);
            ClearBottomWindow();
            DoTrySetDisplayMonData();
            PrintDisplayMonInfo();
            gStorage->state++;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_GiveMovingItemToMon(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        ClearBottomWindow();
        gStorage->state++;
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        Item_GiveMovingToMon(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetBoxCursorPosition());
        gStorage->state++;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_BOUNCE);
            DoTrySetDisplayMonData();
            PrintDisplayMonInfo();
            PrintStorageMessage(MSG_ITEM_IS_HELD);
            gStorage->state++;
        }
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            gStorage->state++;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ItemToBag(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (!AddBagItem(gStorage->displayMonItemId, 1))
        {
            PlaySE(SE_FAILURE);
            PrintStorageMessage(MSG_BAG_FULL);
            gStorage->state = 3;
        }
        else
        {
            PlaySE(SE_SELECT);
            Item_TakeMons(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetBoxCursorPosition());
            gStorage->state = 1;
        }
        break;
    case 1:
        if (!IsItemIconAnimActive())
        {
            PrintStorageMessage(MSG_PLACED_IN_BAG);
            gStorage->state = 2;
        }
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            DoTrySetDisplayMonData();
            PrintDisplayMonInfo();
            gStorage->state = 4;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_SwitchSelectedItem(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (!ItemIsMail(gStorage->displayMonItemId))
        {
            ClearBottomWindow();
            gStorage->state++;
        }
        else
            SetPokeStorageTask(Task_PrintCantStoreMail);
        break;
    case 1:
        StartCursorAnim(CURSOR_ANIM_OPEN);
        Item_SwitchMonsWithMoving(sInPartyMenu ? CURSOR_AREA_IN_PARTY : CURSOR_AREA_IN_BOX, GetBoxCursorPosition());
        gStorage->state++;
        break;
    case 2:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_FIST);
            DoTrySetDisplayMonData();
            PrintDisplayMonInfo();
            PrintStorageMessage(MSG_CHANGED_TO_ITEM);
            gStorage->state++;
        }
        break;
    case 3:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            gStorage->state++;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_ShowItemInfo(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        ClearBottomWindow();
        gStorage->state++;
        break;
    case 1:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            PlaySE(SE_WIN_OPEN);
            PrintItemDescription();
            InitItemInfoWindow();
            gStorage->state++;
        }
        break;
    case 2:
        if (!UpdateItemInfoWindowSlideIn())
            gStorage->state++;
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            gStorage->state++;
        break;
    case 4:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            PlaySE(SE_WIN_OPEN);
            gStorage->state++;
        }
        break;
    case 5:
        if (!UpdateItemInfoWindowSlideOut())
            gStorage->state++;
        break;
    case 6:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_CloseBoxWhileHoldingItem(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PlaySE(SE_SELECT);
        PrintStorageMessage(MSG_PUT_IN_BAG);
        ShowYesNoWindow(0);
        gStorage->state = 1;
        break;
    case 1:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case MENU_B_PRESSED:
        case 1:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 0:
            if (AddBagItem(gStorage->movingItemId, 1) == TRUE)
            {
                ClearBottomWindow();
                gStorage->state = 3;
            }
            else
            {
                PrintStorageMessage(MSG_BAG_FULL);
                gStorage->state = 2;
            }
            break;
        }
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            gStorage->state = 5;
        }
        break;
    case 3:
        MoveItemFromCursorToBag();
        gStorage->state = 4;
        break;
    case 4:
        if (!IsItemIconAnimActive())
        {
            StartCursorAnim(CURSOR_ANIM_BOUNCE);
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 5:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_HandleMovingMonFromParty(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        CompactPartySlots();
        CompactPartySprites();
        gStorage->state++;
        break;
    case 1:
        if (GetNumPartySpritesCompacting() == 0)
        {
            UpdatePartySlotColors();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_PrintCantStoreMail(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_CANT_STORE_MAIL);
        gStorage->state++;
        break;
    case 1:
        if (!IsDma3ManagerBusyWithBgCopy())
            gStorage->state++;
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            gStorage->state++;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
            SetPokeStorageTask(Task_PokeStorageMain);
        break;
    }
}

static void Task_HandleBoxOptions(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_WHAT_YOU_DO);
        AddMenu();
        gStorage->state++;
        break;
    case 1:
        if (IsMenuLoading())
            return;
        gStorage->state++;
    case 2:
        switch (HandleMenuInput())
        {
        case MENU_B_PRESSED:
        case MENU_TEXT_CANCEL:
            AnimateBoxScrollArrows(TRUE);
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case MENU_TEXT_NAME:
            PlaySE(SE_SELECT);
            SetPokeStorageTask(Task_NameBox);
            break;
        case MENU_TEXT_WALLPAPER:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_HandleWallpapers);
            break;
        case MENU_TEXT_JUMP:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            SetPokeStorageTask(Task_JumpBox);
            break;
        }
        break;
    }
}

static void Task_HandleWallpapers(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        AddWallpaperSetsMenu();
        PrintStorageMessage(MSG_PICK_A_THEME);
        gStorage->state++;
        break;
    case 1:
        if (!IsMenuLoading())
            gStorage->state++;
        break;
    case 2:
        gStorage->wallpaperSetId = HandleMenuInput();
        switch (gStorage->wallpaperSetId)
        {
        case MENU_B_PRESSED:
            AnimateBoxScrollArrows(TRUE);
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case MENU_TEXT_SCENERY_1:
        case MENU_TEXT_SCENERY_2:
        case MENU_TEXT_SCENERY_3:
        case MENU_TEXT_ETCETERA:
            PlaySE(SE_SELECT);
            RemoveMenu();
            gStorage->wallpaperSetId -= MENU_TEXT_SCENERY_1;
            gStorage->state++;
            break;
        }
        break;
    case 3:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            AddWallpapersMenu(gStorage->wallpaperSetId);
            PrintStorageMessage(MSG_PICK_A_WALLPAPER);
            gStorage->state++;
        }
        break;
    case 4:
        gStorage->wallpaperId = HandleMenuInput();
        switch (gStorage->wallpaperId)
        {
        case MENU_NOTHING_CHOSEN:
            break;
        case MENU_B_PRESSED:
            ClearBottomWindow();
            gStorage->state = 0;
            break;
        default:
            PlaySE(SE_SELECT);
            ClearBottomWindow();
            gStorage->wallpaperId -= MENU_TEXT_FOREST;
            SetWallpaperForCurrentBox(gStorage->wallpaperId);
            gStorage->state++;
            break;
        }
        break;
    case 5:
        if (!DoWallpaperGfxChange())
        {
            AnimateBoxScrollArrows(TRUE);
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_JumpBox(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        PrintStorageMessage(MSG_JUMP_TO_WHICH_BOX);
        LoadChooseBoxMenuGfx(&gStorage->chooseBoxMenu, GFXTAG_CHOOSE_BOX_MENU_CENTER, PALTAG_MISC_1, 3, FALSE);
        CreateChooseBoxMenuSprites(StorageGetCurrentBox());
        gStorage->state++;
        break;
    case 1:
        gStorage->newCurrBoxId = HandleBoxChooseSelectionInput();
        switch (gStorage->newCurrBoxId)
        {
        case BOXID_NONE_CHOSEN:
            break;
        default:
            ClearBottomWindow();
            DestroyChooseBoxMenuSprites();
            FreeBoxSelectionPopupSpriteGfx();
            if (gStorage->newCurrBoxId == BOXID_CANCELED || gStorage->newCurrBoxId == StorageGetCurrentBox())
            {
                AnimateBoxScrollArrows(TRUE);
                SetPokeStorageTask(Task_PokeStorageMain);
            }
            else
                gStorage->state++;
            break;
        }
        break;
    case 2:
        SetUpScrollToBox(gStorage->newCurrBoxId);
        gStorage->state++;
        break;
    case 3:
        if (!ScrollToBox())
        {
            SetCurrentBox(gStorage->newCurrBoxId);
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    }
}

static void Task_NameBox(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        SaveMovingMon();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gStorage->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            sWhichToReshow = SCREEN_CHANGE_NAME_BOX - 1;
            gStorage->screenChangeType = SCREEN_CHANGE_NAME_BOX;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_ShowMonSummary(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        InitSummaryScreenData();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gStorage->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            sWhichToReshow = SCREEN_CHANGE_SUMMARY_SCREEN - 1;
            gStorage->screenChangeType = SCREEN_CHANGE_SUMMARY_SCREEN;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_GiveItemFromBag(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gStorage->state++;
        break;
    case 1:
        if (!UpdatePaletteFade())
        {
            sWhichToReshow = SCREEN_CHANGE_ITEM_FROM_BAG - 1;
            gStorage->screenChangeType = SCREEN_CHANGE_ITEM_FROM_BAG;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_OnCloseBoxPressed(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (IsMonBeingMoved())
        {
            PlaySE(SE_FAILURE);
            PrintStorageMessage(MSG_HOLDING_POKE);
            gStorage->state = 1;
        }
        else if (IsActiveItemMoving())
            SetPokeStorageTask(Task_CloseBoxWhileHoldingItem);
        else
        {
            PlaySE(SE_SELECT);
            PrintStorageMessage(MSG_EXIT_BOX);
            ShowYesNoWindow(0);
            gStorage->state = 2;
        }
        break;
    case 1:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 2:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case MENU_B_PRESSED:
        case 1:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 0:
            PlaySE(SE_PC_OFF);
            ClearBottomWindow();
            gStorage->state++;
            break;
        }
        break;
    case 3:
        BeginPCScreenEffect_TurnOff(20, 0, 1);
        gStorage->state++;
        break;
    case 4:
        if (!IsPCScreenEffectRunning_TurnOff())
        {
            UpdateBoxToSendMons();
            gPlayerPartyCount = CalculatePlayerPartyCount();
            gStorage->screenChangeType = SCREEN_CHANGE_EXIT_BOX;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_OnBPressed(u8 taskId)
{
    switch (gStorage->state)
    {
    case 0:
        if (IsMonBeingMoved())
        {
            PlaySE(SE_FAILURE);
            PrintStorageMessage(MSG_HOLDING_POKE);
            gStorage->state = 1;
        }
        else if (IsActiveItemMoving())
            SetPokeStorageTask(Task_CloseBoxWhileHoldingItem);
        else
        {
            PlaySE(SE_SELECT);
            PrintStorageMessage(MSG_CONTINUE_BOX);
            ShowYesNoWindow(0);
            gStorage->state = 2;
        }
        break;
    case 1:
        if (JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
        {
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
        }
        break;
    case 2:
        switch (Menu_ProcessInputNoWrapClearOnChoose())
        {
        case 0:
            ClearBottomWindow();
            SetPokeStorageTask(Task_PokeStorageMain);
            break;
        case 1:
        case MENU_B_PRESSED:
            PlaySE(SE_PC_OFF);
            ClearBottomWindow();
            gStorage->state++;
            break;
        }
        break;
    case 3:
        BeginPCScreenEffect_TurnOff(20, 0, 0);
        gStorage->state++;
        break;
    case 4:
        if (!IsPCScreenEffectRunning_TurnOff())
        {
            UpdateBoxToSendMons();
            gPlayerPartyCount = CalculatePlayerPartyCount();
            gStorage->screenChangeType = SCREEN_CHANGE_EXIT_BOX;
            SetPokeStorageTask(Task_ChangeScreen);
        }
        break;
    }
}

static void Task_ChangeScreen(u8 taskId)
{
    struct Pokemon *party;
    u8 mode, cursorPos, lastIndex;
    u8 screenChangeType = gStorage->screenChangeType;

    if (gStorage->boxOption == OPTION_MOVE_ITEMS && IsActiveItemMoving() == TRUE)
        sMovingItemId = GetMovingItem();
    else
        sMovingItemId = ITEM_NONE;

    switch (screenChangeType)
    {
    case SCREEN_CHANGE_EXIT_BOX:
    default:
        FreePokeStorageData();
        SetMainCallback2(CB2_ExitPokeStorage);
        break;
    case SCREEN_CHANGE_SUMMARY_SCREEN:
        party = gStorage->summaryMonPtr.mon;
        cursorPos = gStorage->summaryCursorPos;
        lastIndex = gStorage->summaryLastIndex;
        mode = gStorage->summaryScreenMode;
        FreePokeStorageData();
        ShowPokemonSummaryScreen(party, cursorPos, lastIndex, CB2_ReturnToPokeStorage, mode);
        break;
    case SCREEN_CHANGE_NAME_BOX:
        FreePokeStorageData();
        DoNamingScreen(NAMING_SCREEN_BOX, GetBoxNamePtr(StorageGetCurrentBox()), 0, 0, 0, CB2_ReturnToPokeStorage);
        break;
    case SCREEN_CHANGE_ITEM_FROM_BAG:
        FreePokeStorageData();
        GoToBagMenu(ITEMMENULOCATION_PCBOX, OPEN_BAG_ITEMS, CB2_ReturnToPokeStorage);
        break;
    }

    DestroyTask(taskId);
}

static void GiveChosenBagItem(void)
{
    u16 item = gSpecialVar_ItemId;

    if (item != ITEM_NONE)
    {
        u8 id = GetBoxCursorPosition();

        if (sInPartyMenu)
            SetMonData(&gPlayerParty[id], MON_DATA_HELD_ITEM, &item);
        else
            SetCurrentBoxMonData(id, MON_DATA_HELD_ITEM, &item);

        RemoveBagItem(item, 1);
    }
}

static void FreePokeStorageData(void)
{
    TilemapUtil_Free();
    MultiMove_Free();
    FREE_AND_SET_NULL(gStorage);
    FreeAllWindowBuffers();
}

static void SetScrollingBackground(void)
{
    SetGpuReg(REG_OFFSET_BG3CNT, BGCNT_PRIORITY(3) | BGCNT_CHARBASE(3) | BGCNT_16COLOR | BGCNT_SCREENBASE(31));
    DecompressAndLoadBgGfxUsingHeap(3, sScrollingBg_Gfx, 0, 0, 0);
    LZ77UnCompVram(sScrollingBg_Tilemap, (void *)BG_SCREEN_ADDR(31));
}

static void ScrollBackground(void)
{
    ChangeBgX(3, 128, 1);
    ChangeBgY(3, 128, 2);
}

static void LoadPokeStorageMenuGfx(void)
{
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DecompressAndLoadBgGfxUsingHeap(1, gPokeStorageMenu_Gfx, 0, 0, 0);
    LZ77UnCompWram(sMenu_Tilemap, gStorage->menuTilemapBuffer);
    SetBgTilemapBuffer(1, gStorage->menuTilemapBuffer);
    ShowBg(1);
    ScheduleBgCopyTilemapToVram(1);
}

static bool8 InitPokeStorageWindows(void)
{
    if (!InitWindows(sWindowTemplates))
        return FALSE;

    DeactivateAllTextPrinters();
    return TRUE;
}

static void LoadsMiscSpritePalette(void)
{
    LoadSpritePalette(&sMiscSpritePalette);
}

static void InitPalettesAndSprites(void)
{
    LoadPalette(gPokeStorageInterface_Pal, 0, 0x20);
    LoadPalette(gPokeStorageInterface_NoDisplayMon_Pal, 0x20, 0x20);
    LoadPalette(sItemInfoFrame_Pal, 0xF0, 0x20);
    if (gStorage->boxOption != OPTION_MOVE_ITEMS)
        LoadPalette(sScrollingBg_Pal, 0x30, sizeof(sScrollingBg_Pal));
    else
        LoadPalette(sScrollingBgMoveItems_Pal, 0x30, sizeof(sScrollingBgMoveItems_Pal));

    SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(1) | BGCNT_CHARBASE(1) | BGCNT_16COLOR | BGCNT_SCREENBASE(30));
    CreateDisplayMonSprite();
    CreateMarkingComboSprite();
    CreateWaveformSprites();
    RefreshDisplayMonData();
}

static void CreateMarkingComboSprite(void)
{
    gStorage->markingComboSprite = CreateMonMarkingComboSprite(GFXTAG_MARKING_COMBO, PALTAG_MARKING_COMBO, NULL);
    gStorage->markingComboSprite->oam.priority = 1;
    gStorage->markingComboSprite->subpriority = 1;
    gStorage->markingComboSprite->x = 40;
    gStorage->markingComboSprite->y = 150;
    gStorage->markingComboTilesPtr = (void *)OBJ_VRAM0 + 32 * GetSpriteTileStartByTag(GFXTAG_MARKING_COMBO);
}

static void CreateWaveformSprites(void)
{
    u16 i;
    struct SpriteSheet sheet = sWaveformSpriteSheet;

    LoadSpriteSheet(&sheet);
    for (i = 0; i < 2; i++)
    {
        u8 spriteId = CreateSprite(&sSpriteTemplate_Waveform, i * 63 + 8, 9, 2);
        gStorage->waveformSprites[i] = &gSprites[spriteId];
    }
}

static void RefreshDisplayMonData(void)
{
    LoadDisplayMonGfx(gStorage->displayMonSpecies, gStorage->displayMonPersonality);
    PrintDisplayMonInfo();
    UpdateWaveformAnimation();
    ScheduleBgCopyTilemapToVram(0);
}

static void StartDisplayMonMosaic(void)
{
    RefreshDisplayMonData();
    if (gStorage->displayMonSprite)
    {
        gStorage->displayMonSprite->oam.mosaic = TRUE;
        gStorage->displayMonSprite->data[0] = 10;
        gStorage->displayMonSprite->data[1] = 1;
        gStorage->displayMonSprite->callback = SpriteCB_DisplayMonMosaic;
        SetGpuReg(REG_OFFSET_MOSAIC, (gStorage->displayMonSprite->data[0] << 12) | (gStorage->displayMonSprite->data[0] << 8));
    }
}

static u8 IsDisplayMonMosaicActive(void)
{
    return gStorage->displayMonSprite->oam.mosaic;
}

static void SpriteCB_DisplayMonMosaic(struct Sprite *sprite)
{
    sprite->data[0] -= sprite->data[1];
    if (sprite->data[0] < 0)
        sprite->data[0] = 0;
    SetGpuReg(REG_OFFSET_MOSAIC, (sprite->data[0] << 12) | (sprite->data[0] << 8));
    if (sprite->data[0] == 0)
    {
        sprite->oam.mosaic = FALSE;
        sprite->callback = SpriteCallbackDummy;
    }
}

static void CreateDisplayMonSprite(void)
{
    u16 i;
    u16 tileStart;
    u8 palSlot;
    u8 spriteId;
    struct SpriteSheet sheet = {gStorage->tileBuffer, MON_PIC_SIZE, GFXTAG_DISPLAY_MON};
    struct SpritePalette palette = {gStorage->displayMonPalBuffer, PALTAG_DISPLAY_MON};
    struct SpriteTemplate template = sSpriteTemplate_DisplayMon;

    for (i = 0; i < MON_PIC_SIZE; i++)
        gStorage->tileBuffer[i] = 0;
    for (i = 0; i < 0x10; i++)
        gStorage->displayMonPalBuffer[i] = 0;

    gStorage->displayMonSprite = NULL;

    do
    {
        tileStart = LoadSpriteSheet(&sheet);
        if (tileStart == 0)
            break;

        palSlot = LoadSpritePalette(&palette);
        if (palSlot == 0xFF)
            break;

        spriteId = CreateSprite(&template, 40, 48, 0);
        if (spriteId == MAX_SPRITES)
            break;

        gStorage->displayMonSprite = &gSprites[spriteId];
        gStorage->displayMonPalOffset = palSlot * 16 + 0x100;
        gStorage->displayMonTilePtr = (void *)OBJ_VRAM0 + tileStart * 32;
    } while (FALSE);

    if (gStorage->displayMonSprite == NULL)
    {
        FreeSpriteTilesByTag(GFXTAG_DISPLAY_MON);
        FreeSpritePaletteByTag(PALTAG_DISPLAY_MON);
    }
}

static void LoadDisplayMonGfx(u16 species, u32 personality)
{
    if (gStorage->displayMonSprite == NULL)
        return;

    if (species != SPECIES_NONE)
    {
        HandleLoadSpecialPokePic(&gMonFrontPicTable[species], gStorage->tileBuffer, species, personality);
        LZ77UnCompWram(gStorage->displayMonPalette, gStorage->displayMonPalBuffer);
        CpuCopy32(gStorage->tileBuffer, gStorage->displayMonTilePtr, 0x800);
        LoadPalette(gStorage->displayMonPalBuffer, gStorage->displayMonPalOffset, 0x20);
        gStorage->displayMonSprite->invisible = FALSE;
    }
    else
        gStorage->displayMonSprite->invisible = TRUE;
}

static void PrintDisplayMonInfo(void)
{
    u16 i;
    u16 y;
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    if (gStorage->boxOption != OPTION_MOVE_ITEMS)
    {
        for (i = 0, y = 0; i < 3; i++, y += 14)
            AddTextPrinterParameterized(0, FONT_2, gStorage->displayMonTexts[i], i == 2 ? 10 : 6, y, TEXT_SKIP_DRAW, NULL);

        AddTextPrinterParameterized(0, FONT_0, gStorage->displayMonTexts[3], 6, y + 2, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        AddTextPrinterParameterized(0, FONT_0, gStorage->displayMonTexts[3], 6, 0, TEXT_SKIP_DRAW, NULL);
        for (i = 0, y = 15; i < 3; i++, y += 14)
            AddTextPrinterParameterized(0, FONT_2, gStorage->displayMonTexts[i], i == 2 ? 10 : 6, y, TEXT_SKIP_DRAW, NULL);
    }

    CopyWindowToVram(0, COPYWIN_GFX);
    if (gStorage->displayMonSpecies != SPECIES_NONE)
    {
        UpdateMonMarkingTiles(gStorage->displayMonMarkings, gStorage->markingComboTilesPtr);
        gStorage->markingComboSprite->invisible = FALSE;
    }
    else
        gStorage->markingComboSprite->invisible = TRUE;
}

static void UpdateWaveformAnimation(void)
{
    u16 i;

    if (gStorage->displayMonSpecies != SPECIES_NONE)
    {
        TilemapUtil_SetRect(TILEMAP_PKMN_DATA, 0, 0, 8, 2);
        for (i = 0; i < 2; i++)
            StartSpriteAnimIfDifferent(gStorage->waveformSprites[i], i * 2 + 1);
    }
    else
    {
        TilemapUtil_SetRect(TILEMAP_PKMN_DATA, 0, 2, 8, 2);
        for (i = 0; i < 2; i++)
            StartSpriteAnim(gStorage->waveformSprites[i], i * 2);
    }

    TilemapUtil_Update(TILEMAP_PKMN_DATA);
    ScheduleBgCopyTilemapToVram(1);
}

static void InitSupplementalTilemaps(void)
{
    LZ77UnCompWram(gPokeStoragePartyMenu_Tilemap, gStorage->partyMenuTilemapBuffer);
    LoadPalette(gPokeStoragePartyMenu_Pal, 0x10, 0x20);
    TilemapUtil_SetTilemap(TILEMAP_PARTY_MENU, 1, gStorage->partyMenuTilemapBuffer, 12, 22);
    TilemapUtil_SetTilemap(TILEMAP_CLOSE_BUTTON, 1, sCloseBoxButton_Tilemap, 9, 4);
    TilemapUtil_SetPos(TILEMAP_PARTY_MENU, 10, 0);
    TilemapUtil_SetPos(TILEMAP_CLOSE_BUTTON, 21, 0);
    SetPartySlotTilemaps();
    if (sInPartyMenu)
    {
        UpdateCloseBoxButtonTilemap(TRUE);
        CreatePartyMonsSprites(TRUE);
        TilemapUtil_Update(TILEMAP_CLOSE_BUTTON);
        TilemapUtil_Update(TILEMAP_PARTY_MENU);
    }
    else
    {
        TilemapUtil_SetRect(TILEMAP_PARTY_MENU, 0, 20, 12, 2);
        UpdateCloseBoxButtonTilemap(TRUE);
        TilemapUtil_Update(TILEMAP_PARTY_MENU);
        TilemapUtil_Update(TILEMAP_CLOSE_BUTTON);
    }

    ScheduleBgCopyTilemapToVram(1);
    gStorage->closeBoxFlashing = FALSE;
}

static void SetUpShowPartyMenu(void)
{
    gStorage->partyMenuUnused1 = 20;
    gStorage->partyMenuY = 2;
    gStorage->partyMenuMoveTimer = 0;
    CreatePartyMonsSprites(FALSE);
}

static bool8 ShowPartyMenu(void)
{
    if (gStorage->partyMenuMoveTimer == 20)
        return FALSE;

    gStorage->partyMenuUnused1--;
    gStorage->partyMenuY++;
    TilemapUtil_Move(TILEMAP_PARTY_MENU, 3, 1);
    TilemapUtil_Update(TILEMAP_PARTY_MENU);
    ScheduleBgCopyTilemapToVram(1);
    MovePartySprites(8);
    if (++gStorage->partyMenuMoveTimer == 20)
    {
        sInPartyMenu = TRUE;
        return FALSE;
    }
    else
        return TRUE;
}

static void SetUpHidePartyMenu(void)
{
    gStorage->partyMenuUnused1 = 0;
    gStorage->partyMenuY = 22;
    gStorage->partyMenuMoveTimer = 0;
    if (gStorage->boxOption == OPTION_MOVE_ITEMS)
        MoveHeldItemWithPartyMenu();
}

static bool8 HidePartyMenu(void)
{
    if (gStorage->partyMenuMoveTimer != 20)
    {
        gStorage->partyMenuUnused1++;
        gStorage->partyMenuY--;
        TilemapUtil_Move(TILEMAP_PARTY_MENU, 3, -1);
        TilemapUtil_Update(TILEMAP_PARTY_MENU);
        FillBgTilemapBufferRect_Palette0(1, 0x100, 10, gStorage->partyMenuY, 12, 1);
        MovePartySprites(-8);
        if (++gStorage->partyMenuMoveTimer != 20)
        {
            ScheduleBgCopyTilemapToVram(1);
            return TRUE;
        }
        else
        {
            sInPartyMenu = FALSE;
            DestroyAllPartyMonIcons();
            CompactPartySlots();
            TilemapUtil_SetRect(TILEMAP_CLOSE_BUTTON, 0, 0, 9, 2);
            TilemapUtil_Update(TILEMAP_CLOSE_BUTTON);
            ScheduleBgCopyTilemapToVram(1);
            return FALSE;
        }
    }

    return FALSE;
}

static void UpdateCloseBoxButtonTilemap(bool8 normal)
{
    if (normal)
        TilemapUtil_SetRect(TILEMAP_CLOSE_BUTTON, 0, 0, 9, 2);
    else
        TilemapUtil_SetRect(TILEMAP_CLOSE_BUTTON, 0, 2, 9, 2);

    TilemapUtil_Update(TILEMAP_CLOSE_BUTTON);
    ScheduleBgCopyTilemapToVram(1);
}

static void StartFlashingCloseBoxButton(void)
{
    gStorage->closeBoxFlashing = TRUE;
    gStorage->closeBoxFlashTimer = 30;
    gStorage->closeBoxFlashState = TRUE;
}

static void StopFlashingCloseBoxButton(void)
{
    if (gStorage->closeBoxFlashing)
    {
        gStorage->closeBoxFlashing = FALSE;
        UpdateCloseBoxButtonTilemap(TRUE);
    }
}

static void UpdateCloseBoxButtonFlash(void)
{
    if (gStorage->closeBoxFlashing && ++gStorage->closeBoxFlashTimer > 30)
    {
        gStorage->closeBoxFlashTimer = 0;
        gStorage->closeBoxFlashState = (gStorage->closeBoxFlashState == FALSE);
        UpdateCloseBoxButtonTilemap(gStorage->closeBoxFlashState);
    }
}

static void SetPartySlotTilemaps(void)
{
    u8 i;

    // Skips first party slot, it should always be drawn
    // as if it has a Pokémon in it
    for (i = 1; i < PARTY_SIZE; i++)
    {
        s32 species = GetMonData(gPlayerParty + i, MON_DATA_SPECIES);
        SetPartySlotTilemap(i, (species != SPECIES_NONE));
    }
}

static void SetPartySlotTilemap(u8 pos, bool8 isFilled)
{
    u16 i, j, index;
    const u16 *tilemap;

    if (isFilled)
        tilemap = sPartySlotFilled_Tilemap;
    else
        tilemap = sPartySlotEmpty_Tilemap;

    index = 3 * (3 * (pos - 1) + 1);
    index *= 4;
    index += 7;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 4; j++)
            gStorage->partyMenuTilemapBuffer[index + j] = tilemap[j];

        tilemap += 4;
        index += 12;
    }
}

static void UpdatePartySlotColors(void)
{
    SetPartySlotTilemaps();
    TilemapUtil_SetRect(TILEMAP_PARTY_MENU, 0, 0, 12, 22);
    TilemapUtil_Update(TILEMAP_PARTY_MENU);
    ScheduleBgCopyTilemapToVram(1);
}

static void SetUpDoShowPartyMenu(void)
{
    gStorage->showPartyMenuState = 0;
    PlaySE(SE_WIN_OPEN);
    SetUpShowPartyMenu();
}

static bool8 DoShowPartyMenu(void)
{
    switch (gStorage->showPartyMenuState)
    {
    case 0:
        if (!ShowPartyMenu())
        {
            SetCursorInParty();
            gStorage->showPartyMenuState++;
        }
        break;
    case 1:
        if (!UpdateCursorPos())
        {
            if (gStorage->setMosaic)
                StartDisplayMonMosaic();
            gStorage->showPartyMenuState++;
        }
        break;
    case 2:
        return FALSE;
    }
    return TRUE;
}

static void InitPokeStorageBg0(void)
{
    SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(29));
    TextWindow_SetStdFrame0_WithPal(1, 2, 0xD0);
    FillBgTilemapBufferRect(0, 0, 0, 0, 32, 20, 17);
    CopyBgTilemapBufferToVram(0);
}

static void PrintStorageMessage(u8 id)
{
    u8 *txtPtr;

    DynamicPlaceholderTextUtil_Reset();
    switch (sMessages[id].format)
    {
    case MSG_FMT_NONE:
        break;
    case MSG_FMT_MON_NAME_1:
    case MSG_FMT_MON_NAME_2:
    case MSG_FMT_MON_NAME_3:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->displayMonNickname);
        break;
    case MSG_FMT_RELEASE_MON_1:
    case MSG_FMT_RELEASE_MON_2:
    case MSG_FMT_RELEASE_MON_3:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->releaseMonName);
        break;
    case MSG_FMT_ITEM_NAME:
        if (IsActiveItemMoving())
            txtPtr = StringCopy(gStorage->itemName, GetMovingItemName());
        else
            txtPtr = StringCopy(gStorage->itemName, gStorage->displayMonTexts[3]);

        while (*(txtPtr - 1) == CHAR_SPACE)
            txtPtr--;

        *txtPtr = EOS;
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->itemName);
        break;
    }

    DynamicPlaceholderTextUtil_ExpandPlaceholders(gStorage->actionText, sMessages[id].text);
    FillWindowPixelBuffer(1, PIXEL_FILL(1));
    AddTextPrinterParameterized(1, FONT_1, gStorage->actionText, 0, 2, TEXT_SKIP_DRAW, NULL);
    DrawTextBorderOuter(1, 2, 13);
    PutWindowTilemap(1);
    CopyWindowToVram(1, COPYWIN_GFX);
    ScheduleBgCopyTilemapToVram(0);
}

static void PrintReleaseMessage(u8 id)
{
    u8 *txtPtr;

    DynamicPlaceholderTextUtil_Reset();
    switch (sReleaseMessages[id].format)
    {
    case MSG_FMT_NONE:
        break;
    case MSG_FMT_MON_NAME_1:
    case MSG_FMT_MON_NAME_2:
    case MSG_FMT_MON_NAME_3:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->displayMonNickname);
        break;
    case MSG_FMT_RELEASE_MON_1:
    case MSG_FMT_RELEASE_MON_2:
    case MSG_FMT_RELEASE_MON_3:
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->releaseMonName);
        break;
    case MSG_FMT_ITEM_NAME:
        if (IsActiveItemMoving())
            txtPtr = StringCopy(gStorage->itemName, GetMovingItemName());
        else
            txtPtr = StringCopy(gStorage->itemName, gStorage->displayMonTexts[3]);

        while (*(txtPtr - 1) == CHAR_SPACE)
            txtPtr--;

        *txtPtr = EOS;
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gStorage->itemName);
        break;
    }

    DynamicPlaceholderTextUtil_ExpandPlaceholders(gStorage->actionText, sReleaseMessages[id].text);
    FillWindowPixelBuffer(1, PIXEL_FILL(1));
    AddTextPrinterParameterized(1, FONT_1, gStorage->actionText, 0, 2, TEXT_SKIP_DRAW, NULL);
    DrawTextBorderOuter(1, 2, 13);
    PutWindowTilemap(1);
    CopyWindowToVram(1, COPYWIN_GFX);
    ScheduleBgCopyTilemapToVram(0);
}

static void ShowYesNoWindow(s8 cursorPos)
{
    CreateYesNoMenu(&sYesNoWindowTemplate, FONT_1, 0, 2, 11, 14, 1);
    Menu_MoveCursorNoWrapAround(cursorPos);
}

static void ClearBottomWindow(void)
{
    ClearStdWindowAndFrameToTransparent(1, FALSE);
    ScheduleBgCopyTilemapToVram(0);
}

static void AddWallpaperSetsMenu(void)
{
    InitMenu();
    SetMenuText(MENU_TEXT_SCENERY_1);
    SetMenuText(MENU_TEXT_SCENERY_2);
    SetMenuText(MENU_TEXT_SCENERY_3);
    SetMenuText(MENU_TEXT_ETCETERA);
    AddMenu();
}

static void AddWallpapersMenu(u8 wallpaperSet)
{
    InitMenu();
    switch (wallpaperSet)
    {
    case 0: // SCENERY_1
        SetMenuText(MENU_TEXT_FOREST);
        SetMenuText(MENU_TEXT_CITY);
        SetMenuText(MENU_TEXT_DESERT);
        SetMenuText(MENU_TEXT_SAVANNA);
        break;
    case 1: // SCENERY_2
        SetMenuText(MENU_TEXT_CRAG);
        SetMenuText(MENU_TEXT_VOLCANO);
        SetMenuText(MENU_TEXT_SNOW);
        SetMenuText(MENU_TEXT_CAVE);
        break;
    case 2: // SCENERY_3
        SetMenuText(MENU_TEXT_BEACH);
        SetMenuText(MENU_TEXT_SEAFLOOR);
        SetMenuText(MENU_TEXT_RIVER);
        SetMenuText(MENU_TEXT_SKY);
        break;
    case 3: // ETCETERA
        SetMenuText(MENU_TEXT_POLKADOT);
        SetMenuText(MENU_TEXT_POKECENTER);
        SetMenuText(MENU_TEXT_MACHINE);
        SetMenuText(MENU_TEXT_SIMPLE);
        break;
    }
    AddMenu();
}

u8 GetCurrentBoxOption(void)
{
    return sCurrentBoxOption;
}

static void InitCursorItemIcon(void)
{
    if (!IsCursorOnBoxTitle())
    {
        if (sInPartyMenu)
            TryLoadItemIconAtPos(CURSOR_AREA_IN_PARTY, GetBoxCursorPosition());
        else
            TryLoadItemIconAtPos(CURSOR_AREA_IN_BOX, GetBoxCursorPosition());
    }

    if (sMovingItemId != ITEM_NONE)
    {
        InitItemIconInCursor(sMovingItemId);
        StartCursorAnim(CURSOR_ANIM_FIST);
    }
}

static void SetPokeStorageQuestLogEvent(u8 action)
{
    u16 event;
    struct PokeStorageQuestLogData *questLogData;
    u8 box1 = GetMovingMonOriginalBoxId();
    u16 species1 = gStorage->displayMonSpecies;
    u16 species2;
    u8 box2;
    if (sInPartyMenu)
    {
        box2 = TOTAL_BOXES_COUNT;
        species2 = GetMonData(&gPlayerParty[GetBoxCursorPosition()], MON_DATA_SPECIES2);
    }
    else
    {
        box2 = StorageGetCurrentBox();
        species2 = GetCurrentBoxMonData(GetBoxCursorPosition(), MON_DATA_SPECIES2);
    }
    questLogData = &gStorage->pokeStorageQuestLogData;

    switch (action)
    {
    default:
        return;
    case 0:
        if (sInPartyMenu)
        {
            if (box1 == TOTAL_BOXES_COUNT)
                return;
            else
                event = QL_EVENT_SWITCHED_PARTY_MON_FOR_PC_MON;
        }
        else
        {
            if (box1 == TOTAL_BOXES_COUNT)
                // Should upmerge but doesn't
                event = QL_EVENT_SWITCHED_PARTY_MON_FOR_PC_MON;
            else
                event = box1 != box2 ? QL_EVENT_SWITCHED_MONS_BETWEEN_BOXES : QL_EVENT_SWITCHED_MONS_WITHIN_BOX;
        }
        questLogData->species1 = species1;
        questLogData->species2 = species2;
        questLogData->box1 = box1;
        questLogData->box2 = box2;
        break;
    case 1:
        questLogData->species1 = species1;
        questLogData->species2 = SPECIES_NONE;
        questLogData->box1 = box1;
        questLogData->box2 = 0xFF;
        if (sInPartyMenu)
        {
            if (box1 == TOTAL_BOXES_COUNT)
                return;
            else
                event = QL_EVENT_WITHDREW_MON_PC;
        }
        else
        {
            if (box1 == TOTAL_BOXES_COUNT)
            {
                event = QL_EVENT_DEPOSITED_MON_PC;
                questLogData->box1 = box2;
            }
            else if (box1 != box2)
            {
                event = QL_EVENT_MOVED_MON_BETWEEN_BOXES;
                questLogData->box2 = box2;
            }
            else
                event = QL_EVENT_MOVED_MON_WITHIN_BOX;
        }
        break;
    case 2:
        event = QL_EVENT_DEPOSITED_MON_PC;
        questLogData->species1 = species1;
        questLogData->species2 = SPECIES_NONE;
        questLogData->box1 = sDepositBoxId;
        questLogData->box2 = 0xFF;
        break;
    case 3:
        event = QL_EVENT_SWITCHED_MULTIPLE_MONS;
        questLogData->species1 = SPECIES_NONE;
        questLogData->species2 = SPECIES_NONE;
        questLogData->box1 = box1;
        questLogData->box2 = box2;
        break;
    }
    SetQuestLogEvent(event, (const u16 *)questLogData);
}

static void UpdateBoxToSendMons(void)
{
    if (sLastUsedBox != StorageGetCurrentBox())
    {
        FlagClear(FLAG_SHOWN_BOX_WAS_FULL_MESSAGE);
        VarSet(VAR_PC_BOX_TO_SEND_MON, StorageGetCurrentBox());
    }
}
