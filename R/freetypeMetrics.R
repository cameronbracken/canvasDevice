#' Font Metrics
#'
#' Get font metrics from freetype
#' 
#'
#' @param file font file (must be readable by freetype)
#' @param text string 
#'
#' @return A list containing metrics (width, height, ascent, descent) 
#'         for the given string (in pixels).
#'
#' @keywords device
#'
#' @examples
#'
#' \dontrun{
#'    font_name <- '/Library/Fonts/Arial Regular.ttf'
#'
#'    # one off usage on a multi-character string
#'    fontMetrics('A Text String', font_name)
#'  
#'    # get metrics for the whole alphabet!
#'    Vectorize(fontMetrics, 'string')(font_name, c(letters,LETTERS))
#' }
#'
#' @export
#' @useDynLib canvasDevice 
fontMetrics <- function(string, file="", ps = 12, encoding = "UTF-8") {

    # check if the file exists otherwise bail out
    if(!file.exists(file)) stop(sprintf('%s does not exist.', file))

    # Set the encoding of the string if it is not explicitly set
    if(Encoding(string) == "unknown")
        Encoding(string) <- encoding

    # convert the string to UTF-8
    string <- enc2utf8(string)

    codes <- utf8ToInt(string)
    n <- length(codes)
    
    metrics <- .Call("freetypeMetrics", 
        as.character(file), 
        as.integer(codes), 
        as.integer(n),
        as.integer(ps),
        package='canvasDevice')
    
    return(metrics)
}
