#include "compressionhelper.h"
#include "main.cc"
#include <gdkmm/general.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/pixbufloader.h>
#include <glibmm/fileutils.h>
#include <iostream>
#include <sstream>
#include <algorithm>


compressionHelper::compressionHelper()
: import_button("Convert!"),
normal_map_selector("Normal Map"),
advanced_label("Advanced"),
main_box(Gtk::ORIENTATION_VERTICAL)
{
  set_border_width(5);
  set_title("Compression Helper V0.1");
  set_default_size(640,480);

  add(main_box);

  main_box.pack_start(top_box);
  top_box.pack_start(import_swin);
  import_swin.add(import_frame);
  import_swin.set_size_request(400, 400);
  import_frame.add(import_image);
  main_box.pack_start(inoutbtn_box, Gtk::PACK_SHRINK, 5);
  main_box.pack_end(adv_grid);

  import_button.signal_clicked().connect(sigc::mem_fun(*this,
              &compressionHelper::on_import));

  import_button.set_tooltip_text("Select a URTW list file for conversion and converts it.");

  inoutbtn_box.pack_start(import_button, Gtk::PACK_SHRINK, 20);

  normal_map_selector.signal_clicked().connect(sigc::mem_fun(*this,
              &compressionHelper::on_normal_checked));


  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  select_cformat.set_model(m_refTreeModel);

  Gtk::TreeModel::Row row = *(m_refTreeModel->append());
  row[m_Columns.m_col_id] = 1;
  row[m_Columns.m_col_name] = "DXT1 (BC1)";

  row = *(m_refTreeModel->append());
  row[m_Columns.m_col_id] = 2;
  row[m_Columns.m_col_name] = "DXT1a (BC1a)";

  row = *(m_refTreeModel->append());
  row[m_Columns.m_col_id] = 3;
  row[m_Columns.m_col_name] = "DXT5 (BC3)";

  select_cformat.pack_start(m_Columns.m_col_id);
  select_cformat.pack_start(m_Columns.m_col_name);

  adv_grid.add(advanced_label);
  adv_grid.attach_next_to(select_cformat, advanced_label, Gtk::POS_BOTTOM, 2, 1);
  adv_grid.attach(normal_map_selector, 2, 1, 2, 2);

  select_cformat.signal_changed().connect( sigc::mem_fun(*this, &compressionHelper::on_cselect));

  show_all_children();
}

compressionHelper::~compressionHelper()
{
}

void compressionHelper::on_normal_checked()
{
  if (nm_flag)
    nm_flag = false;
  else
    nm_flag = true;
}

void compressionHelper::on_cselect()
{
  Gtk::TreeModel::iterator iter = select_cformat.get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    if(row)
    {
      compress_select = row[m_Columns.m_col_id];
    }
  }
}

void compressionHelper::on_import()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter_URTW = Gtk::FileFilter::create();
  filter_URTW->set_name("URTW Files");
  filter_URTW->add_pattern("*.urtw");
  filter_URTW->add_pattern("*.URTW");
  dialog.add_filter(filter_URTW);

  Glib::RefPtr<Gtk::FileFilter> filter_all = Gtk::FileFilter::create();
  filter_all->set_name("Anything");
  filter_all->add_pattern("*");
  dialog.add_filter(filter_all);

  int result = dialog.run();

  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      Glib::RefPtr<Gdk::Pixbuf> new_image = compressionHelper::image_checker(dialog.get_filename());
      import_image.set(new_image);
      break;
    }
    case(Gtk::RESPONSE_CANCEL):
    {
      break;
    }
    default:
    {
      break;
    }
  }
  show_all_children();
}

Glib::RefPtr<Gdk::Pixbuf> compressionHelper::image_checker(std::string inputFile)
{
  Glib::RefPtr<Gdk::Pixbuf> new_image;
  Glib::RefPtr<Gdk::Pixbuf> scaled_image;
  int width;
  int height;
  if (inputFile.find(".URTW") != std::string::npos or inputFile.find(".urtw") != std::string::npos)
  {
    std::vector<guint8> raw_data = URTW_parser(inputFile);
    for (int i=0; i<mapPos.size();i++)
    {

      if (raw_data[mapPos[i]] == mapTypes[0] and raw_data[mapPos[i]+1] == mapTypes[1] and raw_data[mapPos[i]+2] == mapTypes[2] and raw_data[mapPos[i]+3] == mapTypes[3])
      {

        std::stringstream s; //Grabs the Width from the header.
        for (int j=0; j!=4; j++)
        {
          s << raw_data[mapPos[i]+4+j];
        }
        width = std::stoi(s.str());

        std::stringstream s2; //Grabs the Height from the header.
        for (int k=0; k!=4; k++)
        {
          s2 << raw_data[mapPos[i]+8+k];
        }
        height = std::stoi(s2.str());

        std::stringstream s3; //Grabs the alpha flag from the header.
        s3 << raw_data[mapPos[i]+12];
        int alpha_check = std::stoi(s3.str());

        int bpp; //Base Bits Per Pixel; on if there is an alpha channel.
        bool alpha_not_binary = false;
        if (alpha_check == 1)
        {
          bpp = 4;
          for (int l=13; l<raw_data.size(); l+=4)
          {
            if (raw_data[l+3] != 0x00 or raw_data[l+3] != 0xff)
            {
              alpha_not_binary = true;
            }
          }
        }
        else
          bpp = 3;

        new_image = Gdk::Pixbuf::create_from_data(&raw_data[mapPos[i]+14], Gdk::Colorspace::COLORSPACE_RGB, alpha_check, 8, width, height, width*bpp);
        scaled_image = new_image->scale_simple(512, 512, Gdk::INTERP_BILINEAR);

        int ck;
        if (i != mapPos.size())
        {
          ck = mapPos[i+1];
        }
        else
        {
          ck = mapPos.size();
        }
        for (int b = mapPos[i]+14; b != ck; b++) //NEEDS TO GO OVER EACH PIXEL FOR EACH MAP
        {
          guint8 ch1 = raw_data[b]; //R
          guint8 ch2 = raw_data[b+1]; //G
          guint8 ch3 = raw_data[b+2]; //B
          guint8 ch4 = raw_data[b+3]; //A
          raw_data[b] = ch3;
          raw_data[b+1] = ch2;
          raw_data[b+2] = ch1;
          raw_data[b+3] = ch4;
        }

        to_compress(raw_data, width, height, alpha_check, alpha_not_binary);
      }
    }
  }
  else
  {
    std::cout << "Not a URTW file! Support for other types of images will be added later, but for now please use a URTW file." << std::endl;
  }
  return scaled_image;
}

std::vector<guint8> compressionHelper::URTW_parser(std::string realFile)
{
  std::ifstream inFile(realFile, std::ios::binary); //Imports the file.
  std::vector<guint8> buffer((std::istreambuf_iterator<char>(inFile)), //Adds everything to a vector.
                        (std::istreambuf_iterator<char>()));

  //Finds our maps for us by searching the vector for the appropriate tags.
  for (int i=0;i<buffer.size();i++) //For every entry in the buffer
  {
    for (int j=0;j<mapTypes.size();j+=4) //For every entry in the mapTypes list
    {
      if (buffer[i] == mapTypes[j] and buffer[i+1] == mapTypes[j+1] and buffer[i+2] == mapTypes[j+2] and buffer[i+3] == mapTypes[j+3]) //Compare the buffer 4 chars at a time to every set of 4 in the mapTypes list, pushing back the position if there is a match.
      {
        mapPos.push_back(i);
      }
    }
  }

  return buffer;
}

void compressionHelper::to_compress(std::vector<guint8> data, int width, int height, int A, bool CHK_IMAGE)
{
  for (int i=0; i<mapPos.size(); i++)
  {
    nvtt::InputOptions inputOptions;
    inputOptions.setTextureLayout(nvtt::TextureType_2D, width, height, 1);
    inputOptions.setMipmapData(&data[mapPos[i]+14], width, height);
    inputOptions.setMipmapGeneration(false, -1);

    std::string filename = "output" + std::to_string(i) + ".dds";
    nvtt::OutputOptions outputOptions;
    outputOptions.setFileName(filename.c_str());

    nvtt::CompressionOptions compressionOptions;
    if (compress_select != 0)
    {
      if (compress_select == 1)
        compressionOptions.setFormat(nvtt::Format_BC1);
      else
      {
        if (compress_select == 2)
          compressionOptions.setFormat(nvtt::Format_BC1a);
        else
        {
          if (compress_select == 3)
            compressionOptions.setFormat(nvtt::Format_BC3);
        }
      }
    }
    else
    {
      if (CHK_IMAGE == 0)
      {
        compressionOptions.setFormat(nvtt::Format_BC1);
      }
      else
      {
        if (A == true)
        {
          inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
          compressionOptions.setFormat(nvtt::Format_BC1a);
        }
        else
        {
          if (nm_flag == true)
          {
            compressionOptions.setFormat(nvtt::Format_BC3n);
          }
          else
          {
            compressionOptions.setFormat(nvtt::Format_BC3);
          }
        }
      }
    }
    nvtt::Compressor compressor;
    compressor.process(inputOptions, compressionOptions, outputOptions);
  }
}
