#' Render / Convert PDF
#'
#' High quality conversion of pdf page(s) to png, jpeg or tiff format, or render into a
#' raw bitmap array for further processing in R.
#'
#' @export
#' @name rendering
#' @rdname pdf_render_page
#' @param pdf file path or raw vector with pdf data
#' @param page which page to render
#' @param numeric convert raw output to (0-1) real values
#' @param dpi resolution (dots per inch) to render
#' @param antialias enable antialiasing. Must be `"text"` or `"draw"` or `TRUE` (both)
#' or `FALSE` (neither).
#' @param opw owner password
#' @param upw user password
#' @family cpp11poppler
#' @aliases render
#' @examples # Rendering should be supported on all platforms now
#' # convert pdf to png
#' tmpdir <- tempdir()
#' file <- system.file("examples", "apache.pdf", package = "cpp11poppler")
#' outfiles <- paste0(tmpdir, "/apache_", 1:5, ".png")
#' pdf_convert(file, verbose = FALSE, filenames = paste0(tmpdir, "/apache_%d.%s"))
#'
#' # render into raw bitmap
#' # bitmap <- pdf_render_page(file) # memory leak, fix later!
#'
#' # save to bitmap formats
#' # png::writePNG(bitmap, paste0(tmpdir, "/page.png"))
#' # webp::write_webp(bitmap, paste0(tmpdir, "/page.webp"))
pdf_render_page<- function(pdf, page = 1, dpi = 300, numeric = FALSE, antialias = TRUE, opw = "", upw = "") {
  antialiasing <- isTRUE(antialias) || isTRUE(antialias == "draw")
  text_antialiasing <- isTRUE(antialias) || isTRUE(antialias == "text")
  out <- poppler_render_page(loadfile(pdf), page, dpi, opw, upw, antialiasing, text_antialiasing)
  if(identical(dim(out)[1], 4L)){
    out <- out[c(3,2,1,4),,, drop = FALSE] ## convert ARGB to RGBA
  }
  if(isTRUE(numeric)){
    out <- structure(as.numeric(out)/255, dim = dim(out))
    out <- aperm(out)
  } else {
    class(out) <- c("bitmap", "rgba")
  }
  return(out)
}

#' @export
#' @rdname pdf_render_page
#' @param format string with output format such as `"png"` or `"jpeg"`. Must be equal
#' to one of `poppler_config()$supported_image_formats`.
#' @param pages vector with one-based page numbers to render. `NULL` means all pages.
#' @param filenames vector of equal length to `pages` with output filenames. May also be
#' a format string which is expanded using `pages` and `format` respectively.
#' @param verbose print some progress info to stdout
pdf_convert <- function(pdf, format = "png", pages = NULL, filenames = NULL , dpi = 300,
                        antialias = TRUE, opw = "", upw = "", verbose = TRUE){
  config <- poppler_config()
  if(!config$can_render || !length(config$supported_image_formats))
    stop("You version of libppoppler does not support rendering")
  format <- match.arg(format, poppler_config()$supported_image_formats)
  if(is.null(pages))
    pages <- seq_len(pdf_info(pdf, opw = opw, upw = upw)$pages)
  if(!is.numeric(pages) || !length(pages))
    stop("Argument 'pages' must be a one-indexed vector of page numbers")
  if(length(filenames) < 2){
    input <- ifelse(is.raw(pdf), 'output', sub(".pdf", "", basename(pdf), fixed = TRUE))
    filenames <- if (length(filenames)) {
      sprintf(filenames, pages, format)
    } else {
      sprintf("%s_%d.%s", input, pages, format)
    }
  }
  if(length(filenames) != length(pages))
    stop("Length of 'filenames' must be one or equal to 'pages'")
  antialiasing <- isTRUE(antialias) || isTRUE(antialias == "draw")
  text_antialiasing <- isTRUE(antialias) || isTRUE(antialias == "text")
  poppler_convert(loadfile(pdf), format, pages, filenames, dpi, opw, upw, antialiasing, text_antialiasing, verbose)
}


#' @export
#' @rdname pdf_render_page
poppler_config <- function(){
  get_poppler_config()
}
