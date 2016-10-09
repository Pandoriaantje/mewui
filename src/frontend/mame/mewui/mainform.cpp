// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

	mewui/mainform.cpp

	MEWUI Main Form.

***************************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mewui/datfile.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "audit.h"
#include "luaengine.h"
#include "mewui/mainform.h"
#include "mewui/menu.h"
#include "mame.h"
#include "pluginopts.h"
#include "mewui/optionsform.h"
#include <set>

namespace mewui
{

static const std::map<std::string, int> filters_option =
{
	{ "All", -1 },
	{ "Working", -1 },
	{ "Not Working", -1 },
	{ "Parents", -1 },
	{ "Clones", -1 },
	{ "Horizontal", -1 },
	{ "Vertical", -1 },
	{ "Save State", -1 },
	{ "Mechanical", -1 },
	{ "Not Mechanical", -1 },
	{ "Manufacturer", 0 },
	{ "Year", 1 },
	{ "Source", 2 }
};

const std::pair<std::string, std::string> submenu[] = {
	{ "Software Packages", "soft" },
	{ "Status Bar", "statusbar" },
	{ "Filters", "treebox" },
	{ "Right Panel", "rpanel" }
};

static const std::unordered_map<std::string, std::string> soft_type =
{
	{ "unknown", "unkn" },
	{ "cartridge", "cart" },
	{ "floppydisk", "flop" },
	{ "harddisk", "hard" },
	{ "cylinder", "cyln" },
	{ "cassette", "cass" },
	{ "punchcard", "pcrd" },
	{ "punchtape", "ptap" },
	{ "printer", "prin" },
	{ "serial", "serl" },
	{ "parallel", "parl" },
	{ "snapshot", "dump" },
	{ "quickload", "quik" },
	{ "memcard", "memc" },
	{ "cdrom", "cdrm" },
	{ "magtape", "magt" },
	{ "romimage", "rom" },
	{ "midiin", "min" },
	{ "midiout", "mout" }
};

main_form::main_form(running_machine& machine, const game_driver** _system, emu_options& _options, std::unique_ptr<mame_ui_manager>& _mui, std::string& exename)
	: form(nana::rectangle(_mui->options().form_x(), _mui->options().form_y(), _mui->options().form_width(), _mui->options().form_heigth())
	       , appear::decorate<appear::minimize, appear::maximize, appear::sizable>())
	, m_system(_system)
	, m_options(_options)
	, m_latest_machine(_mui->options().last_used_machine())
	, m_ui(_mui)
	, m_exename(exename)
{
	// Load DATs data
	m_datfile = nullptr;
	auto& pl = mame_machine_manager::instance()->plugins();
	auto pred = [](core_options::entry &i) { return std::string(i.value()) == "data"; };
	auto it = std::find_if(pl.begin(), pl.end(), pred);
	if (it != pl.end() && std::string(it->value()) == "1")
		m_datfile = std::make_unique<datfile_manager>(machine, m_ui->options());

	// Main title
	auto maintitle = string_format("MEWUI %s", emulator_info::get_bare_build_version());
	this->caption(maintitle);
	this->bgcolor(color(214, 219, 233));
	if (m_ui->options().form_max()) this->zoom(true); // Maximize

#if defined(NANA_WINDOWS)
	// Tray
	m_notif.icon(m_exename);
	m_notif.text(maintitle);
	m_notif.events().mouse_down([this] {
		msgbox mb{ *this, "Tray Test" };
		mb.icon(mb.icon_information) << "Some msg";
		mb.show();
	});
#endif
	// Main layout
	if (std::string(m_ui->options().form_layout()).empty())
		this->div("<vert <weight=25 margin=2 <menu><weight=200 search><weight=25 s_button>><weight=5><<weight=5><weight=120 treebox>|<vert <machinebox>|<vert weight=20% soft <weight=20 swtabf><swtab>>>|<vert weight=400 rpanel <weight=20 tab><tab_frame>><weight=5>><weight=20 statusbar>>");
	else
		this->div(m_ui->options().form_layout());

	// Initialize GUI widgets
	init_context_menu();
	init_filters();
	init_machinebox();
	init_tabbar();
	this->get_place()["statusbar"] << m_statusbar;
	init_menubar();

	// Save default DATs font
	m_font = m_textpage.m_textbox.typeface();

	// Events
	handle_events();

	// Finalize layout
	m_search.tip_string("Search:").multi_lines(false);
	m_search.tooltip("Enter a term to search...");
	m_search_button.bgcolor(colors::white);
	m_search_button.tooltip("Search");
	this->get_place()["search"] << m_search;
	this->get_place()["s_button"] << m_search_button;

	this->collocate();
	m_machinebox.avoid_drawing([&] {
		if (m_machinebox.at(0).size() > 0)
		{
			m_machinebox.at(0).at(m_resel).select(true, true);
			m_machinebox.focus();
		}
	});

	update_selection();
}

void main_form::handle_events()
{
	// Main listbox events
	static auto prev_sel = 0;
	auto& events = m_machinebox.events();
	events.mouse_down([this](const arg_mouse& arg) {
		auto sel = m_machinebox.selected();
		if (!sel.empty() && sel[0].item != prev_sel)
		{
			prev_sel = sel[0].item;
			update_selection();
		}
		else
		{
			m_machinebox.at(0).at(prev_sel).select(true);
			update_selection();
		}

		// right click, show context menu
		if (arg.right_button)
			menu_popuper(m_context_menu)(arg);
	});

	events.key_release([this](const arg_keyboard& arg) {
		auto sel = m_machinebox.selected();
		if (!sel.empty())
		{
			if (arg.key == keyboard::enter || arg.key == keyboard::enter_n)
			{
				auto game = m_machinebox.at(0).at(sel[0].item).text(1);
				start_machine(game);
			}
			else if (sel[0].item != prev_sel)
			{
				prev_sel = sel[0].item;
				update_selection();
			}
		}
	});

	events.dbl_click([this] {
		auto sel = m_machinebox.selected();
		if (!sel.empty())
		{
			auto game = m_machinebox.at(0).at(sel[0].item).text(1);
			start_machine(game);
		}
	});

	// Search events
	m_search_button.events().click([this] { perform_search(); });
	m_search.events().key_char([this](const arg_keyboard& arg) {
		if (arg.key == keyboard::enter || arg.key == keyboard::enter_n)
			perform_search();
	});

	// Filters event
	m_treebox.events().selected([this](const arg_treebox& ei) {
		auto filt = ei.item.text();
		if (ei.item.level() == 1)
		{
			populate_listbox(filt);
		}
		else
		{
			auto ow = ei.item.owner().text();
			populate_listbox(ow, filt);
		}
		update_selection();
	});

	// DATs events
	m_textpage.m_combox.events().selected([this] { update_selection(); });

	// Software events
	m_swpage.m_softwarebox.events().dbl_click([this] { start_software(); });

	// Image events
	m_imgpage.m_combox.events().selected([this] { update_selection(); });

	// Main exit event
	this->events().unload([this](const arg_unload& ei) {
		msgbox mb(*this, "Quit", msgbox::yes_no);
		mb.icon(mb.icon_question) << "Are you sure you want to exit?";
		ei.cancel = mb.show() == mb.pick_no;
		if (!ei.cancel) save_options();
	});
}

void main_form::perform_search()
{
	if (m_search.caption().empty()) return;
	auto text = m_search.caption();
	strmakelower(text);
	for (auto& elem : m_machinebox.at(0))
	{
		auto comp = elem.text(0);
		strmakelower(comp);
		if (comp.find(text) != std::string::npos)
		{
			elem.select(true, true);
			update_selection();
			break;
		}
	}
}

void main_form::save_options()
{
	std::string err_str;
	auto& mui = m_ui->options();
	mui.set_value(MEWUI_FORM_HEIGHT, static_cast<int>(this->size().height), OPTION_PRIORITY_CMDLINE, err_str);
	mui.set_value(MEWUI_FORM_WIDTH, static_cast<int>(this->size().width), OPTION_PRIORITY_CMDLINE, err_str);
	mui.set_value(MEWUI_FORM_X, this->pos().x, OPTION_PRIORITY_CMDLINE, err_str);
	mui.set_value(MEWUI_FORM_Y, this->pos().y, OPTION_PRIORITY_CMDLINE, err_str);
	mui.set_value(MEWUI_FORM_MAX, this->is_zoomed(true), OPTION_PRIORITY_CMDLINE, err_str);
	auto sel = m_machinebox.selected();
	if (!sel.empty())
	{
		auto game = m_machinebox.at(0).at(sel[0].item).text(1);
		mui.set_value(OPTION_LAST_USED_MACHINE, game.c_str(), OPTION_PRIORITY_CMDLINE, err_str);
	}
	auto tsel = m_treebox.selected();
	if (tsel.level() == 0)
		mui.set_value(OPTION_LAST_USED_FILTER, m_treebox.selected().text().c_str(), OPTION_PRIORITY_CMDLINE, err_str);
	else
		mui.set_value(OPTION_LAST_USED_FILTER, m_treebox.selected().owner().text().c_str(), OPTION_PRIORITY_CMDLINE, err_str);
	auto &pl = this->get_place().div();
	mui.set_value(MEWUI_LAYOUT, pl.c_str(), OPTION_PRIORITY_CMDLINE, err_str);
	m_ui->save_ui_options();
}

void main_form::init_filters()
{
	std::set<std::string> manuf, year, src;
	for (auto x = 0; x < driver_list::total(); x++)
	{
		auto drv = &driver_list::driver(x);
		if (drv == &GAME_NAME(___empty)) continue;

		manuf.insert(drv->manufacturer);
		year.insert(drv->year);
		src.insert(core_filename_extract_base(drv->source_file));
	}

	// TEST
	for (auto & m : filters_option)
	{
		auto node = m_treebox.insert(m.first, m.first);
		switch (m.second)
		{
			case 0: // Manufacturers
				for (auto & e : manuf)
					node.append(e, e);
				break;
			
			case 1: // Years
				for (auto & e : year)
					node.append(e, e);
				break;
			
			case 2: // Sources
				for (auto & e : src)
					node.append(e, e);
				break;
			
			default:
				break;
		}
	}

	auto node = m_treebox.find(m_ui->options().last_used_filter());

	if (node.level() == 0)
		node = m_treebox.find("All");

	node.select(true);

	this->get_place()["treebox"] << m_treebox;
	m_treebox.renderer(treebox_custom_renderer(m_treebox.renderer()));
}

void main_form::init_tabbar()
{
	this->get_place()["tab"] << m_tabbar;
	this->get_place()["tab_frame"].fasten(m_textpage).fasten(m_imgpage); //Fasten the tab pages

	m_tabbar.append("Images", m_imgpage).append("Info", m_textpage);
	m_tabbar.bgcolor(color(214, 219, 233));
	m_tabbar.tab_bgcolor(0, color(240, 240, 240));
	m_tabbar.tab_bgcolor(1, color(240, 240, 240));
	m_tabbar.activated(0);
	m_tabbar.renderer(tabbar_custom_renderer(m_tabbar.renderer()));

	this->get_place()["swtabf"] << m_tabsw;
	this->get_place()["swtab"].fasten(m_swpage); //Fasten the tab pages

	m_tabsw.append("Software Packages", m_swpage);
	m_tabsw.bgcolor(color(214, 219, 233));
	m_tabsw.tab_bgcolor(0, color(240, 240, 240));
	m_tabsw.activated(0);
	m_tabsw.renderer(tabbar_custom_renderer(m_tabsw.renderer()));
}

void main_form::init_machinebox()
{
	// Add column header
	m_machinebox.append_header("Machine", 256);
	m_machinebox.append_header("Romset");
	m_machinebox.append_header("Manufacturer", 256);
	m_machinebox.append_header("Year");
	m_machinebox.append_header("Source");

	for (auto x = 0; x < driver_list::total(); ++x)
	{
		auto drv = &driver_list::driver(x);
		if (drv == &GAME_NAME(___empty)) continue;

		if (driver_list::non_bios_clone(x) == -1)
			m_sortedlist[drv->description].push_back(drv);
	}

	for (auto x = 0; x < driver_list::total(); ++x)
	{
		auto drv = &driver_list::driver(x);
		if (drv == &GAME_NAME(___empty)) continue;

		auto id = driver_list::non_bios_clone(x);
		if (id != -1)
			m_sortedlist[driver_list::driver(id).description].push_back(drv);
	}

	populate_listbox(m_treebox.selected().text());

	// Place into the layout
	this->get_place()["machinebox"] << m_machinebox;

	m_machinebox.enable_single(true, true);
	m_machinebox.scheme().header_bgcolor = static_cast<color_rgb>(0x324565);
	m_machinebox.scheme().header_fgcolor = colors::white;
	m_machinebox.scheme().item_highlighted = colors::white;
}

void main_form::start_software()
{
	auto sel = m_machinebox.selected();
	if (sel.empty()) return;

	std::string error_string;
	auto game = m_machinebox.at(0).at(sel[0].item).text(1);
	driver_enumerator drivlist(m_options, game.c_str());
	media_auditor auditor(drivlist);
	drivlist.next();

	auto selection = m_swpage.m_softwarebox.selected();
	if (selection.empty()) return;
	auto name = m_swpage.m_softwarebox.at(0).at(selection[0].item).text(1);
	software_list_device_iterator iter(drivlist.config().root_device());
	for (auto& swlistdev : iter)
	{
		auto swinfo = swlistdev.find(name.c_str());
		if (swinfo != nullptr)
		{
			// loop through all parts
			for (const auto& swpart : swinfo->parts())
			{
				// only load compatible software this way
				if (swlistdev.is_compatible(swpart) == SOFTWARE_IS_COMPATIBLE)
				{
					auto image = software_list_device::find_mountable_image(drivlist.config(), swpart);
					if (image != nullptr)
					{
						// audit software
						auto summary = auditor.audit_software(swlistdev.list_name(), swinfo, AUDIT_VALIDATE_FAST);
						if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
						{
							auto string_list = string_format("%s:%s:%s:%s", swlistdev.list_name(), name, swpart.name(), image->instance_name());
							m_options.set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
							start_machine(game);
						}
						else
						{
							// otherwise, display an error
							msgbox mb(*this, "Error");
							mb.icon(mb.icon_error) << "The selected software is missing one or more required files.";
							mb.show();
						}
					}
				}
			}
		}
	}
}

void main_form::start_machine(std::string& game)
{
	// audit the game first to see if we're going to work
	driver_enumerator enumerator(m_options, game.c_str());
	enumerator.next();
	media_auditor auditor(enumerator);
	auto summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// if everything looks good, schedule the new driver
	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		*m_system = &enumerator.driver();
		save_options();
		API::exit();
	}
	else
	{
		// otherwise, display an error
		msgbox mb(*this, "Error");
		mb.icon(mb.icon_error) << "The selected machine is missing one or more required files.";
		mb.show();
	}
}

void main_form::populate_listbox(const std::string& filter, const std::string& subfilter)
{
	m_machinebox.auto_draw(false);
	if (!m_machinebox.empty())
		m_machinebox.clear(0);

	auto cat = m_machinebox.at(0);
	auto index = 0;
	for (auto parent : m_sortedlist)
	{
		for (auto drv : parent.second)
		{
			if (filter != "All")
			{
				if (subfilter.empty())
				{
					if (filter == "Working" && (drv->flags & MACHINE_NOT_WORKING) != 0) continue;
					if (filter == "Not Working" && (drv->flags & MACHINE_NOT_WORKING) == 0) continue;
					if (filter == "Parents" && parent.first != drv->description) continue;
					if (filter == "Clones" && parent.first == drv->description) continue;
					if (filter == "Horizontal" && (drv->flags & ORIENTATION_SWAP_XY) != 0) continue;
					if (filter == "Vertical" && (drv->flags & ORIENTATION_SWAP_XY) == 0) continue;
					if (filter == "Save State" && (drv->flags & MACHINE_SUPPORTS_SAVE) == 0) continue;
					if (filter == "Mechanical" && (drv->flags & MACHINE_MECHANICAL) == 0) continue;
					if (filter == "Not Mechanical" && (drv->flags & MACHINE_MECHANICAL) != 0) continue;
				}
				else
				{
					switch (filters_option.at(filter))
					{
						case 0: if (subfilter != drv->manufacturer) continue; break;
						case 1: if (subfilter != drv->year) continue; break;
						case 2: if (subfilter != core_filename_extract_base(drv->source_file)) continue; break;
						default: break;
					}
				}
			}

			cat.append({ std::string(drv->description), std::string(drv->name), std::string(drv->manufacturer), std::string(drv->year), core_filename_extract_base(drv->source_file) });
			if (m_latest_machine == drv->name) m_resel = index;

			bool cloneof = strcmp(drv->parent, "0");
			if (cloneof)
			{
				auto cx = driver_list::find(drv->parent);
				if (cx != -1 && (driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) == 0)
					cat.at(index).fgcolor(colors::gray);
			}
			index++;
		}
	}
	m_machinebox.auto_draw(true);
	m_latest_machine.clear();
}

void main_form::update_selection()
{
	std::string work;
	auto sel = m_machinebox.selected();
	if (!sel.empty())
	{
		auto game = m_machinebox.at(0).at(sel[0].item).text(1);
		auto drv = &driver_list::driver(driver_list::find(game.c_str()));

		// Load image
		load_image(drv);

		// Load data from DATs
		load_data(drv);

		// Update menu item
		m_menubar.at(0).change_text(0, string_format("&Play %s", drv->description));
		m_context_menu.change_text(0, string_format("Play %s", drv->description));
		//m_context_menu.change_text(16, string_format("Properties for %s", core_filename_extract_base(drv->source_file)));

		// Update software
		m_swpage.m_softwarebox.auto_draw(false);
		auto cat = m_swpage.m_softwarebox.at(0);

		if (!m_swpage.m_softwarebox.empty())
			m_swpage.m_softwarebox.clear(0);

		std::unordered_set<std::string> list_map;
		driver_enumerator drivlist(m_options, game.c_str());
		while (drivlist.next())
		{
			for (auto& swlistdev : software_list_device_iterator(drivlist.config().root_device()))
				if (list_map.insert(swlistdev.list_name()).second && !swlistdev.get_info().empty())
					for (const auto& swinfo : swlistdev.get_info())
					{
						auto& part = swinfo.parts().front();
						auto pred = [&part](const std::pair<std::string, std::string>& i) {
							return part.name().find(i.second) != std::string::npos;
						};
						auto it = std::find_if(soft_type.begin(), soft_type.end(), pred);
						std::string part_name = (it != soft_type.end()) ? it->first : "unknown";
						cat.append({ swinfo.longname(), swinfo.shortname(), swinfo.publisher(), swinfo.year(), part_name });
						if (!swinfo.parentname().empty())
							cat.back().fgcolor(colors::gray);
					}
		}
		m_swpage.m_softwarebox.auto_draw(true);

		work = (drv->flags & MACHINE_NOT_WORKING) ? "Not Working" : "Working";
	}

	// Update status bar
	m_statusbar.update(m_machinebox.at(0).size(), work);
}

void main_form::load_image(const game_driver* drv)
{
	auto opt = m_imgpage.m_combox.option();
	std::string addpath, searchstr;
	std::array<std::string, 3> exte = { ".png", ".jpg", ".jpeg" };

	if (opt == 0)
	{
		emu_options moptions;
		searchstr = m_options.value(arts_info[opt].second.c_str());
		addpath = moptions.value(arts_info[opt].second.c_str());
	}
	else
	{
		ui_options moptions;
		searchstr = m_ui->options().value(arts_info[opt].second.c_str());
		addpath = moptions.value(arts_info[opt].second.c_str());
	}

	path_iterator path(searchstr);
	path_iterator path_iter(addpath);
	std::string c_path, curpath;

	// iterate over path and add path for zipped formats
	while (path.next(curpath))
	{
		path_iter.reset();
		while (path_iter.next(c_path))
			searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append(c_path);
	}

	emu_file snapfile(std::move(searchstr), OPEN_FLAG_READ);
	snapfile.set_restrict_to_mediapath(true);
	std::string fname;
	using osd_err = osd_file::error;
	auto filerr = osd_err::FAILURE;

	for (auto& e : exte)
	{
		fname.assign(drv->name).append(e);
		filerr = snapfile.open(fname);
		if (filerr == osd_err::NONE) break;
	}

	if (filerr != osd_err::NONE && driver_list::non_bios_clone(*drv) != -1)
		for (auto& e : exte)
		{
			fname.assign(drv->parent).append(e);
			filerr = snapfile.open(fname);
			if (filerr == osd_err::NONE) break;
		}

	if (filerr == osd_err::NONE)
	{
		auto length = static_cast<UINT32>(snapfile.size());
		auto data = global_alloc_array(UINT32, length);
		if (snapfile.read(data, length) > 0)
		{
			m_imgpage.m_picture.show();
			paint::image img;
			if (img.open(data, length))
			{
				m_imgpage.m_picture.load(img);
				m_imgpage.m_picture.stretchable(true);
				m_imgpage.m_picture.align(align::center, align_v::center);
			}
		}
		global_free_array(data);
	}
	else
		m_imgpage.m_picture.hide();
}

void main_form::load_data(const game_driver* drv)
{
	if (m_datfile == nullptr) return;

	std::string buffer;
	if (m_textpage.m_combox.caption() != "Commands")
		m_datfile->load_data_info(drv, buffer, m_textpage.m_combox.caption());
	else
	{
		std::vector<std::string> m_item;
		m_datfile->command_sub_menu(drv, m_item);
		for (auto& elem : m_item)
		{
			std::string t_buffer;
			buffer.append(elem).append("\n");
			m_datfile->load_command_info(t_buffer, elem);
			if (!t_buffer.empty())
				buffer.append(t_buffer).append("\n");
		}
	}

	if (!buffer.empty())
	{
		m_textpage.m_textbox.typeface(m_font);
		m_textpage.m_textbox.reset(buffer, false);
		m_textpage.m_textbox.line_wrapped(true);
	}
	else
	{
		auto tmp = paint::font(m_font.name(), m_font.size(), true, true);
		m_textpage.m_textbox.typeface(tmp);
		m_textpage.m_textbox.reset("No info available", false);
	}
}

void main_form::init_menubar()
{
	m_menubar.bgcolor(color(214, 219, 233));

	// Initialize menu
	init_file_menu();
	init_view_menu();
	init_options_menu();
	init_help_menu();
}

void main_form::init_file_menu()
{
	auto& menu = m_menubar.push_back("&File");
	menu.append("&Play XXX", [this](menu::item_proxy& ip) {
		auto sel = m_machinebox.selected();
		if (!sel.empty())
		{
			auto game = m_machinebox.at(0).at(sel[0].item).text(1);
			start_machine(game);
		}
	});

/*	menu.append_splitter();
	menu.append("Play and &Record Input", [this](menu::item_proxy& ip) {});
	menu.append("P&layback Input", [this](menu::item_proxy& ip) {});
	menu.append_splitter();
	menu.append("Play and Record &Wave Output", [this](menu::item_proxy& ip) {});
	menu.append("Play and Record &MNG Output", [this](menu::item_proxy& ip) {});
	menu.append("Play and Record &uncompressed AVI Output", [this](menu::item_proxy& ip) {});
	menu.append_splitter();
	menu.append("Loa&d Savestate", [this](menu::item_proxy& ip) {});
	menu.append_splitter();
	menu.append("Pr&opertis", [this](menu::item_proxy& ip) {});
	menu.append_splitter();
	menu.append("Audi&t existing Sets", [this](menu::item_proxy& ip) {});
	menu.append("&Audit All Sets", [this](menu::item_proxy& ip) {}); 
*/
	menu.append_splitter();
	menu.append("E&xit", [this](menu::item_proxy& ip) { this->close(); });
	menu.renderer(custom_renderer(menu.renderer()));
}

void main_form::init_view_menu()
{
	auto& menu = m_menubar.push_back("View");

	for (auto & e: submenu)
	{
		auto item = menu.append(e.first, [this, &e](menu::item_proxy& ip) {
			this->get_place().field_display(e.second.c_str(), ip.checked());
			this->collocate();
		});
		item.check_style(menu::checks::highlight);
		item.checked(this->get_place().field_display(e.second.c_str()));
	}

	menu.renderer(custom_renderer(menu.renderer()));
}

void main_form::init_options_menu()
{
	auto& menu = m_menubar.push_back("Options");
//	menu.append("Interface Options", [this](menu::item_proxy& ip) {});
//	menu.append("Global Machine Options", [this](menu::item_proxy& ip) {});
	menu.append("Directories", [this](menu::item_proxy& ip) {
		dir_form dform{ *this, m_options, m_ui };
		dform.show();
	});
//	menu.append_splitter();
//	menu.append("Machine List Font", [this](menu::item_proxy& ip) {});
//	menu.append("Machine List Clone Color", [this](menu::item_proxy& ip) {});
//	menu.append_splitter();
//	menu.append("Background Image", [this](menu::item_proxy& ip) {});
//	menu.append_splitter();
//	menu.append("Reset to Default", [this](menu::item_proxy& ip) {});
	menu.renderer(custom_renderer(menu.renderer()));
}

void main_form::init_help_menu()
{
	auto& menu = m_menubar.push_back("Help");
	menu.append("About", [this](menu::item_proxy& ip) {
		auto message = string_format("MEWUI %s by Dankan1890", emulator_info::get_bare_build_version());
		msgbox mb(*this, "About MEWUI");
		mb.icon(msgbox::icon_information) << message;
		mb.show();
	});
	menu.renderer(custom_renderer(menu.renderer()));
}

void main_form::init_context_menu()
{
	m_context_menu.append("Play XXX", [this](menu::item_proxy& ip) {
		auto sel = m_machinebox.selected();
		if (!sel.empty())
		{
			auto game = m_machinebox.at(0).at(sel[0].item).text(1);
			start_machine(game);
		}
	});
/*	m_context_menu.append_splitter();
	m_context_menu.append("Play and Record Input", [this](menu::item_proxy& ip) {});
	m_context_menu.append_splitter();
	m_context_menu.append("Add to Custom Folder", [this](menu::item_proxy& ip) {});
	auto item = m_context_menu.append("Remove from this Folder", [this](menu::item_proxy& ip) {});
	item.enabled(false);
	m_context_menu.append("Custom Filter", [this](menu::item_proxy& ip) {});
	m_context_menu.append_splitter();
	m_context_menu.append("Select Random Machine", [this](menu::item_proxy& ip) {});
	m_context_menu.append_splitter();
	m_context_menu.append("Reset Play Count", [this](menu::item_proxy& ip) {});
	m_context_menu.append("Reset Play Time", [this](menu::item_proxy& ip) {});
	m_context_menu.append_splitter();
	m_context_menu.append("Audit", [this](menu::item_proxy& ip) {});
	m_context_menu.append_splitter();
	m_context_menu.append("Properties", [this](menu::item_proxy& ip) {});
	m_context_menu.append("Properties for XXX.cpp", [this](menu::item_proxy& ip) {});
	m_context_menu.append("Vector Properties", [this](menu::item_proxy& ip) {}); 
*/
}

tab_page_picturebox::tab_page_picturebox(window wd)
	: panel<true>(wd)
{
	this->bgcolor(color(240, 240, 240));
	m_picture.bgcolor(color(240, 240, 240));
	for (auto& elem : arts_info)
		m_combox.push_back(elem.first);

	m_combox.option(0);
	m_combox.editable(false);

	// Layout
	m_place.div("vert margin=5<weight=20 combobox><weight=5><pct>");
	m_place["combobox"] << m_combox;
	m_place["pct"] << m_picture;
	dw.draw([this](paint::graphics& graph) {
		nana::rectangle r(graph.size());
		auto right = r.right() - 1;
		auto bottom = r.bottom() - 1;
		graph.line_begin(right, r.y);
		graph.line_to({ right, bottom }, static_cast<color_rgb>(0x9cb6c5));
		graph.line_to({ r.x, bottom }, static_cast<color_rgb>(0x9cb6c5));
		graph.line_to({ r.x, r.y - 1 }, static_cast<color_rgb>(0x9cb6c5));
	});
}

tab_page_softwarebox::tab_page_softwarebox(window wd)
	: panel<true>(wd)
{
	this->bgcolor(color(240, 240, 240));
	m_softwarebox.append_header("Name", 180);
	m_softwarebox.append_header("Set", 70);
	m_softwarebox.append_header("Publisher", 180);
	m_softwarebox.append_header("Year", 70);
	m_softwarebox.append_header("Type", 70);

	// Layout
	m_place.div("<swbox>");
	m_place["swbox"] << m_softwarebox;
	m_softwarebox.scheme().header_bgcolor = static_cast<color_rgb>(0x324565);
	m_softwarebox.scheme().header_fgcolor = colors::white;
}

tab_page_textbox::tab_page_textbox(window wd)
	: panel<true>(wd)
{
	this->bgcolor(color(240, 240, 240));
	m_textbox.editable(false);
	m_textbox.multi_lines(true);
	m_textbox.focus_behavior(widgets::skeletons::text_focus_behavior::none);
	m_combox.push_back("History");
	m_combox.push_back("MameInfo");
	m_combox.push_back("SysInfo");
	m_combox.push_back("MessInfo");
	m_combox.push_back("Commands");
	m_combox.push_back("Machine Init");
	m_combox.push_back("MameScore");
	m_combox.option(0);
	m_combox.editable(false);

	// Layout
	m_place.div("vert margin=5 <weight=20 combobox><weight=5><txt>");
	m_place["combobox"] << m_combox;
	m_place["txt"] << m_textbox;
	dw.draw([this](paint::graphics& graph) {
		nana::rectangle r(graph.size());
		auto right = r.right() - 1;
		auto bottom = r.bottom() - 1;
		graph.line_begin(right, r.y);
		graph.line_to({ right, bottom }, static_cast<color_rgb>(0x9cb6c5));
		graph.line_to({ r.x, bottom }, static_cast<color_rgb>(0x9cb6c5));
		graph.line_to({ r.x, r.y - 1 }, static_cast<color_rgb>(0x9cb6c5));
	});
}

statusbar::statusbar(window wd)
	: panel<true>(wd)
{
	auto bgcolor = color(0, 122, 204);
	this->bgcolor(bgcolor);
	m_machines.bgcolor(bgcolor);
	m_machines.fgcolor(colors::white);
	m_working.bgcolor(bgcolor);
	m_working.fgcolor(colors::white);
	m_working.text_align(align::right);

	// Layout
	m_place.div("<margin=[0,5] <machines> <work>>");
	m_place["machines"] << m_machines;
	m_place["work"] << m_working;
}

void statusbar::update(int m, std::string w)
{
	auto ma = string_format("Machines %d / %d", m, driver_list::total() - 1);
	auto wo = string_format("Overall: %s", w);
	m_machines.caption(ma);
	m_working.caption(wo);
}

} // namespace mewui
