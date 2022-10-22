/*
 * Rewritten menu for GGM handheld.
 * Steward Fu <steward.fu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <dirent.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define GUI_DEBUG 0
#define MAX_ITEMS 7
#define ITEM_SET_AUDIO_POS 0
#define ITEM_SET_VIDEO_POS 1
#define ITEM_QUICK_SAVE_POS 2
#define ITEM_QUICK_LOAD_POS 3
#define ITEM_LOAD_ROM_POS 4
#define ITEM_RESET_EMU_POS 5
#define ITEM_EXIT_EMU_POS 6
#define MYKEY_UP 9
#define MYKEY_DOWN 10
#define MYKEY_LEFT 11
#define MYKEY_RIGHT 12
#define MYKEY_A 0
#define MYKEY_B 1
#define MYKEY_L1 4
#define MYKEY_R1 5

typedef struct _gui_settings_t
{
    int sel_main_item;
    int sel_audio_item;
    int sel_video_item;
    int sel_rom_item;
} gui_settings_t;

static gui_settings_t gui_settings = {0};
uint32_t savestate_slot = 0;
TTF_Font *font = NULL;
TTF_Font *font_small = NULL;

#if GUI_DEBUG

#define CHANGED_PC_STATUS 0
#define ROM_PATH "/home/steward/Downloads/kk/"

static uint32_t reg[255] = {0};
static SDL_Surface *screen = NULL;
static uint32_t global_enable_audio = 0;
static uint32_t current_frameskip_type = 0;
static uint32_t no_frameskip = 0;
static uint32_t screen_scale = 0;
static uint32_t auto_frameskip = 0;
static uint32_t num_skipped_frames = 0;

uint32_t menu(uint16_t *original_screen);

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);

    TTF_Init();
    font = TTF_OpenFont("font.ttf", 20);
    font_small = TTF_OpenFont("font_small.ttf", 16);
    menu(screen->pixels);
    SDL_Quit();
    return 0;
}

void flip_screen(void)
{
    SDL_Flip(screen);
}

void quit(void) {}
void reset_gba(void) {}
void video_resolution_small(void) {}
void set_gba_resolution(uint32_t scale){};
uint32_t load_gamepak(char *name) { return 0; }
#else
#include "common.h"

#define ROM_PATH "/root/roms/gba/"

extern SDL_Surface *screen;
#endif

uint32_t wait_key(void)
{
    SDL_Event event = {0};

    while (1)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_JOYBUTTONDOWN)
            {
                return event.jbutton.button;
            }
            if (event.type == SDL_JOYHATMOTION)
            {
                if (event.jhat.value == SDL_HAT_UP)
                {
                    return MYKEY_UP;
                }
                if (event.jhat.value == SDL_HAT_DOWN)
                {
                    return MYKEY_DOWN;
                }
                if (event.jhat.value == SDL_HAT_LEFT)
                {
                    return MYKEY_LEFT;
                }
                if (event.jhat.value == SDL_HAT_RIGHT)
                {
                    return MYKEY_RIGHT;
                }
            }
        }
        SDL_Delay(30);
    }
}

void draw_main_items(void)
{
    SDL_Rect rt = {0};
    int w = 0, h = 0, cc = 0;
    SDL_Surface *msg = NULL;
    SDL_Color main_item_col = {102, 204, 204};
    SDL_Color audio_item_col = {255, 255, 255};
    SDL_Color video_item_col = {255, 255, 255};
    const char *main_items_str[] = {
        "声音设定",
        "跳帧设定",
        "保存状态",
        "载入状态",
        "载入游戏",
        "重启游戏",
        "离开模拟器",
        NULL};
    const char *audio_items_str[] = {
        "开",
        "关"};
    const char *video_items_str[] = {
        "开",
        "关"};

    rt.x = 0;
    rt.y = 0;
    rt.w = 320;
    rt.h = 240;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0, 0, 0));

    rt.x = 0;
    rt.y = 36 + (23 * (gui_settings.sel_main_item + (gui_settings.sel_main_item >= ITEM_LOAD_ROM_POS ? 1 : 0)));
    rt.w = 320;
    rt.h = 23;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 180, 0, 180));
    for (cc = 0; cc < MAX_ITEMS; cc++)
    {
        TTF_SizeUTF8(font, main_items_str[cc], &w, &h);
        rt.x = 5;
        rt.y = 35 + (23 * (cc + (cc >= ITEM_LOAD_ROM_POS ? 1 : 0)));
        rt.w = 0;
        rt.h = 0;
        if (cc >= ITEM_LOAD_ROM_POS)
        {
            rt.x = (320 - w) / 2;
        }
        msg = TTF_RenderUTF8_Solid(font, main_items_str[cc], main_item_col);
        SDL_BlitSurface(msg, NULL, screen, &rt);
        SDL_FreeSurface(msg);

        switch (cc)
        {
        case ITEM_SET_AUDIO_POS:
            rt.x = 200;
            msg = TTF_RenderUTF8_Solid(font, audio_items_str[gui_settings.sel_audio_item], audio_item_col);
            SDL_BlitSurface(msg, NULL, screen, &rt);
            SDL_FreeSurface(msg);
            break;
        case ITEM_SET_VIDEO_POS:
            rt.x = 200;
            msg = TTF_RenderUTF8_Solid(font, video_items_str[gui_settings.sel_video_item], video_item_col);
            SDL_BlitSurface(msg, NULL, screen, &rt);
            SDL_FreeSurface(msg);
            break;
        }
    }
}

int32_t load_game_config_file(void)
{
}

int32_t load_config_file(void)
{
    int fd = open(GPSP_CONFIG_FILENAME, O_RDONLY);
    if (fd > 0)
    {
        read(fd, &gui_settings, sizeof(gui_settings));
        close(fd);
        return 0;
    }
    return -1;
}

int32_t save_config_file(void)
{
    int fd = open(GPSP_CONFIG_FILENAME, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd > 0)
    {
        write(fd, &gui_settings, sizeof(gui_settings));
        close(fd);
        system("sync");
        return 0;
    }
    system("sync");
    return -1;
}

int32_t save_game_config_file(void)
{
    system("sync");
    return 0;
}

int get_all_roms_count(void)
{
    int cc = 0;
    DIR *dir = NULL;
    struct dirent *ent = NULL;

    dir = opendir(ROM_PATH);
    if (dir)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type != DT_REG)
            {
                continue;
            }

            const char *ext = strrchr(ent->d_name, '.');
            if (strcasecmp(ext, ".gba") && strcasecmp(ext, ".bin") && strcasecmp(ext, ".zip"))
            {
                continue;
            }
            cc += 1;
        }
        closedir(dir);
    }
    return cc;
}

void get_savestate_filename_noshot(uint32_t slot, char *name_buffer)
{
    char savestate_ext[16] = {0};

    sprintf(savestate_ext, "%d.svs", slot);
    make_rpath(name_buffer, 512, savestate_ext);
}

void get_savestate_filename(uint32_t slot, char *name_buffer)
{
    get_savestate_filename_noshot(slot, name_buffer);
}

int32_t load_file(const char **wildcards, char *result)
{
    SDL_Rect rt = {0};
    SDL_Surface *msg = NULL;
    SDL_Color col = {255, 255, 255};

    char buf[255] = {0};
    char filename[255] = {0};
    int key = 0, all_cnt = 0, cc = 0, d0 = 0, d1 = 0, updated = 1, ret = -1;

    DIR *dir = NULL;
    struct dirent *ent = NULL;

    load_config_file();
    all_cnt = get_all_roms_count();
    save_game_config_file();

    updated = 1;
    while (1)
    {
        if (updated)
        {
            if (all_cnt > 0)
            {
                d0 = gui_settings.sel_rom_item - 5;
                d1 = gui_settings.sel_rom_item + 5;

                if (d0 < 0)
                {
                    d0 = 0;
                    d1 = 10;
                }
                if (d1 >= all_cnt)
                {
                    d0 = 0;
                    d1 = all_cnt - 1;
                    if (all_cnt > 11)
                    {
                        d0 = all_cnt - 11;
                    }
                }

                rt.x = 0;
                rt.y = 0;
                rt.w = 320;
                rt.h = 240;
                SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0, 0, 0));

                cc = 0;
                dir = opendir(ROM_PATH);
                if (dir)
                {
                    while ((ent = readdir(dir)) != NULL)
                    {
                        if (ent->d_type != DT_REG)
                        {
                            continue;
                        }
                        const char *ext = strrchr(ent->d_name, '.');
                        if (strcasecmp(ext, ".gba") && strcasecmp(ext, ".bin") && strcasecmp(ext, ".zip"))
                        {
                            continue;
                        }

                        if (cc < d0)
                        {
                            cc += 1;
                            continue;
                        }
                        if (cc > d1)
                        {
                            break;
                        }
                        sprintf(buf, "%d. %s", cc + 1, ent->d_name);

                        if (gui_settings.sel_rom_item == cc)
                        {
                            rt.x = 0;
                            rt.y = 0 + (22 * (cc - d0));
                            rt.w = 320;
                            rt.h = 20;
                            SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 180, 0, 180));
                            sprintf(result, ROM_PATH "%s", ent->d_name);
                        }

                        rt.x = 2;
                        rt.y = 2 + (22 * (cc - d0));
                        msg = TTF_RenderUTF8_Solid(font_small, buf, col);
                        SDL_BlitSurface(msg, NULL, screen, &rt);
                        SDL_FreeSurface(msg);
                        cc += 1;
                    }
                    closedir(dir);
                }
            }
            else
            {
                int w = 0, h = 0;
                const char *err = "请将游戏放到 /gba/ 目录";
                SDL_Color col = {255, 255, 255};

                TTF_SizeUTF8(font_small, err, &w, &h);
                rt.x = (320 - w) / 2;
                rt.y = (240 - h) / 2;
                msg = TTF_RenderUTF8_Solid(font_small, err, col);
                SDL_BlitSurface(msg, NULL, screen, &rt);
                SDL_FreeSurface(msg);
            }
            flip_screen();
        }

        updated = 0;
        key = wait_key();
        if (key == MYKEY_A)
        {
            ret = 0;
            break;
        }
        if (key == MYKEY_B)
        {
            ret = -1;
            break;
        }
        if (key == MYKEY_UP)
        {
            if (gui_settings.sel_rom_item > 0)
            {
                updated = 1;
                gui_settings.sel_rom_item -= 1;
            }
        }
        if (key == MYKEY_DOWN)
        {
            if (gui_settings.sel_rom_item < (all_cnt - 1))
            {
                updated = 1;
                gui_settings.sel_rom_item += 1;
            }
        }
        if (key == MYKEY_L1)
        {
            updated = 1;
            gui_settings.sel_rom_item -= 10;
            if (gui_settings.sel_rom_item < 0)
            {
                gui_settings.sel_rom_item = 0;
            }
        }
        if (key == MYKEY_R1)
        {
            updated = 1;
            gui_settings.sel_rom_item += 10;
            if (gui_settings.sel_rom_item > (all_cnt - 1))
            {
                gui_settings.sel_rom_item = all_cnt - 1;
            }
        }
    }
    save_config_file();
    return ret;
}

uint32_t menu(uint16_t *original_screen)
{
    int updated = 1;
    uint32_t key = 0;
    char buf[255] = {0};

    load_config_file();
    SDL_PauseAudio(1);
    while (1)
    {
        if (updated)
        {
            updated = 0;
            draw_main_items();
            flip_screen();
        }

        key = wait_key();
        if (key == MYKEY_UP)
        {
            if (gui_settings.sel_main_item > 0)
            {
                updated = 1;
                gui_settings.sel_main_item -= 1;
            }
        }
        if (key == MYKEY_DOWN)
        {
            if (gui_settings.sel_main_item < (MAX_ITEMS - 1))
            {
                updated = 1;
                gui_settings.sel_main_item += 1;
            }
        }
        if (key == MYKEY_LEFT)
        {
            switch (gui_settings.sel_main_item)
            {
            case ITEM_SET_AUDIO_POS:
                updated = 1;
                global_enable_audio = 1;
                gui_settings.sel_audio_item = 0;
                break;
            case ITEM_SET_VIDEO_POS:
                updated = 1;
                current_frameskip_type = auto_frameskip;
                gui_settings.sel_video_item = 0;
                break;
            }
        }
        if (key == MYKEY_RIGHT)
        {
            switch (gui_settings.sel_main_item)
            {
            case ITEM_SET_AUDIO_POS:
                updated = 1;
                global_enable_audio = 0;
                gui_settings.sel_audio_item = 1;
                break;
            case ITEM_SET_VIDEO_POS:
                updated = 1;
                current_frameskip_type = no_frameskip;
                gui_settings.sel_video_item = 1;
                break;
            }
        }

        if (key == MYKEY_B)
        {
            break;
        }
        if (key == MYKEY_A)
        {
            if (gui_settings.sel_main_item == ITEM_QUICK_SAVE_POS)
            {
                get_savestate_filename_noshot(savestate_slot, buf);
                save_state(buf, original_screen);
                break;
            }
            if (gui_settings.sel_main_item == ITEM_QUICK_LOAD_POS)
            {
                get_savestate_filename_noshot(savestate_slot, buf);
                load_state(buf);
                break;
            }
            if (gui_settings.sel_main_item == ITEM_LOAD_ROM_POS)
            {
                if (load_file(NULL, buf) == 0)
                {
                    if (load_gamepak(buf) == -1)
                    {
                        quit();
                    }
                    reset_gba();
                    reg[CHANGED_PC_STATUS] = 1;
                }
                break;
            }
            else if (gui_settings.sel_main_item == ITEM_RESET_EMU_POS)
            {
                reset_gba();
                reg[CHANGED_PC_STATUS] = 1;
                break;
            }
            else if (gui_settings.sel_main_item == ITEM_EXIT_EMU_POS)
            {
                quit();
                break;
            }
        }
    }
    save_config_file();
    set_gba_resolution(screen_scale);
    video_resolution_small();
    SDL_PauseAudio(0);
    num_skipped_frames = 100;
    return 0;
}
