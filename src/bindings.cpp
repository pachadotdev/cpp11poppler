#include <poppler-document.h>
#include <poppler-embedded-file.h>
#include <poppler-font.h>
#include <poppler-image.h>
#include <poppler-page-renderer.h>
#include <poppler-page.h>
#include <poppler-toc.h>
#include <poppler-version.h>

#include <algorithm>  // For std::min
#include <cpp11.hpp>
#include <cstring>
#include <memory>  // For std::unique_ptr
#include <sstream>
#include <iostream>
#include <fstream>

#if defined(POPPLER_VERSION_MINOR)
#define VERSION_AT_LEAST(x, y)  \
  (POPPLER_VERSION_MAJOR > x || \
   (POPPLER_VERSION_MAJOR == x && POPPLER_VERSION_MINOR >= y))
#else
#define VERSION_AT_LEAST(x, y) 0
#endif

#if VERSION_AT_LEAST(22, 5)
#define info_date info_date_t
#define creation_date creation_date_t
#define modification_date modification_date_t
#endif

#if VERSION_AT_LEAST(0, 71)
#define POPPLER_HAS_PAGE_TEXT_LIST
#endif

#if VERSION_AT_LEAST(0, 89)
#define POPPLER_HAS_LOCAL_FONT_INFO
#endif

using namespace cpp11;
using namespace poppler;

[[cpp11::register]] bool set_poppler_data(std::string path) {
#ifdef BUNDLE_POPPLER_DATA
  return poppler::set_data_dir(path);
#else
  return false;
#endif
}

std::string ustring_to_utf8(ustring x) {
  byte_array buf = x.to_utf8();
  std::string str(buf.begin(), buf.end());
  if (str.length() && str.back() == '\f') str.erase(str.length() - 1);
#ifdef _WIN32
  str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
#endif
  return str;
}

static cpp11::list item_to_list(toc_item* item) {
  cpp11::writable::list children_list;

  if (item == nullptr) {
    return children_list;
  }

  std::vector<toc_item*> children = item->children();

  for (size_t i = 0; i < children.size(); i++) {
    children_list.push_back(item_to_list(children[i]));
  }

  return cpp11::writable::list({
    "title"_nm = ustring_to_utf8(item->title()),
    "children"_nm = children_list
  });
}

static std::string layout_string(poppler::document::page_layout_enum layout) {
  switch (layout) {
    case poppler::document::no_layout:
      return "no_layout";
    case poppler::document::single_page:
      return "single_page";
    case poppler::document::one_column:
      return "one_column";
    case poppler::document::two_column_left:
      return "two_column_left";
    case poppler::document::two_column_right:
      return "two_column_right";
    case poppler::document::two_page_left:
      return "two_page_left";
    case poppler::document::two_page_right:
      return "two_page_right";
    default:
      return "";
  }
}

static std::string font_string(font_info::type_enum x) {
  switch (x) {
    case font_info::unknown:
      return "unknown";
    case font_info::type1:
      return "type1";
    case font_info::type1c:
      return "type1c";
    case font_info::type1c_ot:
      return "type1c_ot";
    case font_info::type3:
      return "type3";
    case font_info::truetype:
      return "truetype";
    case font_info::truetype_ot:
      return "truetype_ot";
    case font_info::cid_type0:
      return "cid_type0";
    case font_info::cid_type0c:
      return "cid_type0c";
    case font_info::cid_type0c_ot:
      return "cid_type0c_ot";
    case font_info::cid_truetype:
      return "cid_truetype";
    case font_info::cid_truetype_ot:
      return "cid_truetype_ot";
    default:
      return "";
  }
}

static poppler::document* read_raw_pdf(cpp11::raws x, std::string opw,
                                       std::string upw,
                                       bool info_only = false) {
  const unsigned char* data = reinterpret_cast<const unsigned char*>(RAW(x));
  size_t size = Rf_length(x);

  // Debugging information
  // std::string debug_info = "Data size: " + std::to_string(size);
  // cpp11::message(debug_info.c_str());
  // std::string first_bytes;
  // for (size_t i = 0; i < std::min(size, static_cast<size_t>(10)); ++i) {
  //   first_bytes += std::to_string(data[i]) + " ";
  // }
  // std::string first_bytes_info = "First bytes: " + first_bytes;
  // cpp11::message(first_bytes_info.c_str());

  poppler::document* doc = document::load_from_raw_data(
      reinterpret_cast<const char*>(data), size, opw, upw);
  if (!doc) {
    throw std::runtime_error("PDF parsing failure.");
  }
  if (!info_only && doc->is_locked()) {
    throw std::runtime_error("PDF file is locked. Invalid password?");
  }
  return doc;
}

[[cpp11::register]] cpp11::list get_poppler_config() {
  try {
    // Fetch Poppler version string
    std::string version = poppler::version_string();

    // Check rendering capabilities
    bool can_render = poppler::page_renderer::can_render();

    // Check for PDF data support
#if defined(POPPLER_HAS_PAGE_TEXT_LIST)
    bool has_pdf_data = true;
#else
    bool has_pdf_data = false;
#endif

    // Check for local font information support
#if defined(POPPLER_HAS_LOCAL_FONT_INFO)
    bool has_local_font_info = true;
#else
    bool has_local_font_info = false;
#endif

    // Get supported image formats
    std::vector<std::string> supported_formats =
        poppler::image::supported_image_formats();

    // Convert supported formats to writable::strings
    cpp11::writable::strings formats;
    for (const auto& format : supported_formats) {
      formats.push_back(format);
    }

    // Return configuration as a list
    return cpp11::writable::list({
        {"version"_nm = version},
        {"can_render"_nm = can_render},
        {"has_pdf_data"_nm = has_pdf_data},
        {"has_local_font_info"_nm = has_local_font_info},
        {"supported_image_formats"_nm = formats},
    });
  } catch (const std::exception& e) {
    cpp11::stop("Failed to retrieve Poppler configuration: %s", e.what());
  }
}

[[cpp11::register]] cpp11::list poppler_pdf_info(cpp11::raws x, std::string opw,
                                                 std::string upw) {
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw, true));
  if (!doc) {
    throw std::runtime_error("Failed to load PDF document.");
  }

  // Handle locked documents
  if (doc->is_locked()) {
    cpp11::writable::list locked_info;
    locked_info.push_back({"encrypted"_nm = doc->is_encrypted()});
    locked_info.push_back({"linearized"_nm = doc->is_linearized()});
    locked_info.push_back({"created"_nm = doc->info_date("CreationDate")});
    locked_info.push_back({"modified"_nm = doc->info_date("ModDate")});
    locked_info.push_back({"locked"_nm = doc->is_locked()});
    return locked_info;
  }

  // Extract metadata keys
  cpp11::writable::list keys;
  std::vector<std::string> keystrings = doc->info_keys();
  cpp11::writable::strings keys_names;
  for (size_t i = 0; i < keystrings.size(); i++) {
    std::string keystr = keystrings[i];
    if (keystr.compare("CreationDate") == 0) continue;
    if (keystr.compare("ModDate") == 0) continue;
    keys.push_back({"keystr_tmp"_nm = ustring_to_utf8(doc->info_key(keystr))});
    keys_names.push_back(keystr);
  }
  keys.attr("names") = keys_names;

  // Extract version string
  int major = 0, minor = 0;
  doc->get_pdf_version(&major, &minor);
  std::ostringstream version_stream;
  version_stream << major << "." << minor;

  // Build the result list
  cpp11::writable::list result;
  result.push_back({"version"_nm = version_stream.str()});
  result.push_back({"pages"_nm = doc->pages()});
  result.push_back({"encrypted"_nm = doc->is_encrypted()});
  result.push_back({"linearized"_nm = doc->is_linearized()});
  result.push_back({"keys"_nm = keys});
  result.push_back({"created"_nm = doc->info_date("CreationDate")});
  result.push_back({"modified"_nm = doc->info_date("ModDate")});
  result.push_back({"metadata"_nm = doc->metadata()});
  result.push_back({"locked"_nm = doc->is_locked()});
  result.push_back({"attachments"_nm = doc->has_embedded_files()});
  result.push_back({"layout"_nm = layout_string(doc->page_layout())});

  return result;
}

[[cpp11::register]] list poppler_pdf_data(cpp11::raws x, bool get_font_info,
                                     std::string opw, std::string upw) {
#ifdef POPPLER_HAS_PAGE_TEXT_LIST
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  cpp11::writable::list out(doc->pages());
  for (int i = 0; i < doc->pages(); i++) {
    std::unique_ptr<poppler::page> p(doc->create_page(i));
    if (!p) continue;  // missing page
      // poppler::page::text_list() does not resolve the font information. To
      // got it, text_list() must be called with opt_flag = 1, cf popper::page
      // class reference
#ifdef POPPLER_HAS_LOCAL_FONT_INFO
    std::vector<text_box> boxes(p->text_list(
        get_font_info ? poppler::page::text_list_include_font : 0));
#else
    if (get_font_info)
      throw std::runtime_error(
          std::string("Getting font data requires poppler >= 0.89. You have ") +
          POPPLER_VERSION);
    std::vector<text_box> boxes(p->text_list());
#endif
    size_t boxes_size = boxes.size();
    cpp11::writable::strings text(boxes_size);
    cpp11::writable::integers width(boxes_size);
    cpp11::writable::integers height(boxes_size);
    cpp11::writable::integers x(boxes_size);
    cpp11::writable::integers y(boxes_size);
#ifdef POPPLER_HAS_LOCAL_FONT_INFO
    cpp11::writable::strings font_name(boxes_size);
    cpp11::writable::doubles font_size(boxes_size);
#endif
    cpp11::writable::logicals space(boxes_size);
    for (size_t j = 0; j < boxes.size(); j++) {
      text[j] = ustring_to_utf8(boxes.at(j).text());
      width[j] = boxes.at(j).bbox().width();
      height[j] = boxes.at(j).bbox().height();
      x[j] = boxes.at(j).bbox().x();
      y[j] = boxes.at(j).bbox().y();
#ifdef POPPLER_HAS_LOCAL_FONT_INFO
      if (get_font_info) {
        if (boxes.at(j).has_font_info()) {
          font_name[j] = boxes.at(j).get_font_name();
          font_size[j] = boxes.at(j).get_font_size();
        } else {
          font_name[j] = NA_STRING;
          font_size[j] = NA_REAL;
        }
      }
#endif
      space[j] = boxes.at(j).has_space_after();
    }
    cpp11::writable::list df({
      "width"_nm = width, "height"_nm = height, "x"_nm = x,
      "y"_nm = y, "space"_nm = space, "text"_nm = text
    });
#ifdef POPPLER_HAS_LOCAL_FONT_INFO
    if (get_font_info) {
      df.push_back("font_name"_nm = font_name);
      df.push_back("font_size"_nm = font_size);
    }
#endif
    out[i] = cpp11::writable::data_frame(df);
  }
  return out;
#else  // POPPLER_HAS_PAGE_TEXT_LIST
  throw std::runtime_error(
      std::string("pdf_data() requires poppler >= 0.71. You have ") +
      POPPLER_VERSION);
#endif
}

[[cpp11::register]] cpp11::strings poppler_pdf_text(cpp11::raws x,
                                                     std::string opw,
                                                     std::string upw) {
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  cpp11::writable::strings out(doc->pages());
  for (int i = 0; i < doc->pages(); i++) {
    std::unique_ptr<poppler::page> p(doc->create_page(i));
    if (!p) continue;  // missing page
    page::text_layout_enum show_text_layout = page::physical_layout;

    /* media_box includes text in margins:
     * https://github.com/ropensci/pdftools/issues/67 */
    rectf target(p->page_rect(media_box));

    /* Workaround for bug https://github.com/ropensci/pdftools/issues/7 */
    if (p->orientation() == page::landscape ||
        p->orientation() == page::seascape) {
      target.set_right(target.right() * 2);
    }

    /* Rescale page to start at 0 (#24) */
    if (target.top() < 0) {
      target.set_bottom(target.bottom() - target.top());
      target.set_top(0);
    }

    /* Extract text */
    out[i] = ustring_to_utf8(p->text(target, show_text_layout));
  }
  return out;
}

[[cpp11::register]] cpp11::data_frame poppler_pdf_pagesize(cpp11::raws x, std::string opw,
                                                   std::string upw) {
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  int len = doc->pages();
  cpp11::writable::doubles top(len);
  cpp11::writable::doubles right(len);
  cpp11::writable::doubles bottom(len);
  cpp11::writable::doubles left(len);
  cpp11::writable::doubles width(len);
  cpp11::writable::doubles height(len);

  for (int i = 0; i < len; i++) {
    std::unique_ptr<poppler::page> p(doc->create_page(i));
    if (!p) continue;  // missing page
    rectf rect(p->page_rect());
    top[i] = rect.top();
    bottom[i] = rect.bottom();
    left[i] = rect.left();
    right[i] = rect.right();
    width[i] = rect.width();
    height[i] = rect.height();
  }
  return cpp11::writable::data_frame({"top"_nm = top, "right"_nm = right,
                                     "bottom"_nm = bottom, "left"_nm = left,
                                     "width"_nm = width, "height"_nm = height});
}

[[cpp11::register]] cpp11::list poppler_pdf_fonts(cpp11::raws x, std::string opw, std::string upw) {
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  std::vector<font_info> fonts = doc->fonts();
  cpp11::writable::strings fonts_name;
  cpp11::writable::strings fonts_type;
  cpp11::writable::strings fonts_file;
  cpp11::writable::logicals fonts_embedded;
  for (size_t i = 0; i < fonts.size(); i++) {
    font_info font = fonts[i];
    fonts_name.push_back(font.name());
    fonts_type.push_back(font_string(font.type()));
    fonts_file.push_back(font.file());
    fonts_embedded.push_back(font.is_embedded());
  }
  return cpp11::writable::data_frame(
      {"name"_nm = fonts_name, "type"_nm = fonts_type,
       "embedded"_nm = fonts_embedded, "file"_nm = fonts_file});
}

[[cpp11::register]] cpp11::list poppler_pdf_files(cpp11::raws x, std::string opw,
                                           std::string upw) {
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  cpp11::writable::list out;
  if (doc->has_embedded_files()) {
    std::vector<embedded_file*> files = doc->embedded_files();
    for (size_t i = 0; i < files.size(); i++) {
      embedded_file* file = files[i];
      poppler::byte_array data = file->data();
      cpp11::writable::raws res(data.size());
      std::copy(data.begin(), data.end(), res.begin());
      out.push_back({"name"_nm = file->name()});
      out.push_back({"mime"_nm = file->mime_type()});
      out.push_back({"created"_nm = file->creation_date()});
      out.push_back({"modified"_nm = file->modification_date()});
      out.push_back({"description"_nm = ustring_to_utf8(file->description())});
      out.push_back({"data"_nm = res});
    }
  }
  return out;
}

[[cpp11::register]] cpp11::list poppler_pdf_toc(cpp11::raws x, std::string opw, std::string upw) {
  try {
    std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
    if (!doc) {
      throw std::runtime_error("Failed to load PDF document.");
    }

    std::unique_ptr<toc> contents(doc->create_toc());
    if (!contents) {
      return writable::list();
    }

    return item_to_list(contents->root());
  } catch (const std::exception& e) {
    cpp11::stop("PDF parsing failure: %s", e.what());
  }
}

[[cpp11::register]] writable::raws poppler_render_page(
    raws x, int pagenum, double dpi, std::string opw, std::string upw,
    bool antialiasing = true, bool text_antialiasing = true) {
  if (!page_renderer::can_render())
    throw std::runtime_error("Rendering not supported on this platform!");

  std::unique_ptr<document> doc(read_raw_pdf(x, opw, upw));
  std::unique_ptr<page> p(doc->create_page(pagenum - 1));
  if (!p) throw std::runtime_error("Invalid page.");

  page_renderer pr;
  pr.set_render_hint(page_renderer::antialiasing, antialiasing);
  pr.set_render_hint(page_renderer::text_antialiasing, text_antialiasing);

  image img = pr.render_page(p.get(), dpi, dpi);
  if (!img.is_valid()) throw std::runtime_error("PDF rendering failure.");

  writable::raws res(img.bytes_per_row() * img.height());
  std::memcpy(res.data(), img.data(), res.size());

  res.attr("dim") = writable::doubles(
      {static_cast<double>(img.width()), static_cast<double>(img.height())});
  return res;
}

[[cpp11::register]] std::vector<std::string> poppler_convert(
    cpp11::raws x, std::string format, std::vector<int> pages,
    std::vector<std::string> names, double dpi, std::string opw,
    std::string upw, bool antialiasing = true, bool text_antialiasing = true,
    bool verbose = true) {
  if (!page_renderer::can_render())
    throw std::runtime_error("Rendering not supported on this platform!");
  std::unique_ptr<poppler::document> doc(read_raw_pdf(x, opw, upw));
  for (size_t i = 0; i < pages.size(); i++) {
    int pagenum = pages[i];
    std::string filename = names[i];
    if (verbose)
      cpp11::message("Converting page %d to %s...", pagenum, filename.c_str());
    std::unique_ptr<poppler::page> p(doc->create_page(pagenum - 1));
    if (!p) throw std::runtime_error("Invalid page.");
    page_renderer pr;
    pr.set_render_hint(page_renderer::antialiasing, antialiasing);
    pr.set_render_hint(page_renderer::text_antialiasing, text_antialiasing);
    image img = pr.render_page(p.get(), dpi, dpi);
    if (!img.is_valid()) throw std::runtime_error("PDF rendering failure.");
    if (!img.save(filename, format, dpi))
      throw std::runtime_error("Failed to save file" + filename);
    if (verbose) cpp11::message("done.");
  }
  return names;
}

static void error_callback(const std::string& msg, void* context) {
  cpp11::function err_cb = cpp11::package("cpp11poppler")["err_cb"];
  err_cb(cpp11::writable::strings({msg.c_str()}));
}

[[cpp11::register]] void set_error_callback() {
#if VERSION_AT_LEAST(0, 30)
  poppler::set_debug_error_function(error_callback, NULL);
#endif
}
