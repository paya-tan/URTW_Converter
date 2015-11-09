#include "compressionhelper.h"
#include "main.cc"
#include <gdkmm/general.h>
#include <gdkmm/pixbuf.h>
#include <glibmm/fileutils.h>
#include <cairomm/context.h>
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
  // set_default_size(640,480);

  add(main_box);

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
  row[m_Columns.m_col_name] = "DXT3 (BC3)";

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
      int compress_select = row[m_Columns.m_col_id];
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
      std::cout << "File Selected: " << dialog.get_filename() << std::endl;
      compressionHelper::image_checker(dialog.get_filename());
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
}

void compressionHelper::image_checker(std::string inputFile)
{
  using namespace std;
  std::vector<std::string> raw_data;
  int width;
  int height;
  if (inputFile.find(".URTW") != std::string::npos or inputFile.find(".urtw") != std::string::npos)
  {
    raw_data = URTW_parser(inputFile);
  }
  else
  {
    cout << "Not a URTW file! Support for other types of images will be added later, but for now please use a URTW file.";
  }

  for (int i=0; i<raw_data.size(); i++)
  {
    if (raw_data[i].substr(0,4).compare("TX2D") == 0)
    {
      string marker = raw_data[i].substr(0,4);
      width = atoi(raw_data[i].substr(4,4).c_str());
      height = atoi(raw_data[i].substr(8,4).c_str());
      string raw_image = raw_data[i].substr(12);
      cout << "marker: " << marker << endl << "width: " << width << endl << "height: " << height << endl;

      vector<char> textels;
      for (int x=0; x<raw_image.length(); x+=4)
      {
        textels.push_back(raw_image[x+3]);
      }

      bool A = true;
      bool CHK_IMAGE = true;
      int numitter = 36;
      int length_of_textel_list = textels.size();

      if ( length_of_textel_list < 36)
      {
        numitter = length_of_textel_list;
      }
      for (int z=0; z < numitter; z++)
      {
        //check alpha to see if it's binary or changes at all over the whole file.
        for (unsigned int b=0; b<numitter; b++)
        {
          if (textels[z] != textels[z+1])
          {
            A = false;
          }
          else
          {
            // string input = textels[z].substr(3,1);
            if (textels[z] != static_cast<char>(0x00) and textels[z] != static_cast<char>(0xff))
              CHK_IMAGE = false;
          }
        }
      }

      // for specific channel switching or reversing (as in bmp)
      // for (int a=0; a<raw_image.size(); a+=4)
      // {
      //   char Alpha = raw_image[a];
      //   char R = raw_image[a+1];
      //   char G = raw_image[a+2];
      //   char B = raw_image[a+3];
      //   raw_image[a] = B;
      //   raw_image[a+1] = G;
      //   raw_image[a+2] = R;
      //   raw_image[a+3] = Alpha;
      // }
      // std::reverse(raw_image.begin(), raw_image.end());
      to_compress(raw_image, width, height, A, CHK_IMAGE);
    }
  }
}

std::vector<std::string> compressionHelper::URTW_parser(std::string realFile)
{
  using namespace std;
  string buffer;
  ifstream inFile(realFile.c_str());
  vector<string> inRaw;
  vector<int> mapPos;

  vector<string> mapTypes; //For future handling of stuff like cubemaps.
  mapTypes.push_back("TX2D");

  if (inFile.is_open())
  {
    while(getline(inFile, buffer))
    {
      //Checks the input for markers and adds their positions to a vector.
      for (int i=0; i<mapTypes.size(); i++)
      {
        std::size_t pos = buffer.find(mapTypes[i]);
        if (pos!=std::string::npos)
        {
          mapPos.push_back(pos);
        }
      }

      //Splits the map information into individual strings.
      for (int i=0; i<mapPos.size(); i++)
      {
        if (i+1 <= mapPos.size())
        {
          string mapUnique = buffer.substr(mapPos[i], mapPos[i+1] - mapPos[i]);
          inRaw.push_back(mapUnique);
        }
        else
        {
          string mapUnique = buffer.substr(mapPos[i], buffer.size() - mapPos[i]);
          inRaw.push_back(mapUnique);
        }
      }

      inFile.close();
    }
    return inRaw;

  }
}

void compressionHelper::to_compress(std::string& data, int width, int height, bool A, bool CHK_IMAGE)
{
  // using namespace nvtt;
  nvtt::InputOptions inputOptions;
  inputOptions.setTextureLayout(nvtt::TextureType_2D, width, height, 1);
  inputOptions.setMipmapData(data.c_str(), width, height);
  std::cout << "made it" << std::endl;
  inputOptions.setMipmapGeneration(false, -1);
  inputOptions.setFormat(nvtt::InputFormat_BGRA_8UB);

  nvtt::OutputOptions outputOptions;
  outputOptions.setFileName("output.dds");

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
    if (CHK_IMAGE = true)
      compressionOptions.setFormat(nvtt::Format_BC3);
    else
    {
      if (A = true)
      {
        inputOptions.setAlphaMode(nvtt::AlphaMode_Transparency);
        compressionOptions.setFormat(nvtt::Format_BC1a);
      }
      else
      {
        if (nm_flag)
          compressionOptions.setFormat(nvtt::Format_BC3n);
        else
          compressionOptions.setFormat(nvtt::Format_BC3);
      }
    }
  }
  nvtt::Compressor compressor;
  compressor.process(inputOptions, compressionOptions, outputOptions);
  std::cout << "DONE" << std::endl;
}


//STILL NEEDS TO BE DONE
//   ADD IN INFO IN UI ABOUT SOME DXT VS OTHERS AND
          //NEEDS FIX FOR LARGER FILES
          //POSSIBLY ADD IN SUPPORT FOR OTHER FORMATS
