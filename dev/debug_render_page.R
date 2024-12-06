devtools::load_all()
tmpdir <- tempdir()
file <- system.file("examples", "apache.pdf", package = "cpp11poppler")
raw_data <- loadfile(file)
poppler_render_page(x = raw_data, page = 1L, dpi = 300, opw = "", upw = "",
  antialiasing = T, text_antialiasing = T)
