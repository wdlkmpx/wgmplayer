
#include "w_gtk_menu.h"

void menuitem_play_callback (GtkMenuItem * menuitem, void *data);
void menuitem_pause_callback(GtkMenuItem * menuitem, void *data);
void menuitem_stop_callback (GtkMenuItem * menuitem, void *data);
void menuitem_prev_callback (GtkMenuItem * menuitem, void *data);
void menuitem_next_callback (GtkMenuItem * menuitem, void *data);
void menuitem_open_callback (GtkMenuItem * menuitem, void *data);
void menuitem_showcontrols_callback (GtkCheckMenuItem * menuitem, void *data);
void menuitem_fs_callback      (GtkMenuItem * menuitem, void *data);
void menuitem_copyurl_callback (GtkMenuItem * menuitem, void *data);
void menuitem_config_dialog_cb (GtkMenuItem * menuitem, void *data);
void menuitem_quit_callback    (GtkMenuItem * menuitem, void *data);

// Popup menu
static GtkWidget *popup_menu;
static GtkMenuItem *menuitem_open;
static GtkMenuItem *menuitem_play;
static GtkMenuItem *menuitem_pause;
static GtkMenuItem *menuitem_stop;
static GtkMenuItem *menuitem_copyurl;
static GtkMenuItem *menuitem_showcontrols;
static GtkMenuItem *menuitem_fullscreen;
static GtkMenuItem *menuitem_config;
static GtkMenuItem *menuitem_quit;

// Menubar
static GtkWidget *menubar;
static GtkWidget *menu_file;
static GtkWidget *menu_file_tv;
static GtkWidget *menu_file_disc;
static GtkWidget *menu_file_recent; /* */
static GtkWidget *menu_edit;
static GtkWidget *menu_view;
static GtkWidget *menu_view_aspect;
static GtkWidget *menu_help;

static GtkMenuItem *menuitem_file;
static GtkMenuItem *menuitem_file_open;
static GtkMenuItem *menuitem_file_open_folder;
static GtkMenuItem *menuitem_file_open_location;
static GtkMenuItem *menuitem_file_disc;

static GtkMenuItem *menuitem_file_open_acd;
static GtkMenuItem *menuitem_file_open_sep1;
static GtkMenuItem *menuitem_file_open_dvdnav;
static GtkMenuItem *menuitem_file_open_dvdnav_folder;
static GtkMenuItem *menuitem_file_open_dvdnav_iso;
static GtkMenuItem *menuitem_file_open_sep2;
static GtkMenuItem *menuitem_file_open_vcd;

static GtkMenuItem *menuitem_file_tv;
static GtkMenuItem *menuitem_file_open_atv;
static GtkMenuItem *menuitem_file_open_dtv;
static GtkMenuItem *menuitem_file_recent;
static GtkMenuItem *menuitem_file_sep2;
static GtkMenuItem *menuitem_file_quit;

static GtkMenuItem *menuitem_edit;
static GtkMenuItem *menuitem_edit_switch_audio;
static GtkMenuItem *menuitem_edit_set_audiofile;
static GtkMenuItem *menuitem_edit_set_subtitle;
static GtkMenuItem *menuitem_edit_take_screenshot;
static GtkMenuItem *menuitem_edit_sep1;
static GtkMenuItem *menuitem_edit_config;
static GtkMenuItem *menuitem_help;
static GtkMenuItem *menuitem_view;
static GtkMenuItem *menuitem_view_details;
static GtkMenuItem *menuitem_view_sep0;
static GtkMenuItem *menuitem_view_fullscreen;
static GtkMenuItem *menuitem_view_sep1;
static GtkMenuItem *menuitem_view_onetoone;
static GtkMenuItem *menuitem_view_twotoone;
static GtkMenuItem *menuitem_view_onetotwo;
static GtkMenuItem *menuitem_view_onetoonepointfive;
static GtkMenuItem *menuitem_view_aspect;

static GtkMenuItem *menuitem_view_aspect_default;
static GtkMenuItem *menuitem_view_aspect_four_three;
static GtkMenuItem *menuitem_view_aspect_sixteen_nine;
static GtkMenuItem *menuitem_view_aspect_sixteen_ten;
static GtkMenuItem *menuitem_view_aspect_anamorphic;
static GtkMenuItem *menuitem_view_aspect_follow_window;
static GtkMenuItem *menuitem_view_sep2;
static GtkMenuItem *menuitem_view_subtitles;
static GtkMenuItem *menuitem_view_smaller_subtitle;
static GtkMenuItem *menuitem_view_larger_subtitle;
static GtkMenuItem *menuitem_view_decrease_subtitle_delay;
static GtkMenuItem *menuitem_view_increase_subtitle_delay;
static GtkMenuItem *menuitem_view_sep5;
static GtkMenuItem *menuitem_view_angle;
static GtkMenuItem *menuitem_view_controls;
static GtkMenuItem *menuitem_view_sep3;
static GtkMenuItem *menuitem_view_advanced;

static GtkMenuItem *menuitem_help_about;

// ==================================================================

static void create_popup_menu (GtkAccelGroup * accel_group)
{
    WGtkMenuItemParams menuitem;
    memset (&menuitem, 0, sizeof(menuitem));

    popup_menu = gtk_menu_new();

    // Play
    menuitem.parent_menu = popup_menu;
    menuitem.label       = _("_Play");
    menuitem.icon_name   = "gtk-media-play";
    menuitem.accel_group = accel_group; /* 1 */
    menuitem.activate_cb = menuitem_play_callback;
    menuitem_play = w_gtk_menu_item_new (&menuitem);
    // Pause
    menuitem.label       = _("P_ause");
    menuitem.icon_name   = "gtk-media-pause";
    menuitem.activate_cb = menuitem_pause_callback;
    menuitem_pause = w_gtk_menu_item_new (&menuitem);
    // Stop
    menuitem.label       = _("_Stop");
    menuitem.icon_name   = "gtk-media-stop";
    menuitem.activate_cb = menuitem_stop_callback;
    menuitem_stop = w_gtk_menu_item_new (&menuitem);
    // Previous
    menuitem.label       = _("Pre_vious");
    menuitem.icon_name   = "gtk-media-previous";
    menuitem.activate_cb = menuitem_prev_callback;
    menuitem_prev = w_gtk_menu_item_new (&menuitem);
    // Next
    menuitem.label       = _("_Next");
    menuitem.icon_name   = "gtk-media-next";
    menuitem.activate_cb = menuitem_next_callback;
    menuitem_next = w_gtk_menu_item_new (&menuitem);
    // ----
    w_gtk_menu_item_new (&menuitem);
    // Open
    menuitem.label       = _("_Open");
    menuitem.icon_name   = "gtk-open";
    menuitem.accel_str   = "<Control>o";
    menuitem.activate_cb = menuitem_open_callback;
    menuitem_open = w_gtk_menu_item_new (&menuitem);
    // ----
    w_gtk_menu_item_new (&menuitem);
    // Show controls
    menuitem.label       = _("S_how Controls");
    menuitem.checkbox    = TRUE;
    menuitem.check_state = TRUE;
    menuitem.activate_cb = menuitem_showcontrols_callback;
    menuitem_showcontrols = w_gtk_menu_item_new (&menuitem);
    // Full Screen
    menuitem.label       = _("_Full Screen");
    menuitem.checkbox    = TRUE;
    menuitem.activate_cb = menuitem_fs_callback;
    menuitem_fullscreen = w_gtk_menu_item_new (&menuitem);
    // Copy Location
    menuitem.label       = _("_Copy Location");
    menuitem.activate_cb = menuitem_copyurl_callback;
    menuitem_copyurl = w_gtk_menu_item_new (&menuitem);
    // Preferences
    menuitem.label       = _("_Preferences");
    menuitem.icon_name   = "gtk-preferences";
    menuitem.activate_cb = menuitem_config_dialog_cb;
    menuitem_config = w_gtk_menu_item_new (&menuitem);
    // ----
    w_gtk_menu_item_new (&menuitem);
    // Quit
    menuitem.label       = _("_Quit");
    menuitem.icon_name   = "gtk-quit";
    menuitem.accel_str   = "<Control>q";
    menuitem.activate_cb = menuitem_quit_callback;
    menuitem_quit = w_gtk_menu_item_new (&menuitem);
}

// ==================================================================

void create_menubar (GtkAccelGroup * accel_group)
{
    WGtkMenuItemParams menuitem;
    memset (&menuitem, 0, sizeof(menuitem));

    menubar = gtk_menu_bar_new();
    menu_file = gtk_menu_new();
    menu_file_disc = gtk_menu_new();
    menu_file_tv   = gtk_menu_new();
    menu_edit = gtk_menu_new();
    menu_view = gtk_menu_new();
    menu_view_aspect = gtk_menu_new();
    menu_help = gtk_menu_new();

    // Menu File -----------------------------------
    menuitem.parent_menu = menubar;
    menuitem.submenu     = menu_file;
    menuitem.label       = _("_File");
    menuitem.accel_group = accel_group; /* 1 */
    menuitem_file = w_gtk_menu_item_new (&menuitem);

    // File -> Open
    menuitem.parent_menu = menu_file;
    menuitem.label       = _("_Open");
    menuitem.icon_name   = "gtk-open";
    menuitem.accel_str   = "<Control>o";
    menuitem_file_open = w_gtk_menu_item_new (&menuitem);

    // File -> Open Folder
    menuitem.parent_menu = menu_file;
    menuitem.label       = _("Open _Folder");
    menuitem.icon_name   = "folder";
    menuitem_file_open_folder = w_gtk_menu_item_new (&menuitem);

    // File -> Open Location
    menuitem.parent_menu = menu_file;
    menuitem.label       = _("Open _Location");
    menuitem.icon_name   = "network-server";
    menuitem.accel_path  = ACCEL_PATH_OPEN_LOCATION;
    menuitem_file_open_location = w_gtk_menu_item_new (&menuitem);

    // File -> Disc
    menuitem.parent_menu = menu_file;
    menuitem.submenu     = menu_file_disc;
    menuitem.label       = _("_Disc");
    menuitem.icon_name   = "media-optical";
    menuitem_file_disc = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> Open Audio CD
    menuitem.parent_menu = menu_file_disc;
    menuitem.label       = _("Open _Audio CD");
    menuitem_file_open_acd = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> ------
    menuitem_file_open_sep1 = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> Open DVD With Menus
    menuitem.label       = _("Open DVD with _Menus");
    menuitem_file_open_dvdnav = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> Open DVD from Folder with Menus
    menuitem.label       = _("Open DVD from Folder with M_enus");
    menuitem_file_open_dvdnav_folder = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> Open DVD from ISO with Menus
    menuitem.label       = _("Open DVD from ISO with Me_nus");
    menuitem_file_open_dvdnav_iso = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> ------
    menuitem_file_open_sep2 = w_gtk_menu_item_new (&menuitem);
    // File -> Disc -> Open VCD
    menuitem.label       = _("Open _VCD");
    menuitem_file_open_vcd = w_gtk_menu_item_new (&menuitem);

    // File -> TV
    menuitem.parent_menu = menu_file;
    menuitem.submenu     = menu_file_tv;
    menuitem.label       = _("_TV");
    menuitem_file_tv = w_gtk_menu_item_new (&menuitem);
    // File -> TV -> Open Analog TV
    menuitem.parent_menu = menu_file_tv;
    menuitem.label       = _("Open _Analog TV");
    menuitem_file_open_atv = w_gtk_menu_item_new (&menuitem);
    // File -> TV -> Open Digital TV
    menuitem.parent_menu = menu_file_tv;
    menuitem.label       = _("Open _Digital TV");
    menuitem_file_open_dtv = w_gtk_menu_item_new (&menuitem);

    // File -> Open Recent
    menu_file_recent = w_gtk_recent_menu_new ("wgmplayer", NULL);
    menuitem.parent_menu = menu_file;
    menuitem.submenu     = menu_file_recent ;
    menuitem.label       = _("Open _Recent")    ;
    menuitem_file_recent = w_gtk_menu_item_new (&menuitem);

    // File -> ------
    menuitem.parent_menu = menu_file;
    menuitem_file_sep2 = w_gtk_menu_item_new (&menuitem);

    // File -> Quit
    menuitem.parent_menu = menu_file;
    menuitem.label       = _("_Quit");
    menuitem.icon_name   = "gtk-quit";
    menuitem.accel_str   = "<Control>q";
    menuitem_file_quit = w_gtk_menu_item_new (&menuitem);

    // Menu Edit -----------------------------------
    menuitem.parent_menu = menubar;
    menuitem.submenu     = menu_edit;
    menuitem.label       = _("_Edit");
    menuitem_edit = w_gtk_menu_item_new (&menuitem);

    // Edit -> Shuffle Playlist
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("_Shuffle Playlist");
    menuitem.checkbox    = TRUE;
    menuitem.check_state = random_order;
    menuitem_edit_random = w_gtk_menu_item_new (&menuitem);

    // Edit -> Loop Playlist
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("_Loop Playlist");
    menuitem.checkbox    = TRUE;
    menuitem.check_state = loop;
    menuitem_edit_loop = w_gtk_menu_item_new (&menuitem);

    // Edit -> Play Single Track from Playlist
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("P_lay Single Track from Playlist");
    menuitem.checkbox    = TRUE;
    menuitem_edit_play_single = w_gtk_menu_item_new (&menuitem);

    // Edit -> ------
    menuitem.parent_menu = menu_edit;
    menuitem_view_sep0 = w_gtk_menu_item_new (&menuitem);

    // Edit -> Switch Audio Track
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("S_witch Audio Track");
    menuitem_edit_switch_audio = w_gtk_menu_item_new (&menuitem);

    // Edit -> Set Audio
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("Set Audi_o");
    menuitem_edit_set_audiofile = w_gtk_menu_item_new (&menuitem);

    // Edit -> Select Audio Language
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("Select _Audio Language");
    menuitem_edit_select_audio_lang = w_gtk_menu_item_new (&menuitem);

    // Edit -> Set Subtitle
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("Set S_ubtitle");
    menuitem_edit_set_subtitle = w_gtk_menu_item_new (&menuitem);

    // Edit -> Select Subtitle Language
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("S_elect Subtitle Language");
    menuitem_edit_select_sub_lang = w_gtk_menu_item_new (&menuitem);

    // Edit -> ------
    menuitem.parent_menu = menu_edit;
    menuitem_view_sep0 = w_gtk_menu_item_new (&menuitem);

    // Edit -> Take Screenshot
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("_Take Screenshot");
    menuitem.tooltip     = _("Files named ’shotNNNN.png’ will be saved in the working directory");
    menuitem_edit_take_screenshot = w_gtk_menu_item_new (&menuitem);

    // Edit -> ------
    menuitem.parent_menu = menu_edit;
    menuitem_edit_sep1 = w_gtk_menu_item_new (&menuitem);

    // Edit -> Take Screenshot
    menuitem.parent_menu = menu_edit;
    menuitem.label       = _("_Preferences");
    menuitem.icon_name   = "gtk-preferences";
    menuitem.accel_path  = ACCEL_PATH_EDIT_PREFERENCES;
    menuitem_edit_config = w_gtk_menu_item_new (&menuitem);

    // Menu View -----------------------------------
    menuitem.parent_menu = menubar;
    menuitem.submenu     = menu_view;
    menuitem.label       = _("_View");
    menuitem_view = w_gtk_menu_item_new (&menuitem);

    // View -> Playlist
    menuitem.parent_menu = menu_view;
    menuitem.label       = _("_Playlist");
    menuitem.checkbox    = TRUE;
    menuitem.accel_path  = ACCEL_PATH_VIEW_PLAYLIST;
    menuitem_view_playlist = w_gtk_menu_item_new (&menuitem);

    // View -> Media Info
    menuitem.label       = _("Media _Info");
    menuitem.checkbox    = TRUE;
    menuitem.accel_path  = ACCEL_PATH_VIEW_INFO;
    menuitem_view_info = w_gtk_menu_item_new (&menuitem);

    // View -> Details
    menuitem.label       = _("D_etails");
    menuitem.checkbox    = TRUE;
    menuitem.accel_path  = ACCEL_PATH_VIEW_DETAILS;
    menuitem_view_details = w_gtk_menu_item_new (&menuitem);

    // View -> Full Screen
    menuitem.label       = _("_Full Screen");
    menuitem.checkbox    = TRUE;
    menuitem.accel_path  = ACCEL_PATH_VIEW_FULLSCREEN;
    menuitem_view_fullscreen = w_gtk_menu_item_new (&menuitem);

    // View -> ------
    menuitem_view_sep1 = w_gtk_menu_item_new (&menuitem);

    // View -> Normal (1:1)
    menuitem.label       = _("_Normal (1:1)");
    menuitem.accel_path  = ACCEL_PATH_VIEW_NORMAL;
    menuitem_view_onetoone = w_gtk_menu_item_new (&menuitem);

    // View -> Double Size (2:1)
    menuitem.label       = _("_Double Size (2:1)");
    menuitem.accel_path  = ACCEL_PATH_VIEW_DOUBLE;
    menuitem_view_twotoone = w_gtk_menu_item_new (&menuitem);

    // View -> Half Size (1:2)
    menuitem.label       = _("_Half Size (1:2)");
    menuitem_view_onetotwo = w_gtk_menu_item_new (&menuitem);

    // View -> Half Larger (1.5:1)
    menuitem.label       = _("Half _Larger (1.5:1)");
    menuitem_view_onetoonepointfive = w_gtk_menu_item_new (&menuitem);

    // View -> Aspect
    menuitem.submenu     = menu_view_aspect;
    menuitem.label       = _("_Aspect");
    menuitem.accel_path  = ACCEL_PATH_VIEW_ASPECT;
    menuitem_view_aspect = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> Default Aspect
    menuitem.parent_menu = menu_view_aspect;
    menuitem.label       = _("D_efault Aspect");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "default";
    menuitem.check_state = TRUE;
    menuitem_view_aspect_default = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> 4:3 Aspect
    menuitem.label       = _("_4:3 Aspect");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "4:3";
    menuitem_view_aspect_four_three = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> 16:9 Aspect
    menuitem.label       = _("_16:9 Aspect");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "16:9";
    menuitem_view_aspect_sixteen_nine = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> 16:10 Aspect
    menuitem.label       = _("1_6:10 Aspect");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "16:10";
    menuitem_view_aspect_sixteen_ten = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> 2.39:1 Aspect (Anamorphic)
    menuitem.label       = _("_2.39:1 Aspect (Anamorphic)");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "anamorphic";
    menuitem_view_aspect_anamorphic = w_gtk_menu_item_new (&menuitem);
    // View -> Aspect -> Follow Window
    menuitem.label       = _("_Follow Window");
    menuitem.radio_group = "aspect";
    menuitem.radio_id    = "window";
    menuitem_view_aspect_follow_window = w_gtk_menu_item_new (&menuitem);

    // View -> ------
    menuitem.parent_menu = menu_view;
    menuitem_view_sep2 = w_gtk_menu_item_new (&menuitem);

    // View -> Show Subtitles
    menuitem.label       = _("Show _Subtitles");
    menuitem.checkbox    = TRUE;
    menuitem.check_state = showsubtitles;
    menuitem.accel_path  = ACCEL_PATH_VIEW_SUBTITLES;
    menuitem_view_subtitles = w_gtk_menu_item_new (&menuitem);

    // View -> Decrease Subtitle Size
    menuitem.label       = _("Decrease Subtitle Size");
    menuitem.accel_path  = ACCEL_PATH_VIEW_DECREASE_SIZE;
    menuitem_view_smaller_subtitle = w_gtk_menu_item_new (&menuitem);

    // View -> Increase Subtitle Size
    menuitem.label       = _("Increase Subtitle Size");
    menuitem.accel_path  = ACCEL_PATH_VIEW_INCREASE_SIZE;
    menuitem_view_larger_subtitle = w_gtk_menu_item_new (&menuitem);

    // View -> Decrease Subtitle Delay
    menuitem.label       = _("Decrease Subtitle Delay");
    menuitem_view_decrease_subtitle_delay = w_gtk_menu_item_new (&menuitem);

    // View -> Increase Subtitle Delay
    menuitem.label       = _("Increase Subtitle Delay");
    menuitem_view_increase_subtitle_delay = w_gtk_menu_item_new (&menuitem);

    // View -> ------
    menuitem_view_sep5 = w_gtk_menu_item_new (&menuitem);

    // View -> Switch Angle
    menuitem.label       = _("Switch An_gle");
    menuitem.accel_path  = ACCEL_PATH_VIEW_ANGLE;
    menuitem_view_angle = w_gtk_menu_item_new (&menuitem);

    // View -> Switch Angle
    menuitem.label       = _("_Controls");
    menuitem.checkbox    = TRUE;
    menuitem.check_state = TRUE;
    menuitem.accel_path  = ACCEL_PATH_VIEW_CONTROLS;
    menuitem_view_controls = w_gtk_menu_item_new (&menuitem);

    // View -> ------
    menuitem_view_sep3 = w_gtk_menu_item_new (&menuitem);

    // View -> Video Picture Adjustments
    menuitem.label       = _("_Video Picture Adjustments");
    menuitem_view_advanced = w_gtk_menu_item_new (&menuitem);

    // Menu Help -----------------------------------
    menuitem.parent_menu = menubar;
    menuitem.submenu     = menu_help;
    menuitem.label       = _("_Help");
    menuitem_help = w_gtk_menu_item_new (&menuitem);

    // Help -> About
    menuitem.parent_menu = menu_help;
    menuitem.label       = _("_About");
    menuitem.icon_name   = "gtk-about";
    menuitem_help_about = w_gtk_menu_item_new (&menuitem);
}
