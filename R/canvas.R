canvas <- function(width=480, height=480, file="", ...) {
	invisible(.External("canvas_new_device", file, width, height, ..., PACKAGE="canvas"))
}
