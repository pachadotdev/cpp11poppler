# Poppler ----

if (!file.exists("../windows/poppler/include/poppler/cpp/poppler-document.h")) {
  unlink("../windows", recursive = TRUE)
  url <- if (grepl("aarch", R.version$platform)) {
    "https://github.com/pachadotdev/cpp11bundles/releases/download/poppler-23.08.0/poppler-23.08.0-clang-aarch64.tar.xz"
  } else if (grepl("clang", Sys.getenv("R_COMPILED_BY"))) {
    "https://github.com/pachadotdev/cpp11bundles/releases/download/poppler-23.08.0/poppler-23.08.0-clang-x86_64.tar.xz"
  } else {
    "https://github.com/pachadotdev/cpp11bundles/releases/download/poppler-23.08.0/poppler-23.08.0-ucrt-x86_64.tar.xz"
  }
  download.file(url, basename(url), quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  untar(basename(url), exdir = "../windows", tar = "internal")
  unlink(basename(url))
  setwd("../windows")
  file.rename(list.files(), "poppler")
}

# Harfbuzz ----

if (!file.exists("../windows/harfbuzz/include/harfbuzz/hb.h")) {
  unlink("../windows/harfbuzz", recursive = TRUE)
  url <- if (grepl("aarch", R.version$platform)) {
    "https://github.com/pachadotdev/readpdf/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-clang-aarch64.tar.xz"
  } else if (grepl("clang", Sys.getenv("R_COMPILED_BY"))) {
    "https://github.com/pachadotdev/readpdf/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-clang-x86_64.tar.xz"
  } else if (getRversion() >= "4.3") {
    "https://github.com/pachadotdev/readpdf/releases/download/harfbuzz-8.2.1/harfbuzz-8.2.1-ucrt-x86_64.tar.xz"
  }
  download.file(url, basename(url), quiet = TRUE)
  untar(basename(url), exdir = "../windows/", tar = "internal")
  file.rename(file.path("../windows", dir("../windows", pattern = "harfbuzz-")), "../windows/harfbuzz")
  unlink(basename(url))
}
