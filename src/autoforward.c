/*
 * Auto mail forward Plug-in
 *  -- forward received mail to address described in autoforwardrc.
 *
 * Copyright (c) 2011-2013, HAYASHI Kentaro <kenhys@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "autoforward.h"

static SylPluginInfo info = {
  N_(PLUGIN_NAME),
  VERSION,
  "HAYASHI Kentaro",
  N_(PLUGIN_DESC)
};

static gboolean g_enable = FALSE;

static void exec_autoforward_cb(GObject *obj, FolderItem *item, const gchar *file, guint num);
static void exec_autoforward_menu_cb(void);
static void exec_autoforward_onoff_cb(void);

static GtkWidget *g_plugin_on = NULL;
static GtkWidget *g_plugin_off = NULL;
static GtkWidget *g_onoff_switch = NULL;
static GtkTooltips *g_tooltip = NULL;

static GKeyFile *g_keyfile=NULL;

static GtkWidget *g_address;
static gboolean g_startup_flg = FALSE;
static GtkWidget *g_startup = NULL;
static GtkWidget *g_unreadonly = NULL;
static gboolean g_unreadonly_flg = TRUE;
static gboolean g_forward_flg = TRUE;
static GtkListStore *g_folders = NULL;
static GtkWidget *g_add_btn;
static GtkWidget *g_delete_btn;


void plugin_load(void)
{
  GtkWidget *mainwin;
  GtkWidget *statusbar;
  GtkWidget *plugin_box;
  GdkPixbuf *on_pixbuf;
  GdkPixbuf *off_pixbuf;
  gchar *rcpath;
  gboolean startup;

  syl_init_gettext("autoforward", "lib/locale");
  
  debug_print(gettext("Auto mail forward Plug-in"));
  debug_print(dgettext("autoforward", "Auto mail forward Plug-in"));

  syl_plugin_add_menuitem("/Tools", _("Autoforward Settings [autoforward]"), exec_autoforward_menu_cb, NULL);

  g_signal_connect(syl_app_get(), "add-msg", G_CALLBACK(exec_autoforward_cb), NULL);

  mainwin = syl_plugin_main_window_get();
  statusbar = syl_plugin_main_window_get_statusbar();
  plugin_box = gtk_hbox_new(FALSE, 0);

  on_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)online);
  g_plugin_on=gtk_image_new_from_pixbuf(on_pixbuf);
    
  off_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)offline);
  g_plugin_off=gtk_image_new_from_pixbuf(off_pixbuf);

  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_on, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_off, FALSE, FALSE, 0);
    
  g_tooltip = gtk_tooltips_new();
    
  g_onoff_switch = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(g_onoff_switch), GTK_RELIEF_NONE);
  GTK_WIDGET_UNSET_FLAGS(g_onoff_switch, GTK_CAN_FOCUS);
  gtk_widget_set_size_request(g_onoff_switch, 20, 20);

  gtk_container_add(GTK_CONTAINER(g_onoff_switch), plugin_box);
  g_signal_connect(G_OBJECT(g_onoff_switch), "clicked",
                   G_CALLBACK(exec_autoforward_onoff_cb), mainwin);
  gtk_box_pack_start(GTK_BOX(statusbar), g_onoff_switch, FALSE, FALSE, 0);

  gtk_widget_show_all(g_onoff_switch);
  gtk_widget_hide(g_plugin_on);
  info.name = g_strdup(_(PLUGIN_NAME));
  info.description = g_strdup(_(PLUGIN_DESC));

  rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, "autoforwardrc", NULL);
  g_keyfile = g_key_file_new();
  if (g_key_file_load_from_file(g_keyfile, rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    startup=g_key_file_get_boolean (g_keyfile, "forward", "startup", NULL);
    debug_print("startup:%s", startup ? "true" : "false");

    if (startup){
      g_enable=TRUE;
      gtk_widget_hide(g_plugin_off);
      gtk_widget_show(g_plugin_on);
      gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                           _("Autoforward is enabled. Click the icon to disable plugin."),
                           NULL);
    }
      
    g_free(rcpath);
  }
}

void plugin_unload(void)
{
}

SylPluginInfo *plugin_info(void)
{
  return &info;
}

gint plugin_interface_version(void)
{
  /* sylpheed 3.1.0 or later */
  return 0x0107;
}

static void prefs_ok_cb(GtkWidget *widget, gpointer data)
{
  gchar *rcpath;
  const gchar *address;
  gboolean startup;
  gboolean unreadonly;
  GtkTreeModel *model;
  gint nfolder;
  gchar **folders;
  GtkTreeIter iter;
  gboolean valid;
  int nindex;
  gchar *folder;
  gsize sz;
  gchar *buf;
  
  rcpath  = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, "autoforwardrc", NULL);
g_keyfile = g_key_file_new();
  g_key_file_load_from_file(g_keyfile, rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);

  address = gtk_entry_get_text(GTK_ENTRY(g_address));
  if (address!=NULL){
    g_key_file_set_string (g_keyfile, "forward", "to", address);
  }
  startup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_startup));
  g_key_file_set_boolean (g_keyfile, "forward", "startup", startup);
  debug_print("startup:%s\n", startup ? "true" : "false");

  unreadonly = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_unreadonly));
  g_key_file_set_boolean (g_keyfile, "forward", "unreadonly", unreadonly);
  debug_print("unread only:%s\n", g_unreadonly_flg ? "true" : "false");

  g_key_file_set_boolean (g_keyfile, "forward", "all", g_forward_flg);
  debug_print("forward all:%s\n", g_forward_flg ? "true" : "false");

  /**/
  model = GTK_TREE_MODEL(g_folders);
  nfolder = gtk_tree_model_iter_n_children(model, NULL);
  if (nfolder > 0){
    folders = malloc(sizeof(gchar*)*nfolder);
    nindex = 0;
    for (valid = gtk_tree_model_get_iter_first(model, &iter); valid;
         valid = gtk_tree_model_iter_next(model, &iter)) {
      gtk_tree_model_get(model, &iter, 0, &folder, -1);
      folders[nindex] = folder;
      g_print("%d:%s\n", nindex, folder);
      nindex++;
    }
    g_key_file_set_string_list(g_keyfile, "forward", "folder",
                               (const gchar * const*)folders, nfolder);
  }else{
    g_key_file_remove_key(g_keyfile, "forward", "folder", NULL);
  }
    
  buf=g_key_file_to_data(g_keyfile, &sz, NULL);
  g_file_set_contents(rcpath, buf, sz, NULL);
    
  g_free(rcpath);

  gtk_widget_destroy(GTK_WIDGET(data));
}
static void prefs_cancel_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void add_mail_folder_cb( GtkWidget *widget,
                                gpointer   data )
{
  FolderItem *dest;
  
  GtkTreeView *view;
  GtkTreeModel *model;
  GtkTreeIter iter;

  dest = syl_plugin_folder_sel(NULL, FOLDER_SEL_COPY, NULL);
  if (!dest || !dest->path){
    return;
  }
  /**/
  view = GTK_TREE_VIEW(data);
    
  model = gtk_tree_view_get_model(view);
   
  g_folders = GTK_LIST_STORE ( model );

  gtk_list_store_append(g_folders, &iter);
  gtk_list_store_set(g_folders, &iter, 0, dest->path, -1);
}

static void forward_mail_all_cb( GtkButton *widget,
                                 gpointer   data )
{
  g_forward_flg = TRUE;

  gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_add_btn), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_delete_btn), FALSE);
  
}

static void forward_mail_folder_cb( GtkButton *widget,
                                    gpointer   data )
{
  g_forward_flg = FALSE;

  gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_add_btn), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_delete_btn), TRUE);
}

static void delete_mail_folder_cb( GtkWidget *widget,
                                   gpointer   data )
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(data) );
  GtkTreeModel *model;
  GtkTreeIter iter;
  GList *list;
  int nRemoved;
  int ipath;
  GString *fixed_path;
  GtkTreePath *path;
  
  if (gtk_tree_selection_count_selected_rows(selection) == 0){
    g_print("no selection\n");
    return;
  }
  list = gtk_tree_selection_get_selected_rows( selection, &model );
  g_folders = GTK_LIST_STORE ( model );
   
  if (!list){
    g_print("no list\n");
    return;
  }

  nRemoved = 0;
  while(list) {
    ipath = atoi(gtk_tree_path_to_string(list->data));
    ipath-=nRemoved;
    fixed_path = g_string_new("");
    g_string_printf(fixed_path, "%d", ipath);
      
    path = gtk_tree_path_new_from_string(fixed_path->str);
    g_string_free(fixed_path, TRUE);
      
    if (path) {
      if ( gtk_tree_model_get_iter ( model, &iter, path) ) { 
        gtk_list_store_remove (g_folders, &iter );
        nRemoved++;   
      }
      gtk_tree_path_free (path);
    }
    list = list->next;
  }
  g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);

  g_list_free (list);
}

static void exec_autoforward_menu_cb(void)
{
  /* show modal dialog */
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *confirm_area;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *radio1;
  GtkWidget *radio2;
  GtkWidget *frame;
  GtkWidget *vbox_cond;
  GtkTreeIter iter;
  GtkTreeViewColumn *column;
  GtkWidget *view;
  GtkWidget *sw;
  GtkWidget *bbox;
  gchar *rcpath;
  gchar *to;
  gsize sz=0;
  GError *errval;
  gchar **folders;
  int nindex;
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  /*gtk_widget_set_size_request(window, 200, 100);*/
  gtk_window_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_widget_realize(window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  confirm_area = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(confirm_area), 6);


  ok_btn = gtk_button_new_from_stock(GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(ok_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), ok_btn, FALSE, FALSE, 0);
  gtk_widget_show(ok_btn);

  cancel_btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  GTK_WIDGET_SET_FLAGS(cancel_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), cancel_btn, FALSE, FALSE, 0);
  gtk_widget_show(cancel_btn);

  gtk_widget_show(confirm_area);
	
  gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_grab_default(ok_btn);

  gtk_window_set_title(GTK_WINDOW(window), _("Autoforward Settings [autoforward]"));

  g_signal_connect(G_OBJECT(ok_btn), "clicked",
                   G_CALLBACK(prefs_ok_cb), window);
  g_signal_connect(G_OBJECT(cancel_btn), "clicked",
                   G_CALLBACK(prefs_cancel_cb), window);

  /* email settings */
  hbox = gtk_hbox_new(FALSE, 6);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new(_("Forward to(E-mail):"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  g_address = gtk_entry_new();
  gtk_widget_show(g_address);
  gtk_box_pack_start(GTK_BOX(hbox), g_address, FALSE, TRUE, 0);

  /* email settings */
  g_startup = gtk_check_button_new_with_label(_("Enable plugin on startup"));
  gtk_widget_show(g_startup);
  gtk_box_pack_start(GTK_BOX(vbox), g_startup, FALSE, FALSE, 0);

#if 0
  g_unreadonly = gtk_check_button_new_with_label(_("Forward unread mail only"));
  gtk_widget_show(g_unreadonly);
  gtk_box_pack_start(GTK_BOX(vbox), g_unreadonly, FALSE, FALSE, 0);
#endif

  /* all */
  radio1 = gtk_radio_button_new_with_label(NULL, _("Forward all mail"));
  /* folder filtered */
  radio2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1),
                                                                   _("Forward mail in folder"));

  frame = gtk_frame_new(_("Forward condition:"));
  gtk_widget_show(frame);
  gtk_container_add(GTK_CONTAINER(vbox), frame);
    
  vbox_cond = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox_cond);
  gtk_container_add(GTK_CONTAINER(frame), vbox_cond);
    
  gtk_box_pack_start(GTK_BOX(vbox_cond), radio1, FALSE, FALSE, 6);
  gtk_box_pack_start(GTK_BOX(vbox_cond), radio2, FALSE, FALSE, 6);

  /* treeview */
  g_folders = gtk_list_store_new(1, G_TYPE_STRING);
  
  column = gtk_tree_view_column_new_with_attributes (_("Forward mail in following folder:"),
                                                     gtk_cell_renderer_text_new (),
                                                     "text", 0,
                                                     NULL);
  view=gtk_tree_view_new_with_model(GTK_TREE_MODEL(g_folders));
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (sw), view);
  gtk_box_pack_start (GTK_BOX(vbox_cond), sw, TRUE, TRUE, 2);


  /* add and delete */
  bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(bbox), 6);
  g_add_btn = gtk_button_new_from_stock(GTK_STOCK_ADD);
  g_delete_btn = gtk_button_new_from_stock(GTK_STOCK_DELETE);

  gtk_box_pack_start(GTK_BOX(bbox), g_add_btn, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(bbox), g_delete_btn, FALSE, FALSE, 0);
  gtk_widget_show_all(bbox);
    
  gtk_box_pack_end(GTK_BOX(vbox_cond), bbox, FALSE, FALSE, 6);
  gtk_widget_show_all(vbox_cond);
    

  g_signal_connect (g_add_btn, "clicked",
                    G_CALLBACK (add_mail_folder_cb), view);

  g_signal_connect (g_delete_btn, "clicked",
                    G_CALLBACK (delete_mail_folder_cb), view);

  g_signal_connect(GTK_BUTTON(radio1), "clicked",
                   G_CALLBACK(forward_mail_all_cb), view);

  g_signal_connect(GTK_BUTTON(radio2), "clicked",
                   G_CALLBACK(forward_mail_folder_cb), view);

  /* load settings */

  rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, "autoforwardrc", NULL);
  g_keyfile = g_key_file_new();
  if (g_key_file_load_from_file(g_keyfile, rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    g_startup_flg = g_key_file_get_boolean (g_keyfile, "forward", "startup", NULL);
    debug_print("startup:%s\n", g_startup_flg ? "true" : "false");
    if (g_startup_flg){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_startup), TRUE);
    }

#if 0
    g_unreadonly_flg = g_key_file_get_boolean (g_keyfile, "forward", "unreadonly", NULL);
    debug_print("unreadonly:%s\n", g_unreadonly_flg ? "true" : "false");
    if (g_unreadonly_flg){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_unreadonly), TRUE);
    }
#endif

    to=g_key_file_get_string (g_keyfile, "forward", "to", NULL);
    gtk_entry_set_text(GTK_ENTRY(g_address), to);

    g_forward_flg=g_key_file_get_boolean (g_keyfile, "forward", "all", NULL);
    if (g_forward_flg){
      gtk_widget_set_sensitive(GTK_WIDGET(view), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(g_add_btn), FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(g_delete_btn), FALSE);
    }else{
      g_print("activate view and radio2\n");
      gtk_widget_set_sensitive(GTK_WIDGET(view), TRUE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio2), TRUE);
    }
    /**/
    
    folders = g_key_file_get_string_list(g_keyfile, "forward", "folder", &sz, &errval);
    if (folders!=NULL){
      nindex=0;
      for (nindex = 0; nindex < sz; nindex++){
        gtk_list_store_append(g_folders, &iter);
        gtk_list_store_set(g_folders, &iter, 0, folders[nindex], -1);
      }
    }
  }else{
    /* default settings */
    g_startup_flg = FALSE;
    g_forward_flg = TRUE;
    gtk_widget_set_sensitive(GTK_WIDGET(view), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(g_add_btn), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(g_delete_btn), FALSE);
  }
  g_free(rcpath);

  gtk_widget_show(window);
}


static void exec_autoforward_onoff_cb(void)
{

  if (g_enable != TRUE){
    syl_plugin_alertpanel_message(_("Autoforward"), _("autoforward plugin is enabled."), ALERT_NOTICE);
    g_enable=TRUE;
    gtk_widget_hide(g_plugin_off);
    gtk_widget_show(g_plugin_on);
    gtk_tooltips_set_tip
      (g_tooltip, g_onoff_switch,
       _("Autoforward is enabled. Click the icon to disable plugin."),
       NULL);
  }else{
    syl_plugin_alertpanel_message(_("Autoforward"), _("autoforward plugin is disabled."), ALERT_NOTICE);
    g_enable=FALSE;
    gtk_widget_hide(g_plugin_on);
    gtk_widget_show(g_plugin_off);
    gtk_tooltips_set_tip
      (g_tooltip, g_onoff_switch,
       _("Autoforward is disabled. Click the icon to enable plugin."),
       NULL);
  }
}

#if !GLIB_CHECK_VERSION(2, 16, 0)
int g_strcmp0(const char *str1, const char *str2)
{
  size_t str1_len = strlen(str1);
  size_t str2_len = strlen(str2);

  int value;
  if ((str1_len - str2_len) == 0) {
    value = memcmp(str1, str2, str1_len);
  } else {
    value = str1_len - str2_len;
  }
  return value;
}
#endif

void exec_autoforward_cb(GObject *obj, FolderItem *item, const gchar *file, guint num)
{
  PrefsCommon *prefs_common;
  PrefsAccount *ac;

  gchar *rcpath;
  GSList* to_list=NULL;

  gchar *to;
  GError *errval;
  gboolean forward_all;

  gsize gz=0;
  gchar **folders;
  gboolean bmatch;
  int nindex;
  
  if (g_enable!=TRUE){
    return;
  }
  if (item->stype != F_NORMAL && item->stype != F_INBOX){
    return;
  }

  prefs_common = prefs_common_get();
  if (prefs_common->online_mode != TRUE){
    return;
  }

  ac = (PrefsAccount*)account_get_default();
  g_return_if_fail(ac != NULL);

  /* check item->path for filter */
  g_print("[DEBUG] %s:%s name:%s\n", G_STRLOC, G_STRFUNC, item->name);
  g_print("[DEBUG] %s:%s path:%s\n", G_STRLOC, G_STRFUNC,  item->path);

#if 0
  MsgInfo *msginfo = folder_item_get_msginfo(item, num);
  debug_print("[DEBUG] flags:%08x UNREAD:%08x NEW:%08x MARKED:%08x ", msginfo->flags, MSG_UNREAD, MSG_NEW, MSG_MARKED);
  debug_print("[DEBUG] perm_flags:%08x \n", msginfo->flags.perm_flags);
  debug_print("[DEBUG] tmp_flags:%08x \n", msginfo->flags.tmp_flags);
  if ( g_unreadonly_flg != FALSE){
    debug_print("[DEBUG] unreadonly flag:%s\n", g_unreadonly_flg ? "true" : "false");
    if (MSG_IS_UNREAD(msginfo->flags)){
      debug_print("[DEBUG] MSG_IS_UNREAD:true");
    } else {
      return;
    }
  }
#endif    

  rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, "autoforwardrc", NULL);

  g_keyfile = g_key_file_new();
  g_key_file_load_from_file(g_keyfile, rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);
  
  to=g_key_file_get_string (g_keyfile, "forward", "to", NULL);
  debug_print("[DEBUG] to:%s", to);
  to_list = address_list_append(to_list, to);


  forward_all = g_key_file_get_boolean (g_keyfile, "forward", "all", &errval);
  if (forward_all != TRUE){
    if (errval) {
      switch (errval->code){
      case G_KEY_FILE_ERROR_INVALID_VALUE:
      case G_KEY_FILE_ERROR_KEY_NOT_FOUND:
        forward_all=TRUE;
        break;
      default:
        break;
      }
    }
  }

  bmatch = FALSE;
  if (forward_all != TRUE){
    folders = g_key_file_get_string_list(g_keyfile, "forward", "folder", &gz, NULL);
    if (gz != 0) {
      /* match or not */
      nindex = 0;
      for (nindex = 0; nindex < gz; nindex++){
        if (g_strcmp0(folders[nindex], item->path) == 0){
          bmatch = TRUE;
          debug_print("[DEBUG] %s %s => match\n", folders[nindex], item->path);
        }
      }
    } else {
      bmatch = FALSE;
    }
  }else{
    bmatch = TRUE;
  }
  g_free(rcpath);
  g_return_if_fail(to_list != NULL);

  g_print("[DEBUG] item->path:%s\n", item->path);
  g_print("[DEBUG] bmatch:%d\n", bmatch);
    
  g_return_if_fail(bmatch == TRUE);

  syl_plugin_send_message_set_forward_flags(ac->address);
  syl_plugin_send_message(file, ac, to_list);
}
