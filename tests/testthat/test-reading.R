test_that("reading password protected pdf", {
  # Read encrypted file with password
  file <- "password.pdf"

  if(poppler_config()$version >= "0.30"){
    expect_message(expect_true(pdf_info(file)$locked))
    expect_error(expect_message(pdf_text(file), "locked"))
  } else{
    expect_true(pdf_info(file)$locked)
    expect_error(pdf_text(file), "locked")
  }
  expect_equal(1L, length(pdf_text(file, upw = "userpwd")))
  expect_false(pdf_info(file, upw = "userpwd")$locked)
})
