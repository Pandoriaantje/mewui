// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

	mewui/combo.h

	MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef MEWUI_COMBO_H
#define MEWUI_COMBO_H
#include <string>
#include <vector>

namespace mewui
{
	class combo
	{
	public:
		combo();

	private:
		std::vector<std::string> m_items;
	};
}
#endif /* MEWUI_COMBO_H */