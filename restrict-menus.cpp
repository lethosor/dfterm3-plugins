#include "Console.h"
#include "Core.h"
#include "DataDefs.h"
#include "Export.h"
#include "PluginManager.h"

#include <set>

#include <modules/Gui.h>
#include <modules/Screen.h>
#include <VTableInterpose.h>

#include "ColorText.h"
#include "df/graphic.h"
#include "df/viewscreen_optionst.h"
#include "df/viewscreen_titlest.h"

using namespace DFHack;

DFHACK_PLUGIN("restrict-menus");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

std::string title_error = "";
struct title_hooks : df::viewscreen_titlest
{
    typedef df::viewscreen_titlest interpose_base;
    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        df::viewscreen * top = Gui::getCurViewscreen();
        VIRTUAL_CAST_VAR(screen, df::viewscreen_titlest, top);
        if (!screen)
            return;
        title_error = "";
        if (input->count(df::interface_key::SELECT) && screen->menu_line_id[screen->sel_menu_line] == 6 /* Quit */)
        {
            title_error = "Not permitted";
            return;
        }
        INTERPOSE_NEXT(feed)(input);
    }
    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        INTERPOSE_NEXT(render)();
        auto dim = Screen::getWindowSize();
        Screen::paintString(Screen::Pen(' ', COLOR_LIGHTMAGENTA, COLOR_BLACK),
                            (dim.x - title_error.length()) / 2, 0, title_error);
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(title_hooks, feed);
IMPLEMENT_VMETHOD_INTERPOSE(title_hooks, render);

std::string option_error = "";
struct option_hooks : df::viewscreen_optionst
{
    typedef df::viewscreen_optionst interpose_base;
    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        df::viewscreen * top = Gui::getCurViewscreen();
        VIRTUAL_CAST_VAR(screen, df::viewscreen_optionst, top);
        if (!screen)
            return;
        option_error = "";
        if (input->count(df::interface_key::SELECT) &&
            (screen->anon_6 == 2 || screen->anon_6 == 5))
        {
            option_error = "Not permitted";
            return;
        }
        INTERPOSE_NEXT(feed)(input);
    }
    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        INTERPOSE_NEXT(render)();
        auto dim = Screen::getWindowSize();
        Screen::paintString(Screen::Pen(' ', COLOR_LIGHTMAGENTA, COLOR_BLACK),
                           ((dim.x - option_error.length()) / 2) - 6, 9, option_error);
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(option_hooks, feed);
IMPLEMENT_VMETHOD_INTERPOSE(option_hooks, render);

bool apply_hooks (bool enable)
{
    return INTERPOSE_HOOK(title_hooks, feed).apply(enable) &&
           INTERPOSE_HOOK(title_hooks, render).apply(enable) &&
           INTERPOSE_HOOK(option_hooks, feed).apply(enable) &&
           INTERPOSE_HOOK(option_hooks, render).apply(enable);
}

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool enable)
{
    if (enable != is_enabled)
    {
        if (!apply_hooks(enable))
            return CR_FAILURE;
        is_enabled = enable;
    }
    return CR_OK;
}

