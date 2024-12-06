test_that("text in english", {
  file <- system.file("examples", "latkes.pdf", package = "cpp11poppler")
  toc <- pdf_toc(file)
  expect_equal(toc$children[[1]]$title, "Ingredients")

  info <- pdf_info(file)
  expect_equal(info$keys$Author, "Moishe House")
  expect_equal(info$keys$Title, "Latkes Recipe")

  text <- pdf_text(file)
  expect_match(text, "eggs, salt, matzo meal, and any additional seasonings")

  skip_if_not(poppler_config()$has_pdf_data)

  data <- pdf_data(file)
  expect_length(data[[1]]$text, 116L)
  expect_equal(data[[1]]$text[1:2], c("Latkes", "Recipe"))
})

test_that("text in japanese", {
  # skip_on_os("solaris")
  skip_on_cran()
  skip_if(poppler_config()$version < "0.41")

  file <- system.file("examples", "sashimi.pdf", package = "cpp11poppler")

  text <- pdf_text(file)
  sashimi <- "\u523A\u8EAB"
  expect_match(text, sashimi)

  skip_if_not(poppler_config()$has_pdf_data)

  data <- pdf_data(file)
  expect_equal(data[[1]]$text[1:2], c(sashimi, "Wikipedia"))
})

test_that("text in chinese", {
  # skip_on_os("solaris")
  skip_on_cran()
  skip_if(poppler_config()$version < "0.41")

  file <- system.file("examples", "pekingduck.pdf", package = "cpp11poppler")

  text <- pdf_text(file)
  pekingduck <- "\u5317\u4EAC\u70E4\u9E2D"
  expect_match(text, pekingduck)

  skip_if_not(poppler_config()$has_pdf_data)

  data <- pdf_data(file)
  expect_equal(data[[1]]$text[1:2], c(pekingduck, "Wikipedia"))

  fonts <- pdf_fonts(file)
  expect_equal(nrow(fonts), 1)
  expect_true(all(fonts$embedded))
  expect_match(fonts$name, "NotoSerifCJK")
})
