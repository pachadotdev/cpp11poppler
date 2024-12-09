test_that("rendering pdf", {
  # Expected output files
  tmpdir <- tempdir()
  file <- "latkes.pdf"
  outfiles <- paste0(tmpdir, "/latkes_", 1, ".png")

  expect_equal(pdf_convert(file, verbose = FALSE, filenames = paste0(tmpdir, "/latkes_%d.%s")), outfiles)
  expect_true(all(file.exists(outfiles)))
})
