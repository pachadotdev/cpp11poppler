#' @importFrom cpp11qpdf pdf_combine
#' @rdname cpp11qpdf
#' @name cpp11qpdf
#' @title cpp11qpdf utilities
#' @family cpp11poppler
#' @param input A character vector of input PDF file paths.
#' @param output The output PDF file path.
#' @param password An optional password for encrypted PDFs.
#' @param linearize Logical, whether to linearize the output PDF.
#' @param pages A character vector specifying the pages to include.
#' @export
cpp11qpdf::pdf_combine

#' @importFrom cpp11qpdf pdf_compress
#' @rdname cpp11qpdf
#' @export
cpp11qpdf::pdf_compress

#' @importFrom cpp11qpdf pdf_length
#' @rdname cpp11qpdf
#' @export
cpp11qpdf::pdf_length

#' @importFrom cpp11qpdf pdf_split
#' @rdname cpp11qpdf
#' @export
cpp11qpdf::pdf_split

#' @importFrom cpp11qpdf pdf_subset
#' @rdname cpp11qpdf
#' @export
cpp11qpdf::pdf_subset
