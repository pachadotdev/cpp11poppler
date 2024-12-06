test_that("rendering pdf", {
  # Expected output files
  file <- system.file("examples", "apache.pdf", package = "cpp11poppler")
  outfiles <- paste0("apache_", 1:5, ".png")

  expect_equal(pdf_convert(file, verbose = FALSE), outfiles)
  expect_true(all(file.exists(outfiles)))
  unlink(outfiles)

  outfiles <- paste0("test_0", 1:5, ".png")
  expect_equal(
    pdf_convert(file, verbose = FALSE, filenames = "test_%02d.%s"),
    outfiles
  )
  expect_true(all(file.exists(outfiles)))
  unlink(outfiles)
})
