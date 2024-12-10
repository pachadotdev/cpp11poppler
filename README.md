
<!-- README.md is generated from README.Rmd. Please edit that file -->

# cpp11poppler

[![R-CMD-check](https://github.com/pachadotdev/cpp11poppler/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/pachadotdev/cpp11poppler/actions/workflows/R-CMD-check.yaml)
[![Project Status: Active – The project has reached a stable, usable
state and is being actively
developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)

## About

The `cpp11poppler` package provides a set of functions for extracting
text, fonts, attachments and metadata from a PDF file.

## Installation

You can install the development version of cpp11poppler like so:

``` r
remotes::install_github("pachadotdev/cpp11poppler")
```

## Example

See the vignette for more examples.

``` r
library(cpp11poppler)
#> Using poppler version 22.02.0
tmpdir <- tempdir()
file <- system.file("examples", "recipes.pdf", package = "cpp11poppler")
pdf_convert(file, pages = 1:2, verbose = FALSE)
#> [1] "recipes_1.png" "recipes_2.png"
```
