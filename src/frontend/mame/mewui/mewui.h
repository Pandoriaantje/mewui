// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

	mewui/mewui.h

	MEWUI Start Point.

***************************************************************************/
#pragma once

#ifndef MEWUI_H
#define MEWUI_H

namespace mewui
{
const game_driver* start_gui(running_machine& machine, emu_options& options, std::string& exename);
} // namespace mewui

#endif /* MEWUI_H */
