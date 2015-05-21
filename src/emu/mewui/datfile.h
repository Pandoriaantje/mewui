/***************************************************************************

    mewui/datfile.h

    MEWUI data file.

***************************************************************************/

#pragma once

#ifndef __DATFILE_H__
#define __DATFILE_H__

//-------------------------------------------------
//  CONSTANTS AND STRUCTURES
//-------------------------------------------------
#define MAX_TOKEN_LENGTH        65535
#define DATAFILE_TAG '$'
#define MAX_MENUIDX_ENTRIES 64

struct tDatafileIndex
{
    long offset;
    const game_driver *driver;
};

struct sDataDrvIndex
{
    long offset;
    std::string name;
};

struct tMenuIndex
{
    UINT64 offset;
    std::string menuitem;
};

struct SoftwareItem
{
    std::string softname;
    long offset;
};

struct SoftwareListIndex
{
    std::string listname;
    std::vector<SoftwareItem> item_list;
};

//-------------------------------------------------
//  Datafile Manager
//-------------------------------------------------
class datfile_manager
{
public:

    // construction/destruction
    datfile_manager(running_machine &machine);

    // getters
    running_machine &machine() const { return m_machine; }

    bool history_file() const { return m_history_file; }
    bool mame_file() const { return m_mame_file; }
    bool mess_file() const { return m_mess_file; }
    bool command_file() const { return m_command_file; }
    bool sysinfo_file() const { return m_sysinfo_file; }
    bool story_file() const { return m_story_file; }

    tDatafileIndex *historyindex() { return hist_idx; }
    tDatafileIndex *mameindex() { return mame_idx; }
    tDatafileIndex *messindex() { return mess_idx; }
    tDatafileIndex *cmdindex() { return cmnd_idx; }
    tDatafileIndex *sysindex() { return sysi_idx; }
    tDatafileIndex *storyindex() { return story_idx; }

    // actions
    void load_data_info(const game_driver *drv, std::string &buffer, int hm_type);
    void load_command_info(std::string &buffer, const int menu_sel);
    void load_software_info(const char *soft_list, std::string &buffer, const char *soft_name);
    void command_sub_menu(const game_driver *drv, std::vector<std::string> &menu_item);

    std::string rev_history() const { return history_revision; }
    std::string rev_mameinfo() const { return mame_revision; }
    std::string rev_messinfo() const { return mess_revision; }
    std::string rev_sysinfo() const { return sysinfo_revision; }
    std::string rev_storyinfo() const { return story_revision; }

private:

    // global index
    static tDatafileIndex *hist_idx, *mame_idx, *mess_idx, *cmnd_idx, *sysi_idx, *story_idx;
    static sDataDrvIndex *drv_idx, *drvmess_idx;
    static tMenuIndex *menu_idx;

    static std::vector<SoftwareListIndex> sListIndex;

    // internal helpers
    void init_history();
    void init_mameinfo();
    void init_messinfo();
    void init_command();
    void init_sysinfo();
    void init_storyinfo();

    bool ParseOpen(const char *filename);

    int index_mame_mess_info(tDatafileIndex **_index, sDataDrvIndex **_index_drv, int &drvcount);
    int index_datafile(tDatafileIndex **_index, int &swcount);
    int index_menuidx(const game_driver *drv, tDatafileIndex *d_idx, tMenuIndex **_index);

    int load_command_text(std::string &buffer, tMenuIndex *m_idx, const int menu_sel);
    void load_data_text(const game_driver *drv, std::string &buffer, tDatafileIndex *idx, const char *tag);
    void load_driver_text(const game_driver *drv, std::string &buffer, sDataDrvIndex *idx, const char *tag);

    bool find_command(const game_driver *drv);
    void free_menuidx(tMenuIndex **_index);
    int find_or_allocate(std::string name);

    // internal state
    running_machine     &m_machine;             // reference to our machine

	std::string			fullpath;
    static bool         m_history_file, m_mame_file, m_mess_file, m_command_file, m_sysinfo_file, m_story_file;
    static std::string  history_revision, mame_revision, mess_revision, sysinfo_revision, story_revision;
};


#endif  /* __DATFILE_H__ */
