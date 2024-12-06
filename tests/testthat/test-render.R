test_that("rendering pdf", {
  # Expected output files
  tmpdir <- tempdir()
  file <- system.file("examples", "apache.pdf", package = "cpp11poppler")
  outfiles <- paste0(tmpdir, "/apache_", 1:5, ".png")

  expect_equal(pdf_convert(file, verbose = FALSE, filenames = paste0(tmpdir, "/apache_%d.%s")), outfiles)
  expect_true(all(file.exists(outfiles)))
})
