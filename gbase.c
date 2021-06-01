/*****************************************************************************
 * gbase - Base Converter                                                    *
 *                                                                           *
 * Copyright (C) 1999 Damian Kramer (psiren@hibernaculum.demon.co.uk)        *
 * Copyright (C) 1999 Roger Dunce (kro@penguinpowered.com)                   *
 *                                                                           *
 * You may distribute this program under the terms of the Artistic License.  *
 *                                                                           *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * Artistic License for more details.                                        *
 *                                                                           *
 *****************************************************************************/


#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>

#define BIN 2
#define OCT 8
#define DEC 10
#define HEX 16

void create_window();
void quit();
void usage();
void update_entry(int entry_no);
void generate_string(int base);
void clear_entries();
gboolean bad_string(const char *string, int length, int base, int pos);
void process_args(int argc, char **argv);

gint dump_callback(GtkWidget *widget, gpointer *data);
gint clear_callback(GtkWidget *widget, gpointer *data);
gint quit_callback(GtkWidget *widget, gpointer *data);
void radio_button_callback(GtkWidget *widget, gpointer data);
void entry_changed_callback(GtkEntry *entry, gpointer *data);
void text_entered_callback(GtkEditable *editable, gchar *text, gint length,  gint *pos,
			   gpointer data);

long int accumulator = 0;
gboolean ignore_changed_signal = FALSE;
gboolean val_is_signed = FALSE;
gboolean overflow = FALSE;

char buffer[33];

GtkWidget *entry[4];
GtkWidget *overflow_label;

char *labels[] = {"Dec", "Hex", "Oct", "Bin"};
int bases[] = {DEC, HEX, OCT, BIN};

char *version = "0.5";

int main(int argc, char *argv[])
{
  int i;

  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sep;
  GtkWidget *button;
  GSList *group;
  GtkWidget *dump_button;
  GtkWidget *clear_button;
  GtkWidget *quit_button;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *frame;

  gtk_init(&argc, &argv);

  process_args(argc, argv);
  
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Base Converter");
  gtk_window_set_policy(GTK_WINDOW(window), TRUE, FALSE, TRUE);
  gtk_container_border_width(GTK_CONTAINER(window), 5);

  gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(quit), NULL);
  gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(quit), NULL);

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show(vbox);

  table = gtk_table_new(4, 2, FALSE);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), table);
  gtk_table_set_col_spacings(GTK_TABLE(table), 5);
  gtk_table_set_row_spacings(GTK_TABLE(table), 3);
  gtk_widget_show(table);

  for(i=0; i<4; i++) {
    entry[i] = gtk_entry_new();
    gtk_widget_set_usize(GTK_WIDGET(entry[i]), 240, 22);
    label = gtk_label_new(labels[i]);
    GTK_WIDGET_UNSET_FLAGS(label, GTK_CAN_FOCUS);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, i, i+1);
    gtk_table_attach_defaults(GTK_TABLE(table), entry[i], 1, 2, i, i+1);
    gtk_signal_connect(GTK_OBJECT(entry[i]), "insert-text",
		       (GtkSignalFunc) text_entered_callback, GINT_TO_POINTER(i));
    gtk_signal_connect(GTK_OBJECT(entry[i]), "changed",
 		       (GtkSignalFunc) entry_changed_callback, GINT_TO_POINTER(i));
    gtk_widget_show(label);
    gtk_widget_show(entry[i]);
  }

  hbox = gtk_hbox_new(TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox);

  button = gtk_radio_button_new_with_label(NULL, "Unsigned");
  GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_widget_show(button);
  gtk_signal_connect(GTK_OBJECT (button), "toggled",
		     GTK_SIGNAL_FUNC(radio_button_callback), NULL);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
  
  group = gtk_radio_button_group(GTK_RADIO_BUTTON(button));
  button = gtk_radio_button_new_with_label(group, "Signed");
  GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show(frame);

  overflow_label = gtk_label_new(NULL);
  GTK_WIDGET_UNSET_FLAGS(overflow_label, GTK_CAN_FOCUS);
  gtk_container_add(GTK_CONTAINER(frame), overflow_label);
  gtk_widget_show(overflow_label);

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);
  gtk_widget_show(sep);

  hbox = gtk_hbox_new(TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox);

  dump_button = gtk_button_new_with_label("Dump");
  gtk_signal_connect(GTK_OBJECT(dump_button), "clicked",
		     (GtkSignalFunc) dump_callback, NULL);
  gtk_box_pack_start(GTK_BOX(hbox), dump_button, TRUE, TRUE, 0);
  gtk_widget_show(dump_button);

  clear_button = gtk_button_new_with_label("Clear");
  gtk_signal_connect(GTK_OBJECT(clear_button), "clicked",
		     (GtkSignalFunc) clear_callback, NULL);
  gtk_box_pack_start(GTK_BOX(hbox), clear_button, TRUE, TRUE, 0);
  gtk_widget_show(clear_button);

  quit_button = gtk_button_new_with_label("Quit");
  gtk_signal_connect(GTK_OBJECT(quit_button), "clicked",
		     (GtkSignalFunc) quit_callback, NULL);
  gtk_box_pack_start(GTK_BOX(hbox), quit_button, TRUE, TRUE, 0);
  gtk_widget_show(quit_button);

  gtk_widget_show(window);

  gtk_main();

  return 0;
}


void text_entered_callback(GtkEditable *editable, gchar *text, gint length,  gint *pos, gpointer data)
{
  g_strup(text);

  if(bad_string(text, length, bases[GPOINTER_TO_INT(data)], *pos)) {
    gtk_signal_emit_stop_by_name(GTK_OBJECT(editable), "insert-text");
    ignore_changed_signal = TRUE;
  }
  return;
}


gboolean bad_string(const char *string, int length, int base, int pos)
{
  static char *charset = "0123456789ABCDEF";
  const gchar *p;
  int i,j = 0;
  gboolean bad_data = FALSE;

  for(i=0, p=string; i<length; i++, p++) {
    if(base == 10 && i == 0 && *p == '-' && val_is_signed && pos == 0) {
      continue;
    }
    for(j=0; j<base; j++) {
      if(toupper(*p) == charset[j])
        break;
    }
    if(j == base) {
      bad_data = TRUE;
      break;
    }
  }
  return bad_data;
}


void entry_changed_callback(GtkEntry *entry, gpointer *data)
{
  int id = GPOINTER_TO_INT(data);
  int i;

  if(ignore_changed_signal) {
    ignore_changed_signal = FALSE;
    return;
  }

  if(strlen(gtk_entry_get_text(GTK_ENTRY(entry))) == 0) {
    clear_entries();
    return;
  }

  errno = 0;

  if(val_is_signed)
    accumulator = strtol(gtk_entry_get_text(GTK_ENTRY(entry)), (char **)NULL, bases[id]);
  else 
    accumulator = strtoul(gtk_entry_get_text(GTK_ENTRY(entry)), (char **)NULL, bases[id]);

  if(errno == ERANGE) {
    gtk_label_set_text(GTK_LABEL(overflow_label), "OVERFLOW");
    overflow = TRUE;
  }
  else {
    gtk_label_set_text(GTK_LABEL(overflow_label), NULL);
    overflow = FALSE;
  }

  for(i=0; i<4; i++) {
    if(i == id)
      continue;
    
    ignore_changed_signal = TRUE;
    update_entry(i);
  }
}


void update_entry(gint entry_no)
{
  generate_string(bases[entry_no]);
  gtk_entry_set_text(GTK_ENTRY(entry[entry_no]), buffer); 
}


void generate_string(int base)
{
  static char *fmt[] = {"%lu", "%lX", "%lo"};
  int pos;
  int i;
  
  if(base == DEC && val_is_signed)
    sprintf(buffer, "%ld", accumulator);
  else {
    switch(base) {
    case DEC:
      sprintf(buffer, "%lu", accumulator);
      break;
    case HEX:
      sprintf(buffer, "%lX", accumulator);
      break;
    case OCT:
      sprintf(buffer, "%lo", accumulator);
      break;
    case BIN:
      for(i = 31; i > 0 && !((1<<i) & accumulator); i--);
      for(pos = 0;i > -1; buffer[pos++] = ((1<<i--) & accumulator) ? '1' : '0');
      buffer[pos] = '\0';
    }
  }
}

gint dump_callback(GtkWidget *widget, gpointer *data)
{
  int i;

  if(overflow)
    return TRUE;

  for(i=0; i<4; i++) {
    generate_string(bases[i]);
    g_print("%s: %s\n", labels[i], buffer);
  }

  return TRUE;
}


gint clear_callback(GtkWidget *widget, gpointer *data)
{
  clear_entries();
  return TRUE;
}


void clear_entries()
{
  int i;

  for(i=0; i<4; i++) {
    ignore_changed_signal = TRUE;
    gtk_entry_set_text(GTK_ENTRY(entry[i]), "");
  }

  accumulator = 0;
}


void radio_button_callback(GtkWidget *widget, gpointer data)
{
  if(GTK_TOGGLE_BUTTON(widget)->active)
    val_is_signed = FALSE;
  else
    val_is_signed = TRUE;

  update_entry(0);
  update_entry(1);
}


gint quit_callback(GtkWidget *widget, gpointer *data)
{
  gtk_exit(0);
}


void quit()
{
  gtk_exit(0);
}

void process_args(int argc, char **argv)
{
  int i,j;
  int type = 0;

  for(i=1; i<argc; i++) {

    if(!strcmp(argv[i], "--help")) {
      usage();
    }
   
    if(!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
      g_print("gbase version %s\n", version);
      gtk_exit(0);
    }

    if(!strcmp(argv[i], "--signed") || !strcmp(argv[i], "-s")) {
      val_is_signed = TRUE;
      continue;
    }

    if(!strcmp(argv[i], "-d"))
      type = DEC;
    else if(!strcmp(argv[i], "-h"))
      type = HEX;
    else if(!strcmp(argv[i], "-o"))
      type = OCT;
    else if(!strcmp(argv[i], "-b"))
      type = BIN;
    else 
      usage();

    if(i == argc)
      usage();

    
    if(bad_string(argv[i+1], strlen(argv[i+1]), type, 0)) {
      g_print("Bad data: %s\n", argv[i+1]);
      gtk_exit(0);
    }
    
    if(val_is_signed)
      accumulator = strtol(argv[i+1], (char **)NULL, type);
    else 
      accumulator = strtoul(argv[i+1], (char **)NULL, type);

    if(errno == ERANGE) {
      g_print("OVERFLOW\n");
      gtk_exit(0);
    }

    for(j=0; j<4; j++) {
      generate_string(bases[j]);
      g_print("%s: %s\n", labels[j], buffer);
    }
    gtk_exit(0);
  }
}


void usage()
{
  g_print("Usage gbase [OPTIONS]\n");
  g_print("\nOptions:\n\n");
  g_print("    --help                This help\n");
  g_print("    --version             Print version number and exit\n\n");
  g_print("    -s                    Treat the number as a signed value\n\n");
  g_print("    -d <decimal number>   Print this decimal number in all bases\n");
  g_print("    -h <hex number>       Print this hexadecimal number in all bases\n");
  g_print("    -o <octal number>     Print this octal number in all bases\n");
  g_print("    -b <binary number>    Print this binary number in all bases\n");
  gtk_exit(0);
}
