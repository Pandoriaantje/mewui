/*********************************************************************

    mewui/optsmenu.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/datfile.h"
#include "mewui/inifile.h"
#include "mewui/utils.h"
#include "mewui/selector.h"
#include "mewui/custui.h"
#include "mewui/sndmenu.h"
#include "mewui/ctrlmenu.h"
#include "mewui/dsplmenu.h"
#include "mewui/miscmenu.h"
#include "mewui/optsmenu.h"
#include "mewui/custmenu.h"
#include "ui/inputmap.h"
#include "rendfont.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_game_options::ui_menu_game_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
    m_grouped = machine.options().ui_grouped();
    m_audit = machine.options().audit_mode();
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_game_options::~ui_menu_game_options()
{
    std::string error_string;
    machine().options().set_value(OPTION_GROUPED, m_grouped, OPTION_PRIORITY_CMDLINE, error_string);
    machine().options().set_value(OPTION_AUDIT_MODE, m_audit, OPTION_PRIORITY_CMDLINE, error_string);

    ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
    save_game_options(machine());
    mewui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_game_options::handle()
{
    bool changed = false;

    // process the menu
    const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

    if (menu_event != NULL && menu_event->itemref != NULL)
        switch ((FPTR)menu_event->itemref)
        {
            case FILTER_MENU:
            {
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    (menu_event->iptkey == IPT_UI_RIGHT) ? mewui_globals::actual_filter++ : mewui_globals::actual_filter--;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_SELECT)
                {
                    int total = mewui_globals::s_filter_text;
                    std::vector<std::string> s_sel(total);
                    for (int index = 0; index < total; index++)
                        s_sel[index].assign(mewui_globals::filter_text[index]);

                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &mewui_globals::actual_filter)));
                }
                break;
            }

            case DISPLAY_MODE:
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    m_grouped = !m_grouped;
                    changed = true;
                }
                break;

            case AUDIT_MODE:
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    mewui_globals::force_reset_main = true;
                    m_audit = !m_audit;
                    changed = true;
                }
                break;

            case FILE_CATEGORY_FILTER:
            {
                if (menu_event->iptkey == IPT_UI_LEFT)
                {
                    inifile_manager::current_file--;
                    inifile_manager::current_category = 0;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_RIGHT)
                {
                    inifile_manager::current_file++;
                    inifile_manager::current_category = 0;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_SELECT)
                {
                    int total = inifile_manager::ini_index.size();
                    std::vector<std::string> s_sel(total);
                    inifile_manager::current_category = 0;
                    for (size_t index = 0; index < total; ++index)
                        s_sel[index].assign(inifile_manager::ini_index[index].name);

                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &inifile_manager::current_file, SELECTOR_INIFILE)));
                }
                break;
            }

            case CATEGORY_FILTER:
            {
                if (menu_event->iptkey == IPT_UI_LEFT)
                {
					inifile_manager::current_category--;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_RIGHT)
                {
					inifile_manager::current_category++;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_SELECT)
                {
                    int total = inifile_manager::ini_index[inifile_manager::current_file].category.size();
                    std::vector<std::string> s_sel(total);
                    for (int index = 0; index < total; ++index)
                        s_sel[index].assign(inifile_manager::ini_index[inifile_manager::current_file].category[index].name);

                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &inifile_manager::current_category, SELECTOR_CATEGORY)));
                }
                break;
            }

            case MANUFACT_CAT_FILTER:
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    (menu_event->iptkey == IPT_UI_RIGHT) ? c_mnfct::actual++ : c_mnfct::actual--;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_mnfct::ui, &c_mnfct::actual)));

                break;

            case YEAR_CAT_FILTER:
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    (menu_event->iptkey == IPT_UI_RIGHT) ? c_year::actual++ : c_year::actual--;
                    changed = true;
                }

                else if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_year::ui, &c_year::actual)));

                break;

            case MISC_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_misc_options(machine(), container)));
                break;

            case SOUND_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_sound_options(machine(), container)));
                break;

            case DISPLAY_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_display_options(machine(), container)));
                break;

            case CUSTOM_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_ui(machine(), container)));
                break;

            case CONTROLLER_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_controller_mapping(machine(), container)));
                break;

            case CGI_MENU:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_groups(machine(), container)));
                break;

            case CUSTOM_FILTER:
                if (menu_event->iptkey == IPT_UI_SELECT)
                    ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_filter(machine(), container)));
                break;

            case UME_SYSTEM:
                if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
                {
                    (menu_event->iptkey == IPT_UI_RIGHT) ? mewui_globals::ume_system++ : mewui_globals::ume_system--;
                    changed = true;
                }
                break;
        }

    if (changed)
        reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_game_options::populate()
{
    // set filter arrow
    std::string fbuff("^!File");

    // add filter item
    UINT32 arrow_flags = get_arrow_flags(0, mewui_globals::s_ume_text - 1, mewui_globals::ume_system);
    item_append("Machine", mewui_globals::ume_text[mewui_globals::ume_system], arrow_flags, (void *)UME_SYSTEM);

    // add filter item
    arrow_flags = get_arrow_flags((int)FILTER_FIRST, (int)FILTER_LAST, mewui_globals::actual_filter);
    item_append("Filter", mewui_globals::filter_text[mewui_globals::actual_filter], arrow_flags, (void *)FILTER_MENU);

    // add category subitem
    if (mewui_globals::actual_filter == FILTER_CATEGORY && !inifile_manager::ini_index.empty())
    {
        int actual_file = inifile_manager::current_file;

        if (inifile_manager::ini_index.size() == 1)
            arrow_flags = 0;
        else
            arrow_flags = get_arrow_flags(0, inifile_manager::ini_index.size() - 1, actual_file);

        convert_command_glyph(fbuff);
        item_append(fbuff.c_str(), inifile_manager::ini_index[actual_file].name.c_str(), arrow_flags, (void *)FILE_CATEGORY_FILTER);

        int actual_category = inifile_manager::current_category;

        if (inifile_manager::ini_index[actual_file].category.size() == 1)
            arrow_flags = 0;
        else
            arrow_flags = get_arrow_flags(0, inifile_manager::ini_index[actual_file].category.size() - 1, actual_category);

        fbuff.assign(" ^!Category");
        convert_command_glyph(fbuff);
        item_append(fbuff.c_str(), inifile_manager::ini_index[actual_file].category[actual_category].name.c_str(), arrow_flags, (void *)CATEGORY_FILTER);
    }

    // add manufacturer subitem
    else if (mewui_globals::actual_filter == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
    {
        if (c_mnfct::ui.size() == 1)
            arrow_flags = 0;
        else
            arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, c_mnfct::actual);

        fbuff.assign("^!Manufacturer");
        convert_command_glyph(fbuff);
        item_append(fbuff.c_str(), c_mnfct::ui[c_mnfct::actual].c_str(), arrow_flags, (void *)MANUFACT_CAT_FILTER);
    }

    // add year subitem
    else if (mewui_globals::actual_filter == FILTER_YEAR && c_year::ui.size() > 0)
    {
        if (c_year::ui.size() == 1)
            arrow_flags = 0;
        else
            arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, c_year::actual);

        fbuff.assign("^!Year");
        convert_command_glyph(fbuff);
        item_append(fbuff.c_str(), c_year::ui[c_year::actual].c_str(), arrow_flags, (void *)YEAR_CAT_FILTER);
    }

    // add custom subitem
    else if (mewui_globals::actual_filter == FILTER_CUSTOM)
    {
        fbuff.assign("^!Setup custom filter");
        convert_command_glyph(fbuff);
        item_append(fbuff.c_str(), NULL, 0, (void *)CUSTOM_FILTER);
    }

    item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

    std::string str;

    // add grouped subitem
    arrow_flags = (m_grouped) ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW;
    (m_grouped) ? str.assign("Grouped") : str.assign("Classic");
    item_append("Diplay mode", str.c_str(), arrow_flags, (void *)DISPLAY_MODE);

    // add audit subitem
    arrow_flags = (m_audit) ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW;
    (m_audit) ? str.assign("Folders Scan + Audit Missing Files") : str.assign("Folders Scan Only");
    item_append("Audit mode", str.c_str(), arrow_flags, (void *)AUDIT_MODE);

    // add options items
    item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
    item_append("Customize UI", NULL, 0, (void *)CUSTOM_MENU);
    item_append("Display Options", NULL, 0, (void *)DISPLAY_MENU);
    item_append("Sound Options", NULL, 0, (void *)SOUND_MENU);
    item_append("Miscellaneous Options", NULL, 0, (void *)MISC_MENU);
    item_append("Device Mapping", NULL, 0, (void *)CONTROLLER_MENU);
    item_append("General Inputs", NULL, 0, (void *)CGI_MENU);
    item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

    custombottom = 2.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
    customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
    float x1, x2, y1, y2, width, maxwidth = origx2 - origx1;

    // top text
    std::string topbuf("Settings");

    machine().ui().draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
                                    DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
    width += 2 * UI_BOX_LR_BORDER;
    maxwidth = MAX(maxwidth, width);

    // compute our bounds
    x1 = 0.5f - 0.5f * maxwidth;
    x2 = x1 + maxwidth;
    y1 = origy1 - top;
    y2 = origy1 - UI_BOX_TB_BORDER;

    // draw a box
    machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

    // take off the borders
    x1 += UI_BOX_LR_BORDER;
    x2 -= UI_BOX_LR_BORDER;
    y1 += UI_BOX_TB_BORDER;

    // draw the text within it
    machine().ui().draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
                                    DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}
