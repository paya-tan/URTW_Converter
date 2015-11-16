#ifndef COMPRESSIONHELPER_H
#define COMPRESSIONHELPER_H


#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>
#include <gtkmm/alignment.h>
#include <gtkmm/filechooserdialog.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <bitset>

#include <nvtt/nvtt.h>

// #include <inc/crnlib.h>


class compressionHelper : public Gtk::Window
{

public:
  compressionHelper();
  virtual ~compressionHelper();


protected:
  //Signal handlers:
  void on_import();
  void on_normal_checked();
  void on_cselect();
  Glib::RefPtr<Gdk::Pixbuf> image_checker(std::string inputFile);
  std::vector<guint8> URTW_parser(std::string realFile);
  void to_compress(std::vector<guint8> data, int width, int height, int A, bool CHK_IMAGE);

  bool nm_flag = false;
  int compress_select = 0;

  std::vector<int> mapPos;
  std::vector<unsigned char> mapTypes{0x54, 0x58, 0x32, 0x44}; //Every 4 chars == 1 map type. Example: 5458 3244 == TX2D.

  std::vector<unsigned char> images;

  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    {
      add(m_col_id); add(m_col_name);
    }

    Gtk::TreeModelColumn<int> m_col_id;
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
  };

  ModelColumns m_Columns;

  //Member widgets:
  Gtk::ScrolledWindow import_swin, export_swin;
  Gtk::Frame import_frame, export_frame;
  Gtk::Box main_box, top_box, bottom_box, imagein_box, imageout_box, inoutbtn_box;
  Gtk::Grid adv_grid;

  Gtk::Label import_label, advanced_label;
  Gtk::Button import_button;
  Gtk::CheckButton normal_map_selector;
  Gtk::ComboBox select_cformat;
  Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
  Gtk::Alignment text_align;

  Gtk::Image import_image, export_image;

  int in_select;

};

#endif // COMPRESSIONHELPER_H
