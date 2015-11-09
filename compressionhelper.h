#ifndef COMPRESSIONHELPER_H
#define COMPRESSIONHELPER_H


#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/alignment.h>
#include <gtkmm/filechooserdialog.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <bitset>

#include <nvtt/nvtt.h>



// static GdkPixbuf * img_import_raw_to_pixbuf( guchar *data);
class compressionHelper : public Gtk::Window
{

public:
  compressionHelper();
  virtual ~compressionHelper();


protected:
  //Signal handlers:
  // virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
  void on_import();
  void on_export();
  void on_normal_checked();
  void image_checker(std::string inputFile);
  std::vector<std::string> URTW_parser(std::string realFile);
  void to_compress(std::string data, int width, int height, bool A, bool CHK_IMAGE);

  bool nm_flag = false;

  //Member widgets:
  Gtk::ScrolledWindow import_swin, export_swin;
  Gtk::Frame import_frame, export_frame;
  Gtk::Box main_box, top_box, bottom_box, imagein_box, imageout_box, inoutbtn_box;
  Gtk::Grid adv_grid;

  Gtk::Label import_label, export_label, advanced_label;
  Gtk::Button import_button, export_button;
  Gtk::CheckButton normal_map_selector;
  Gtk::Alignment text_align;

  int in_select;

};

#endif // COMPRESSIONHELPER_H
