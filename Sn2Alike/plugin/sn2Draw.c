/*
 * Sn2Alike - the editor panel.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/sn2Draw.md - "// notes §k" refers there. Pulse2Alike's panel, without its presets,
// hardware link and arpeggiator steps (notes §1).

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sn2Draw.h"
#include "sn2Hardware.h"
#include "sn2Params.h"

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "geometry.h"
#include "synthlibHost.h"
#include "synthlibTypes.h"
#include "utilsGraphics.h"
#include "renderBackend.h"
#include "contextMenu.h"
#include "buttonPress.h"
#include "fileBrowser.h"
#include "bankBrowser.h"
#include "alertDialog.h"
#include "menuBar.h"
#include "synthlibPopups.h"

#include "sn2Menu.h"
#include "sn2Patch.h"
#include "sn2Prefs.h"

#define GL_SILENCE_DEPRECATION    1
#include <GLFW/glfw3.h>           // the key constants SynthLib's popups speak

#if defined (_WIN32)
#define FONT_PATH               alike_font_path()     // tools/windows/alikeWin.c: Arial from the Windows fonts folder
#else
#define FONT_PATH               "/System/Library/Fonts/Supplemental/Arial.ttf"
#endif
#define FONT_PRELOAD_SIZE       (16.0)

#define MAX_PARAMS              (200)
#define MAX_PLACED              (64)
#define MAX_PAGES               (16)
#define MARGIN_X                (16.0)
#define TITLE_Y                 (30.0)       // below the menu bar
#define TAB_Y                   (58.0)
#define TAB_H                   (14.0)
#define TAB_GAP                 (6.0)
#define TAB_PAD                 (4.0)        // either side of a tab's title
#define PAGE_Y                  (90.0)
#define ROW_H                   (112.0)
#define SECTION_GAP             (16.0)
#define CONTROL_W               (58.0)
#define DIAL_SIZE               (40.0)
#define LABEL_H                 (11.0)
#define HEADING_TO_DIAL         (34.0)
#define BUTTON_H                (12.0)
#define PATCH_BAR_X             (300.0)
#define PATCH_BUTTON_H          (14.0)
#define PATCH_BUTTON_GAP        (6.0)
#define NAME_FIELD_SIZING       "WWWWWWWWWW"     // SN2_SINGLE_NAME_LENGTH of the widest capital
#define PRESET_COUNT_TEXT       "1536"
#define MENU_BG                 {0.22, 0.22, 0.25}
#define MENU_MAX                (32)
#define MENU_CELL_W             (96.0)
#define MENU_ROWS_PER_COLUMN    (8)
#define OPENING_RELEASE_SLOP    (4.0)     // notes §3: how far the pointer may move and still be the opening click

typedef struct {
    const tSn2Control * control;
    const tSn2Section * section;
    tRectangle          dial;         // the dial's square; a choice's button sits across its middle
    double              x;            // the control's column
} tPlaced;

typedef struct {
    const tSn2Section * section;
    double              x, y, w;
} tPlacedSection;

static const tSn2Page *      gPages;
static uint32_t              gPageCount;
static uint32_t              gPage;
static double                gValue[MAX_PARAMS]; // normalized, as the host holds them
static bool                  gFontReady;
static tCoord                gMouse;

static tPlaced               gPlaced[MAX_PLACED];
static uint32_t              gPlacedCount;
static tPlacedSection        gPlacedSections[MAX_PLACED];
static uint32_t              gPlacedSectionCount;
static tRectangle            gTabs[MAX_PAGES];

// the dial being dragged; its value runs unrounded while it moves
static const tSn2Control *   gGrabbed;
static double                gGrabValue;
static tRectangle            gGrabDial;         // the grabbed dial's square, for a rotary drag

// notes §7: the open value menu - which control it sets, and what was chosen
static const tSn2Control *   gMenuFor;
static int                   gMenuChoice = -1;
static tMenuItem             gMenuItems[MENU_MAX + 1];

static bool                  gOpeningClick;  // notes §3: the press that opened a menu, until its release
static tCoord                gOpeningAt;

// notes §9: the patch bar
static const tSn2PatchHost * gHost;
static void *                gUser;
static char                  gPatchPath[1024];
static int                   gPatchIndex;
static int                   gPreset;
static bool                  gPresetEdited;
static tRectangle            gPrevRect, gNextRect, gLoadRect, gNameRect, gLinkRect;

// notes §1 (SynthLib buttonPress.h): lit while held, acts on the release
static tRectangle draw_held_button(const tRectangle * rect, const char * text, tRgb colour) {
    return draw_button(mainArea, *rect, text, synthlib_button_held(rect) ? synthlib_button_held_colour(colour) : colour);
}
static bool                  gLinkBusy;

// notes §9: the Singles of the file the sound came from, read again only when the file changes
static uint8_t               gSingles[SN2_MAX_SINGLES][SN2_SINGLE_LENGTH];
static int                   gSingleCount;
static char                  gSinglesPath[1024];

// notes §10: the sound's name, edited in place as Pulse2Alike's is
static struct {
    bool     active;
    char     buffer[SN2_SINGLE_NAME_LENGTH + 1];
    uint32_t cursor;
}                            gNameEdit;

static void menu_action(int index) {
    gMenuChoice = index;
}

static void mouse_coord(tCoord * coord) {
    if (coord != NULL) {
        *coord = gMouse;
    }
}

static bool pointer_captured(void) {
    return gGrabbed != NULL;
}

void sn2_draw_init(void) {
    synthlib_host_init((tSynthLibHost){.mouseCoord = mouse_coord, .pointerCaptured = pointer_captured});
    synthlib_popups_set_menu_bar(sn2_menu_bar(), sn2_menu_bar_rect);
    gFontReady = preload_glyph_textures(FONT_PATH, FONT_PRELOAD_SIZE);
}

void sn2_draw_set_pages(const tSn2Page * pages, uint32_t count) {
    gPages     = pages;
    gPageCount = (count < MAX_PAGES) ? count : MAX_PAGES;

    if (gPage >= gPageCount) {
        gPage = 0;
    }
}

static tSn2DialDisplay gDisplay;

static bool            gClip;

void sn2_draw_set_clip(bool on) {
    gClip = on;
}

void sn2_draw_set_link_busy(bool busy) {
    gLinkBusy = busy;
}

void sn2_draw_set_display(tSn2DialDisplay display) {
    gDisplay = display;
}

void sn2_draw_set_value(uint32_t paramId, double normalized) {
    if (paramId < MAX_PARAMS) {
        gValue[paramId] = normalized;
    }
}

void sn2_draw_set_mouse(double x, double y) {
    gMouse.x = x;
    gMouse.y = y;
    (void)handle_file_browser_mouse_move(gMouse);
    (void)handle_bank_browser_mouse_move(gMouse);   // its scrollbar drags only while it hears the mouse move
}

bool sn2_draw_menu_active(void) {
    return gContextMenu.active || synthlib_popups_modal_active();
}

// ---- Singles - notes §9 -----------------------------------------------------------------------------------

void sn2_draw_set_host(const tSn2PatchHost * host) {
    gHost = host;
}

void sn2_draw_set_user(void * user) {
    gUser = user;
}

static void read_singles(const char * path) {
    if (strcmp(path, gSinglesPath) != 0) {
        snprintf(gSinglesPath, sizeof(gSinglesPath), "%s", path);
        gSingleCount = (path[0] != '\0') ? sn2_patch_read_file(path, gSingles, SN2_MAX_SINGLES) : 0;
    }
}

// notes §9: no reading here - it runs every frame, while the bank browser may be choosing from another file
void sn2_draw_set_patch(const char * path, int index) {
    snprintf(gPatchPath, sizeof(gPatchPath), "%s", (path != NULL) ? path : "");
    gPatchIndex = index;
}

static const char * host_name(void) {
    return ((gHost != NULL) && (gUser != NULL)) ? gHost->name(gUser) : "";
}

static void load_single(int index) {
    if ((gHost != NULL) && (gUser != NULL) && (index >= 0) && (index < gSingleCount)) {
        gHost->load(gUser, gSingles[index], gSinglesPath, index, 0);
        snprintf(gPatchPath, sizeof(gPatchPath), "%s", gSinglesPath);
        gPatchIndex = index;
        gPreset     = 0;   // a file is not a preset until it is stored to one
    }
}

static tBankBrowserItem gSingleItems[SN2_MAX_SINGLES];
static char             gSingleNames[SN2_MAX_SINGLES][SN2_SINGLE_NAME_LENGTH + 8];

static void on_single_chosen(bool confirmed, uint32_t bank1Indexed, uint32_t location1Indexed) {
    if (confirmed) {
        load_single((int)(((bank1Indexed - 1) * SN2_BANK_SIZE) + (location1Indexed - 1)));
    }
}

// notes §9: a file of several Singles opens SynthLib's bank browser, in banks of 128
static void choose_single(void) {
    char name[SN2_SINGLE_NAME_LENGTH + 1];

    for (int i = 0; i < gSingleCount; i++) {
        sn2_patch_name(gSingles[i], name);
        snprintf(gSingleNames[i], sizeof(gSingleNames[0]), "%03d %s", (i % SN2_BANK_SIZE) + 1, name);
        gSingleItems[i] = (tBankBrowserItem){
            gSingleNames[i], 0, (uint32_t)((i / SN2_BANK_SIZE) + 1), (uint32_t)((i % SN2_BANK_SIZE) + 1)
        };
    }

    open_bank_browser("Load Single", strrchr(gSinglesPath, '/') + 1, "Load", gSingleItems, (uint32_t)gSingleCount, NULL, 0, on_single_chosen);
}

static void open_file(const char * path) {
    static char message[1200];

    read_singles(path);

    if (gSingleCount == 0) {
        snprintf(message, sizeof(message), "No Singles found in %s", path);
        show_alert("Open Patch", message);
        gSinglesPath[0] = '\0';
    } else if (gSingleCount == 1) {
        load_single(0);
    } else {
        choose_single();
    }
}

static void on_file_chosen(const char * path) {
    if ((path != NULL) && (path[0] != '\0')) {
        open_file(path);
    }
}

static const char * browser_start(void) {
    static char folder[1024];

    snprintf(folder, sizeof(folder), "%s", gPatchPath);
    char *      slash = strrchr(folder, '/');

    if (slash != NULL) {
        *slash = '\0';
        return folder;
    }
    return sn2_patch_sounds_folder();
}

void sn2_draw_open_patch(void) {
    set_file_browser_start_directory_provider(browser_start);
    open_file_browser_read(on_file_chosen);
}

bool sn2_draw_has_patch_file(void) {
    read_singles(gPatchPath);
    return (strchr(gPatchPath, '/') != NULL) && (gSingleCount == 1);
}

static void save_to(const char * path) {
    char failed[1400];

    if ((gHost != NULL) && gHost->save(gUser, path)) {
        gSinglesPath[0] = '\0';   // its bytes changed
        sn2_draw_set_patch(path, 0);
        gPreset         = 0;
        return;
    }
    snprintf(failed, sizeof(failed), "Could not write %s", path);
    show_alert("Save Patch", failed);
}

static void on_save_chosen(const char * path) {
    char   full[1100];
    size_t n;

    if ((path == NULL) || (path[0] == '\0')) {
        return;
    }
    snprintf(full, sizeof(full), "%s", path);
    n = strlen(full);

    if (((n < 4) || (strcasecmp(full + n - 4, ".syx") != 0)) && (n + 4 < sizeof(full))) {
        strcat(full, ".syx");
    }
    save_to(full);
}

static void on_overwrite_confirmed(bool confirmed) {
    if (confirmed) {
        save_to(gPatchPath);
    }
}

void sn2_draw_save_patch(bool chooseFile) {
    static char message[1200];
    static char suggested[64];

    if (!chooseFile && sn2_draw_has_patch_file()) {
        snprintf(message, sizeof(message), "Overwrite \"%s\" with the sound as it is now?", strrchr(gPatchPath, '/') + 1);
        show_confirm("Save Patch", message, "Save", on_overwrite_confirmed);
        return;
    }
    snprintf(suggested, sizeof(suggested), "%s.syx", (host_name()[0] != '\0') ? host_name() : "Untitled");
    set_file_browser_start_directory_provider(browser_start);
    open_file_browser_write(on_save_chosen, suggested);
}

static void on_init_confirmed(bool confirmed) {
    if (confirmed && (gHost != NULL)) {
        gHost->init(gUser);
        sn2_draw_set_patch("", 0);
    }
}

void sn2_draw_init_patch(void) {
    show_confirm("Init Patch", "Set every sound parameter to its default? No file is changed.", "Init", on_init_confirmed);
}

// ---- the presets - notes §11 ------------------------------------------------------------------------------

void sn2_draw_set_preset(int number, bool edited) {
    gPreset       = number;
    gPresetEdited = edited;
}

int sn2_draw_preset(void) {
    return gPreset;
}

static void load_preset(int number) {
    if ((gHost != NULL) && (gUser != NULL) && (number >= 1) && (number <= SN2_PRESETS)) {
        (void)sn2_presets_refresh();
        gHost->load(gUser, sn2_presets_single(number), "", 0, number);
        gPreset       = number;
        gPresetEdited = false;
        sn2_draw_set_patch("", 0);
    }
}

static tBankBrowserItem gSlotItems[SN2_PRESETS];
static void fill_slot_items(void);

// notes §11: Prev/Next step through the presets in the order the browser lists them under the Sort menu, and wrap;
// from no preset, Next is the first and Prev the last
static void step_patch(int direction) {
    static uint32_t order[SN2_PRESETS];
    int             at   = -1;

    fill_slot_items();
    bank_browser_sorted_order(gSlotItems, SN2_PRESETS, NULL, 0, bank_browser_sort_mode(), order);

    for (int i = 0; (i < SN2_PRESETS) && (gPreset != 0); i++) {
        if ((int)order[i] == (gPreset - 1)) {
            at = i;
            break;
        }
    }

    int             next = (at < 0) ? ((direction > 0) ? 0 : (SN2_PRESETS - 1)) : ((((at + direction) % SN2_PRESETS) + SN2_PRESETS) % SN2_PRESETS);

    load_preset((int)order[next] + 1);
}

static int gStoreTo;

static void on_store_confirmed(bool confirmed) {
    char failed[1200];

    if (!confirmed || (gHost == NULL) || (gUser == NULL)) {
        return;
    }

    if (gHost->storePreset(gUser, gStoreTo)) {
        gPreset       = gStoreTo;
        gPresetEdited = false;
        sn2_draw_set_patch("", 0);
        return;
    }
    snprintf(failed, sizeof(failed), "Could not write %s", sn2_presets_path());
    show_alert("Store Preset", failed);
}

// notes §11: always confirmed, naming what the slot holds now
void sn2_draw_store_preset(int number) {
    static char message[160];
    char        label[8];

    gStoreTo = (number > 0) ? number : gPreset;

    if ((gStoreTo < 1) || (gStoreTo > SN2_PRESETS)) {
        return;
    }
    (void)sn2_presets_refresh();
    sn2_preset_label(gStoreTo, label, sizeof(label));
    snprintf(message, sizeof(message), "Store \"%s\" to %s, replacing \"%s\"?", host_name(), label, sn2_presets_name(gStoreTo));
    show_confirm("Store Preset", message, "Store", on_store_confirmed);
}

static char gSlotNames[SN2_PRESETS][SN2_SINGLE_NAME_LENGTH + 8];

static void fill_slot_items(void) {
    (void)sn2_presets_refresh();

    for (int n = 1; n <= SN2_PRESETS; n++) {
        snprintf(gSlotNames[n - 1], sizeof(gSlotNames[0]), "%s", sn2_presets_name(n));   // the browser shows the slot
        gSlotItems[n - 1] = (tBankBrowserItem){
            gSlotNames[n - 1], 0, (uint32_t)(((n - 1) / SN2_BANK_SIZE) + 1), (uint32_t)(((n - 1) % SN2_BANK_SIZE) + 1)
        };
    }
}

static int slot_number(uint32_t bank1Indexed, uint32_t location1Indexed) {
    return (int)(((bank1Indexed - 1) * SN2_BANK_SIZE) + location1Indexed);
}

static void on_load_slot(bool confirmed, uint32_t bank1Indexed, uint32_t location1Indexed) {
    if (confirmed) {
        load_preset(slot_number(bank1Indexed, location1Indexed));
    }
}

static void on_store_slot(bool confirmed, uint32_t bank1Indexed, uint32_t location1Indexed) {
    if (confirmed) {
        sn2_draw_store_preset(slot_number(bank1Indexed, location1Indexed));
    }
}

void sn2_draw_load_preset_browser(void) {
    fill_slot_items();
    open_bank_browser_lettered("Load Preset", "A000-L127, as banks A-L of 128.", "Load", gSlotItems, SN2_PRESETS, NULL, 0, 0, on_load_slot);
}

void sn2_draw_store_preset_browser(void) {
    fill_slot_items();
    open_bank_browser_lettered("Store Preset", "The slot to store the sound as it is now into.", "Store...", gSlotItems, SN2_PRESETS, NULL, 0, 0,
                               on_store_slot);
}

static void on_revert_confirmed(bool confirmed) {
    if (confirmed) {
        load_preset(gPreset);
    }
}

void sn2_draw_revert_preset(void) {
    static char message[160];
    char        label[8];

    if (gPreset == 0) {
        return;
    }
    (void)sn2_presets_refresh();
    sn2_preset_label(gPreset, label, sizeof(label));
    snprintf(message, sizeof(message), "Discard the edits and go back to %s \"%s\"?", label, sn2_presets_name(gPreset));
    show_confirm("Revert to Stored Preset", message, "Revert", on_revert_confirmed);
}

// ---- import and export - notes §12 ------------------------------------------------------------------------

static uint8_t gImport[SN2_PRESETS][SN2_SINGLE_LENGTH];

static void on_import_confirmed(bool confirmed) {
    char failed[1200];

    if (confirmed && !sn2_presets_store_all((const uint8_t(*)[SN2_SINGLE_LENGTH]) gImport)) {
        snprintf(failed, sizeof(failed), "Could not write %s", sn2_presets_path());
        show_alert("Import Presets", failed);
    }
}

static void confirm_import(int count, const char * from) {
    static char message[1400];

    if (count == 0) {
        snprintf(message, sizeof(message), "No Singles found in %s", from);
        show_alert("Import Presets", message);
        return;
    }
    snprintf(message, sizeof(message), "Replace all " PRESET_COUNT_TEXT " presets with the %d Single%s in %s, from A001? Slots it has no Single for "
             "become Init.", count, (count == 1) ? "" : "s", from);
    show_confirm("Import Presets", message, "Replace", on_import_confirmed);
}

// notes §13: the instrument's whole backup, through the same confirm as a file
static bool gReading;
static char gReadStatus[64];
static int  gReadCount;

#define READ_EXPECTED    (1424)   // what the instrument holds

static void draw_read_progress(void);

static void draw_read_progress(void) {
    if (gReadStatus[0] == '\0') {
        return;
    }
    tRectangle box  = {{(SN2_CANVAS_W - 360.0) / 2.0, (SN2_CANVAS_H - 90.0) / 2.0}, {360.0, 90.0}};
    double     frac = fmin(1.0, (double)gReadCount / READ_EXPECTED);
    double     barY = box.coord.y + box.size.h - 18.0;
    double     barW = box.size.w - 20.0;

    draw_dialog_background_overlay();
    draw_panel_chrome(mainArea, box, 24.0, "Reading from Hardware");
    set_rgb_colour((tRgb)RGB_WHITE);
    render_text(mainArea, (tRectangle){{box.coord.x + 10.0, box.coord.y + 34.0}, {0.0, 14.0}}, gReadStatus);
    set_rgb_colour((tRgb){0.25, 0.25, 0.25});
    render_rectangle(mainArea, (tRectangle){{box.coord.x + 10.0, barY}, {barW, 8.0}});
    set_rgb_colour((tRgb)RGB_GREEN_ON);
    render_rectangle(mainArea, (tRectangle){{box.coord.x + 10.0, barY}, {barW * frac, 8.0}});
}

void sn2_draw_read_hardware(void) {
    if (gReading) {
        return;
    }

    if (sn2_hw_port()[0] == '\0') {
        show_alert("Read from Hardware", "Choose the instrument's output in the MIDI menu first.");
        return;
    }
    tSn2HardwareResult r = sn2_hardware_read_start(sn2_hw_port(), (uint8_t)lround(gValue[kSn2ParamHwDevice] * 127.0));

    if (r != eSn2HardwareSent) {
        sn2_hardware_read_stop();
        show_alert("Read from Hardware", "Could not reach the instrument on that MIDI port.");
        return;
    }
    gReading   = true;
    gReadCount = 0;
    snprintf(gReadStatus, sizeof(gReadStatus), "Asking the instrument...");
}

void sn2_draw_read_tick(void) {
    if (!gReading) {
        return;
    }
    tSn2ReadState state = sn2_hardware_read_state();

    if ((state == eSn2ReadWaiting) || (state == eSn2ReadReceiving)) {
        if (state == eSn2ReadReceiving) {
            gReadCount = sn2_hardware_read_programs();
            snprintf(gReadStatus, sizeof(gReadStatus), "%d of %d programs", gReadCount, READ_EXPECTED);
        }
        return;
    }
    gReading       = false;
    gReadStatus[0] = '\0';

    if (state == eSn2ReadTimedOut) {
        sn2_hardware_read_stop();
        show_alert("Read from Hardware", "Nothing came back. Check the MIDI port, the Device number, and that MIDI runs both ways.");
        return;
    }
    char          path[1100];

    snprintf(path, sizeof(path), "%s", sn2_presets_path());
    snprintf(strrchr(path, '/') + 1, 40, "From Hardware.syx");
    bool          saved = sn2_hardware_read_save(path);

    sn2_hardware_read_stop();

    if (!saved) {
        show_alert("Read from Hardware", "Could not write what came back.");
        return;
    }
    confirm_import(sn2_presets_read_file(path, gImport), "the instrument");
}

static void on_import_file_chosen(const char * path) {
    if ((path != NULL) && (path[0] != '\0')) {
        confirm_import(sn2_presets_read_file(path, gImport), path);
    }
}

static void on_import_folder_chosen(const char * path) {
    if ((path != NULL) && (path[0] != '\0')) {
        confirm_import(sn2_presets_read_folder(path, gImport), path);
    }
}

static void on_export_chosen(const char * path) {
    char message[1200];

    if ((path != NULL) && (path[0] != '\0') && !sn2_presets_write_file(path)) {
        snprintf(message, sizeof(message), "Could not write %s", path);
        show_alert("Export Presets", message);
    }
}

void sn2_draw_import_presets(bool folder) {
    set_file_browser_start_directory_provider(browser_start);

    if (folder) {
        open_file_browser_folder(on_import_folder_chosen, "Import Presets from Folder");
    } else {
        open_file_browser_read(on_import_file_chosen);
    }
}

void sn2_draw_export_presets(void) {
    set_file_browser_start_directory_provider(browser_start);
    open_file_browser_write(on_export_chosen, "Sn2Alike Presets.syx");
}

// ---- the name field - notes §10 ---------------------------------------------------------------------------

static void draw_name_field(double x) {
    const char * shown = gNameEdit.active ? gNameEdit.buffer : host_name();

    gNameRect = (tRectangle){{
                                 x, TITLE_Y + 2.0
                             }, {
                                 get_text_width(NAME_FIELD_SIZING, PATCH_BUTTON_H, eNoCache), PATCH_BUTTON_H
                             }
    };
    draw_button(mainArea, gNameRect, shown, gNameEdit.active ? (tRgb){0.88, 0.88, 0.88}
                                                                                        : (tRgb)RGB_GREY_7);

    if (gNameEdit.active) {
        char   before[SN2_SINGLE_NAME_LENGTH + 1];
        double cursorX, top = gNameRect.coord.y + DRAW_BUTTON_MARGIN;

        snprintf(before, sizeof(before), "%.*s", (int)gNameEdit.cursor, gNameEdit.buffer);
        cursorX = gNameRect.coord.x + DRAW_BUTTON_MARGIN + get_text_width(before, PATCH_BUTTON_H, eNoCache);
        set_rgb_colour((tRgb)RGB_BLACK);
        render_line(mainArea, (tCoord){cursorX, top}, (tCoord){cursorX, top + PATCH_BUTTON_H}, 1.0);
    }
}

static void name_edit_begin(void) {
    snprintf(gNameEdit.buffer, sizeof(gNameEdit.buffer), "%s", host_name());
    gNameEdit.cursor = (uint32_t)strlen(gNameEdit.buffer);
    gNameEdit.active = true;
}

static void name_edit_commit(void) {
    gNameEdit.active = false;

    if ((gHost != NULL) && (gUser != NULL)) {
        gHost->rename(gUser, gNameEdit.buffer);
    }
}

// Return keeps it, Escape drops it; a Single's name takes either case
static bool name_edit_key(int glfwKey, unsigned int codepoint) {
    size_t len = strlen(gNameEdit.buffer);

    switch (glfwKey) {
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER: name_edit_commit();
            return true;

        case GLFW_KEY_ESCAPE: gNameEdit.active = false;
            return true;

        case GLFW_KEY_LEFT: gNameEdit.cursor  -= (gNameEdit.cursor > 0) ? 1 : 0;
            return true;

        case GLFW_KEY_RIGHT: gNameEdit.cursor += (gNameEdit.cursor < len) ? 1 : 0;
            return true;

        case GLFW_KEY_HOME: gNameEdit.cursor   = 0;
            return true;

        case GLFW_KEY_END: gNameEdit.cursor    = (uint32_t)len;
            return true;

        case GLFW_KEY_BACKSPACE:

            if (gNameEdit.cursor > 0) {
                memmove(&gNameEdit.buffer[gNameEdit.cursor - 1], &gNameEdit.buffer[gNameEdit.cursor], len - gNameEdit.cursor + 1);
                gNameEdit.cursor--;
            }
            return true;

        case GLFW_KEY_DELETE:

            if (gNameEdit.cursor < len) {
                memmove(&gNameEdit.buffer[gNameEdit.cursor], &gNameEdit.buffer[gNameEdit.cursor + 1], len - gNameEdit.cursor);
            }
            return true;

        default:
            break;
    }

    if ((glfwKey != GLFW_KEY_UNKNOWN) || (codepoint < 0x20u) || (codepoint > 0x7Eu) || (len >= SN2_SINGLE_NAME_LENGTH)) {
        return glfwKey != GLFW_KEY_UNKNOWN;   // an editing key with no meaning here is still the field's
    }
    memmove(&gNameEdit.buffer[gNameEdit.cursor + 1], &gNameEdit.buffer[gNameEdit.cursor], len - gNameEdit.cursor + 1);
    gNameEdit.buffer[gNameEdit.cursor++] = (char)codepoint;
    return true;
}

static void draw_patch_bar(void) {
    char         label[160];
    const char * base = strrchr(gPatchPath, '/');
    double       x;

    gLinkRect = (tRectangle){{
                                 PATCH_BAR_X, TITLE_Y + 2.0
                             }, {
                                 62.0, PATCH_BUTTON_H
                             }
    };
    x         = gLinkRect.coord.x + draw_button_bounds(gLinkRect).size.w + 3.0 * PATCH_BUTTON_GAP;
    gPrevRect = (tRectangle){{
                                 x, TITLE_Y + 2.0
                             }, {
                                 52.0, PATCH_BUTTON_H
                             }
    };
    gNextRect = (tRectangle){{
                                 x + 58.0, TITLE_Y + 2.0
                             }, {
                                 52.0, PATCH_BUTTON_H
                             }
    };
    gLoadRect = (tRectangle){{
                                 x + 116.0, TITLE_Y + 2.0
                             }, {
                                 56.0, PATCH_BUTTON_H
                             }
    };
    // lit while the instrument follows; amber when another instance has it or no MIDI output is chosen
    bool linkOn   = gValue[kSn2ParamHwLink] >= 0.5;
    bool noPort   = sn2_hw_port()[0] == '\0';
    bool linkWarn = linkOn && (gLinkBusy || noPort);

    draw_held_button(&gLinkRect, !linkOn ? "Link HW" : (noPort ? "No port" : (gLinkBusy ? "Link busy" : "Link HW")),
                     !linkOn ? (tRgb)RGB_GREY_7 : (linkWarn ? (tRgb){0.85, 0.6, 0.15}
                                                                                 : (tRgb)RGB_GREEN_ON));
    draw_held_button(&gPrevRect, "< Prev", (tRgb)RGB_GREY_7);
    draw_held_button(&gNextRect, "Next >", (tRgb)RGB_GREY_7);
    draw_held_button(&gLoadRect, "Load...", (tRgb)RGB_GREY_7);
    draw_name_field(gLoadRect.coord.x + draw_button_bounds(gLoadRect).size.w + 2.0 * PATCH_BUTTON_GAP);

    if (gPreset > 0) {
        sn2_preset_label(gPreset, label, sizeof(label));
        strcat(label, gPresetEdited ? "*" : "");
        set_rgb_colour(gPresetEdited ? (tRgb)RGB_WHITE : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{gNameRect.coord.x + gNameRect.size.w + 8.0, TITLE_Y + 4.0}, {0.0, 11.0}}, label);
        return;
    }

    if (base == NULL) {
        return;
    }
    // the file without its extension, and the Single's place in it when it holds more than one
    snprintf(label, sizeof(label), "%s", base + 1);

    if ((strlen(label) > 4) && (label[strlen(label) - 4] == '.')) {
        label[strlen(label) - 4] = '\0';
    }
    // notes §9: the file's name is cut to fit before the panel's margin, keeping the Single's place after it
    char place[24] = "";

    if ((gSingleCount > 1) && (strcmp(gSinglesPath, gPatchPath) == 0)) {
        snprintf(place, sizeof(place), " %d/%d", gPatchIndex + 1, gSingleCount);
    }
    x = gNameRect.coord.x + gNameRect.size.w + 8.0;

    while ((label[0] != '\0') && (x + get_text_width(label, 11.0, eNoCache) + get_text_width(place, 11.0, eNoCache) > SN2_CANVAS_W - MARGIN_X)) {
        label[strlen(label) - 1] = '\0';
    }
    strcat(label, place);
    set_rgb_colour((tRgb)RGB_GREY_5);
    render_text(mainArea, (tRectangle){{x, TITLE_Y + 4.0}, {0.0, 11.0}}, label);
}

// A button whose label is shrunk to fit it (a mod target's name is wider than its box)
#define LABEL_MIN_H    (7.0)

static void fitted_button(tRectangle r, const char * text, tRgb colour) {
    double h = r.size.h;

    while ((h > LABEL_MIN_H) && (get_text_width(text, h, eNoCache) > r.size.w)) {
        h -= 0.5;
    }
    // where draw_button puts its own label: the box grows by its margin all round, the text starts a margin in
    draw_button(mainArea, r, "", colour);
    set_rgb_colour((tRgb)RGB_BLACK);
    render_text(mainArea, (tRectangle){{r.coord.x + DRAW_BUTTON_MARGIN, r.coord.y + DRAW_BUTTON_MARGIN + ((r.size.h - h) / 2.0)}, {0.0, h}}, text);
}

static int32_t step_of(const tSn2Control * c) {
    long step = lround(gValue[c->paramId] * c->steps);

    return (int32_t)((step < 0) ? 0 : ((step > c->steps) ? c->steps : step));
}

// notes §6: sections flow left to right and wrap onto a new row when the next would not fit
static void layout(void) {
    gPlacedCount        = 0;
    gPlacedSectionCount = 0;

    if ((gPages == NULL) || (gPage >= gPageCount)) {
        return;
    }
    const tSn2Page * page = &gPages[gPage];
    double           x    = MARGIN_X;
    double           y    = PAGE_Y;

    for (uint32_t s = 0; (s < page->count) && (gPlacedSectionCount < MAX_PLACED); s++) {
        const tSn2Section * section = &page->sections[s];
        double              w       = section->count * CONTROL_W;

        if ((x > MARGIN_X) && ((x + w) > (SN2_CANVAS_W - MARGIN_X))) {
            x  = MARGIN_X;
            y += ROW_H;
        }
        gPlacedSections[gPlacedSectionCount++] = (tPlacedSection){
            section, x, y, w
        };

        for (uint32_t i = 0; (i < section->count) && (gPlacedCount < MAX_PLACED); i++) {
            double cx = x + (i * CONTROL_W);

            gPlaced[gPlacedCount++] = (tPlaced){
                &section->controls[i], section,
                {{
                     cx + ((CONTROL_W - DIAL_SIZE) / 2.0), y + HEADING_TO_DIAL
                 }, {
                     DIAL_SIZE, DIAL_SIZE
                 }
                },
                cx
            };
        }

        x                                     += w + SECTION_GAP;
    }
}

static void value_text(const tSn2Control * c, char * out, size_t len) {
    int32_t step = step_of(c);

    if (c->names != NULL) {
        snprintf(out, len, "%s", c->names[step]);
    } else {
        snprintf(out, len, "%d", (int)lround(c->plainMin + step));
    }
}

// Each title is measured at the height draw_button draws it (TAB_H), centred with TAB_PAD each side; if the tabs
// would run past the panel, the padding shrinks so they fit (TiAlike has twelve)
static void draw_tabs(void) {
    double   x     = MARGIN_X;
    double   textW[MAX_PAGES];
    double   total = 0.0;
    double   pad   = TAB_PAD;
    uint32_t p     = 0;

    for (p = 0; (p < gPageCount) && (p < MAX_PAGES); p++) {
        textW[p] = get_text_width(gPages[p].title, TAB_H, eNoCache);
        total   += textW[p] + (2.0 * DRAW_BUTTON_MARGIN) + ((p > 0) ? TAB_GAP : 0.0);
    }

    if ((p > 0) && ((total + (2.0 * pad * p)) > (SN2_CANVAS_W - (2.0 * MARGIN_X)))) {
        pad = fmax(0.0, ((SN2_CANVAS_W - (2.0 * MARGIN_X)) - total) / (2.0 * p));
    }

    for (p = 0; (p < gPageCount) && (p < MAX_PAGES); p++) {
        double w      = textW[p] + (2.0 * pad);
        tRgb   colour = (p == gPage) ? (tRgb)RGB_GREEN_ON : (tRgb)RGB_GREY_7;

        gTabs[p] = (tRectangle){{
                                    x, TAB_Y
                                }, {
                                    w, TAB_H
                                }
        };
        draw_held_button(&gTabs[p], "", colour);
        set_rgb_colour(contrasting_text_colour(colour));
        render_text(mainArea, (tRectangle){{x + DRAW_BUTTON_MARGIN + pad, TAB_Y + DRAW_BUTTON_MARGIN}, {0.0, TAB_H}}, gPages[p].title);
        x       += w + TAB_GAP;
    }
}

void sn2_draw_frame(int pixelWidth, int pixelHeight) {
    char text[64];

    if ((pixelWidth <= 0) || (pixelHeight <= 0)) {
        return;
    }
    render_backend_set_surface(pixelWidth, pixelHeight);
    set_render_width(pixelWidth);
    set_render_height(pixelHeight);
    gGlobalGuiScale = (double)pixelWidth / SN2_CANVAS_W;
    render_backend_clear((tRgb)RGB_BACKGROUND_GREY);

    if (!gFontReady) {
        render_backend_flush();
        return;
    }
    set_rgb_colour((tRgb){0.95, 0.95, 0.97});
    render_text(mainArea, (tRectangle){{MARGIN_X, TITLE_Y}, {0.0, 18.0}}, "Sn2Alike");
    fitted_button((tRectangle){{MARGIN_X + 92.0, TITLE_Y + 1.0}, {38.0, 15.0}}, "CLIP", gClip ? (tRgb){1.0, 0.15, 0.1}
                                                                                                                       : (tRgb){0.32, 0.32, 0.32});

    draw_tabs();
    layout();
    draw_patch_bar();

    for (uint32_t s = 0; s < gPlacedSectionCount; s++) {
        tPlacedSection * ps = &gPlacedSections[s];

        set_rgb_colour((tRgb)RGB_WHITE);
        render_text(mainArea, (tRectangle){{ps->x, ps->y}, {0.0, 12.0}}, ps->section->title);
        set_rgb_colour((tRgb)RGB_GREY_5);
        render_line(mainArea, (tCoord){ps->x, ps->y + 15.0}, (tCoord){ps->x + ps->w - 6.0, ps->y + 15.0}, 1.0);
    }

    for (uint32_t i = 0; i < gPlacedCount; i++) {
        tPlaced *           pc     = &gPlaced[i];
        const tSn2Control * c      = pc->control;
        tRgb                colour = {pc->section->colour[0], pc->section->colour[1], pc->section->colour[2]};

        set_rgb_colour((tRgb)RGB_GREY_7);
        render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y - 14.0}, {0.0, LABEL_H}}, c->label);
        int32_t             shown  = step_of(c);

        value_text(c, text, sizeof(text));

        // notes §8
        if ((gDisplay != NULL) && gDisplay(c->paramId, gValue, &shown, text, sizeof(text))) {
            shown = (shown < 0) ? 0 : ((shown > c->steps) ? c->steps : shown);
        }

        if (c->kind == eTiControlChoice) {
            fitted_button((tRectangle){{pc->x, pc->dial.coord.y + ((DIAL_SIZE - BUTTON_H) / 2.0)}, {CONTROL_W - 6.0, BUTTON_H}}, text, colour);
        } else {
            render_dial(mainArea, pc->dial, (uint32_t)shown, (uint32_t)c->steps + 1u, 0, colour);
            set_rgb_colour((tRgb)RGB_WHITE);
            render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y + DIAL_SIZE + 6.0}, {0.0, LABEL_H}}, text);
        }
    }

    render_menu_bar(sn2_menu_bar(), sn2_menu_bar_rect());
    draw_read_progress();
    synthlib_popups_tick();
    synthlib_popups_render();  // menus, the file browser and dialogues, front-most last
    render_backend_flush();
}

static const tPlaced * hit(double x, double y) {
    for (uint32_t i = 0; i < gPlacedCount; i++) {
        tPlaced * pc = &gPlaced[i];

        if ((x >= pc->x) && (x < pc->x + CONTROL_W) && (y >= pc->dial.coord.y - 16.0) && (y < pc->dial.coord.y + DIAL_SIZE + 20.0)) {
            return pc;
        }
    }

    return NULL;
}

static void open_value_menu(const tPlaced * pc) {
    const tSn2Control * c       = pc->control;
    int                 count   = (c->steps + 1 < MENU_MAX) ? (c->steps + 1) : MENU_MAX;
    uint32_t            columns = (uint32_t)((count + MENU_ROWS_PER_COLUMN - 1) / MENU_ROWS_PER_COLUMN);

    for (int i = 0; i < count; i++) {
        gMenuItems[i] = (tMenuItem){
            .label = c->names[i], .colour = (tRgb)MENU_BG, .action = menu_action, .param = (uint32_t)i
        };
    }

    gMenuItems[count] = (tMenuItem){
        .label = NULL, .action = NULL
    };
    gMenuFor          = c;
    gMenuChoice       = -1;
    open_context_menu(below_rect(pc->dial), gMenuItems, columns, MENU_CELL_W);
}

// notes §2
bool sn2_draw_press(double x, double y, tSn2Edit * edit, bool * grabbed) {
    *grabbed      = false;
    edit->paramId = UINT32_MAX;

    // notes §5: the popups first, front to back - a dialogue or the file browser takes every press, a menu the
    // presses on it, the menu bar its own; a press anywhere else while a menu is open only closes it
    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftDown)) {
        return true;
    }

    if (gContextMenu.active) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    // notes §10: a press on the name edits it; anywhere else while editing keeps the edit, then acts as usual
    if (within_rectangle((tCoord){x, y}, draw_button_bounds(gNameRect))) {
        if (!gNameEdit.active) {
            name_edit_begin();
        }
        return true;
    }

    if (gNameEdit.active) {
        name_edit_commit();
    }

    if (within_rectangle((tCoord){x, y}, draw_button_bounds(gLinkRect))) {
        synthlib_button_press(&gLinkRect);
        return true;
    }

    if (within_rectangle((tCoord){x, y}, draw_button_bounds(gPrevRect))) {
        synthlib_button_press(&gPrevRect);
        return true;
    }

    if (within_rectangle((tCoord){x, y}, draw_button_bounds(gNextRect))) {
        synthlib_button_press(&gNextRect);
        return true;
    }

    if (within_rectangle((tCoord){x, y}, draw_button_bounds(gLoadRect))) {
        synthlib_button_press(&gLoadRect);
        return true;
    }

    for (uint32_t p = 0; p < gPageCount; p++) {
        if (within_rectangle((tCoord){x, y}, draw_button_bounds(gTabs[p]))) {
            synthlib_button_press(&gTabs[p]);
            return true;
        }
    }

    layout();
    const tPlaced * pc = hit(x, y);

    if (pc == NULL) {
        return false;
    }

    if (pc->control->kind == eTiControlChoice) {
        open_value_menu(pc);
        gOpeningClick = true;
        gOpeningAt    = (tCoord){
            x, y
        };
        return true;
    }
    gGrabbed   = pc->control;
    gGrabValue = gValue[pc->control->paramId];
    gGrabDial  = pc->dial;
    *grabbed   = true;
    return true;
}

bool sn2_draw_drag_by(double dx, double dy, tSn2Edit * edit) {
    // up or right raises it, as the Controls menu chose
    double move = (sn2_dial_mode() == eDialModeHorizontal) ? dx : -dy;

    if (gGrabbed == NULL) {
        return false;
    }
    gGrabValue                = fmin(1.0, fmax(0.0, gGrabValue + (move / dial_drag_pixels_for_full_range((uint32_t)gGrabbed->steps))));
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// notes §4: Rotary - the pointer stays, and the dial takes its angle
bool sn2_draw_drag_to(double x, double y, tSn2Edit * edit) {
    if ((gGrabbed == NULL) || (gGrabbed->steps <= 0)) {
        return false;
    }
    uint32_t range = (uint32_t)gGrabbed->steps + 1;

    gGrabValue                = (double)angle_to_value(calculate_mouse_angle((tCoord){x, y}, gGrabDial), range) / (double)gGrabbed->steps;
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// A release ends a drag, and is where a menu's choice is taken (the press opened it, the release on an item picks)
void sn2_draw_release(double x, double y, tSn2Edit * edit) {
    edit->paramId = UINT32_MAX;
    gGrabbed      = NULL;
    gMenuChoice   = -1;

    // notes §1 (SynthLib buttonPress.h): a button acts on the release that lands on it
    tCoord       at   = {x, y};
    const void * held = synthlib_button_release();

    if (held != NULL) {
        if (synthlib_button_clicked(held, &gLinkRect, draw_button_bounds(gLinkRect), at)) {
            edit->paramId    = kSn2ParamHwLink;
            edit->normalized = (gValue[kSn2ParamHwLink] >= 0.5) ? 0.0 : 1.0;
        }

        if (synthlib_button_clicked(held, &gPrevRect, draw_button_bounds(gPrevRect), at)) {
            step_patch(-1);
        }

        if (synthlib_button_clicked(held, &gNextRect, draw_button_bounds(gNextRect), at)) {
            step_patch(1);
        }

        if (synthlib_button_clicked(held, &gLoadRect, draw_button_bounds(gLoadRect), at)) {
            sn2_draw_load_preset_browser();
        }

        for (uint32_t p = 0; p < gPageCount; p++) {
            if (synthlib_button_clicked(held, &gTabs[p], draw_button_bounds(gTabs[p]), at)) {
                gPage = p;
            }
        }

        return;
    }

    // notes §3: the release of the click that opened a menu is not a choice, even where the menu was moved up
    // to fit and so opened under the pointer
    if (gOpeningClick) {
        gOpeningClick = false;

        if (gContextMenu.active && (fabs(x - gOpeningAt.x) <= OPENING_RELEASE_SLOP) && (fabs(y - gOpeningAt.y) <= OPENING_RELEASE_SLOP)) {
            return;
        }
    }

    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftUp) && (gMenuChoice >= 0) && (gMenuFor != NULL)) {
        edit->paramId             = gMenuFor->paramId;
        edit->normalized          = (double)gMenuChoice / gMenuFor->steps;
        gValue[gMenuFor->paramId] = edit->normalized;
    }

    if (!gContextMenu.active) {
        gMenuFor = NULL;
    }
}

void sn2_draw_scroll(double dy) {
    // the browsers count rows, the wheel logical units
    if (!synthlib_popups_dispatch_scroll(-dy / 20.0) && gContextMenu.active) {
        context_menu_scroll(dy / 20.0);
    }
}

bool sn2_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat) {
    int action = isRepeat ? GLFW_REPEAT : GLFW_PRESS;

    if (gNameEdit.active && !synthlib_popups_modal_active()) {
        return name_edit_key(glfwKey, codepoint);
    }

    if ((glfwKey == GLFW_KEY_ESCAPE) && gContextMenu.active && !synthlib_popups_modal_active()) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    if (glfwKey != GLFW_KEY_UNKNOWN) {
        return synthlib_popups_dispatch_key(glfwKey, 0, action);
    }
    return (codepoint >= 0x20u) && (codepoint != 0x7Fu) && synthlib_popups_dispatch_char(codepoint);
}
