poppler_render_page <- function(pdf, page, dpi, opw, upw, antialiasing, text_antialiasing) {
  res <- poppler_render_page_(pdf, page, dpi, opw, upw, antialiasing, text_antialiasing)
  n_matrices <- length(res)
  matrix_dim <- dim(res[[1]])
  res_array <- array(NA, dim = c(matrix_dim[2], matrix_dim[1], n_matrices))
  for (i in seq_along(res)) {
    res_array[, , i] <- as.numeric(t(res[[i]]))
  }
  res_array
}
