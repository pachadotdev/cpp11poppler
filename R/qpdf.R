#' @importFrom qpdf pdf_combine
#' @rdname qpdf
#' @name qpdf
#' @title qpdf utilities
#' @family cpp11poppler
#' @param input A character vector of input PDF file paths.
#' @param output The output PDF file path.
#' @param password An optional password for encrypted PDFs.
#' @param linearize Logical, whether to linearize the output PDF.
#' @param pages A character vector specifying the pages to include.
#' @export
qpdf::pdf_combine

#' @importFrom qpdf pdf_compress
#' @rdname qpdf
#' @export
qpdf::pdf_compress

#' @importFrom qpdf pdf_length
#' @rdname qpdf
#' @export
qpdf::pdf_length

#' @importFrom qpdf pdf_split
#' @rdname qpdf
#' @export
qpdf::pdf_split

#' @importFrom qpdf pdf_subset
#' @rdname qpdf
#' @export
qpdf::pdf_subset
