#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct StringMetrics
{
    int height;
    int descent;
    int ascent;
    int width;

} StringMetrics;

StringMetrics getFreetypeMetrics(const char *filename, 
    const int *char_codes, const int num_chars, const int point_size);

SEXP freetypeMetrics(SEXP filename_r, SEXP char_codes_r, SEXP num_chars_r, SEXP point_size_r);
