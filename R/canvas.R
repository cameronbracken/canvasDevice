#' Canvas Graphics Device
#'
#' R graphics device targetting the HTML canvas element
#' 
#' Initializes a new graphics device that implements the 
#' CanvasRenderingContext2D javascript api.
#'
#' @param file A character string indicating the desired path to the output
#'   file.
#' @param width The width of the output figure, in \bold{pixels}.
#' @param height The height of the output figure, in \bold{pixels}.
#' @param bg The starting background color for the plot.
#'
#' @return Returns NULL invisibly.
#'
#' @keywords device
#'
#' @examples
#'
#' \dontrun{
#'	# very simple plot
#' canvas(600, 600, file="plot.js")
#' plot(rnorm(4000),rnorm(4000),col="#ff000018",pch=19,cex=2) # semi-transparent red
#' dev.off() # creates a file "plot.js" with the above plot
#' }
#'
#' @export
#' @useDynLib canvasDevice canvas_new_device
canvas <- function(file="", width=640, height=480, bg="transparent", ...) {
	invisible(.External("canvas_new_device", file, width, height, bg, ..., PACKAGE="canvasDevice"))
}
