/***************************************************************************

    mewui/selgame.h

    Main MEWUI menu

***************************************************************************/

#pragma once

#ifndef __MEWUI_MAIN_H__
#define __MEWUI_MAIN_H__

class ui_mewui_select_game : public ui_menu
{
public:
	ui_mewui_select_game(running_machine &machine, render_container *container, const char *gamename);
	virtual ~ui_mewui_select_game();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// force game select menu
	static void force_game_select(running_machine &machine, render_container *container);

	virtual bool menu_has_search_active() { return (m_search[0] != 0); }

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char m_search[40];
	int  m_prev_selected;

	std::vector<const game_driver *> m_availablelist;
	std::vector<const game_driver *> m_fulllist;
	std::vector<const game_driver *> m_sortedlist;
	std::vector<const game_driver *> m_availsortedlist;
	std::vector<const game_driver *> m_unavailsortedlist;
	std::vector<const game_driver *> m_unavailablelist;
	std::vector<const game_driver *> m_displaylist;
	std::vector<const game_driver *> m_searchlist;
	std::vector<const game_driver *> m_tmp;

	// internal methods
	void build_custom();
	void build_category();
	void build_full_list();
	void build_available_list();
	void build_list(std::vector<const game_driver *> &vec, const char *filter_text = NULL, int filter = 0, bool bioscheck = false);
	void build_from_cache(std::vector<const game_driver *> &vec, int filter = 0, bool bioscheck = false);

	bool no_active_search();
	void populate_search();

	// handlers
//	void inkey_cancel(const ui_menu_event *menu_event);
	void inkey_select(const ui_menu_event *menu_event);
	void inkey_select_favorite(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);

};


#endif  /* __MEWUI_MAIN_H__ */
