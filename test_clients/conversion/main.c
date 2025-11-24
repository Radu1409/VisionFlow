#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "vf-conversion.h"
#include "vf-logger.h"

// If you run the binary from libraries/conversion/build/linux-x86_64-o:
#define INPUT_DIR      "../../../../vf_frames/raw"
#define OUTPUT_DIR_RGB "../../../../vf_frames/rgb"
#define OUTPUT_DIR_YUV "../../../../vf_frames/yuv"

static int ensure_dir_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        fprintf(stderr, "Path %s exists but is not a director\n", path);
        return -1;
    }

    if (mkdir(path, 0777) != 0) {
        perror("mkdir");
        return -1;
    }
    return 0;
}

static int has_suffix(const char *name, const char *suffix)
{
    size_t nlen = strlen(name);
    size_t slen = strlen(suffix);
    if (nlen < slen) return 0;
    return strcmp(name + nlen - slen, suffix) == 0;
}

int main(int argc, char **argv)
{
    if (argc != 4 && argc != 6) {
        fprintf(stderr,
                "Usage:\n"
                "  %s <rgb|yuv> <in_width> <in_height>\n"
                "  %s rgb <in_width> <in_height> <out_width> <out_height>\n"
                "Exemple:\n"
                "  %s rgb 1280 720          # without resize\n"
                "  %s rgb 1280 720 1920 1080 # resize to 1920x1080\n"
                "  %s yuv 1280 720          # just copy YUV420\n",
                argv[0], argv[0], argv[0], argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *mode = argv[1];
    uint32_t in_w  = (uint32_t)strtoul(argv[2], NULL, 10);
    uint32_t in_h  = (uint32_t)strtoul(argv[3], NULL, 10);

    uint32_t out_w = in_w;
    uint32_t out_h = in_h;

    int to_rgb = 0;

    if (strcmp(mode, "rgb") == 0) {
        to_rgb = 1;
        if (argc == 6) {
            out_w = (uint32_t)strtoul(argv[4], NULL, 10);
            out_h = (uint32_t)strtoul(argv[5], NULL, 10);
        }
    } else if (strcmp(mode, "yuv") == 0) {
        to_rgb = 0;
        // for yuv we ignore any extra parameters
    } else {
        fprintf(stderr, "Mode invalid: %s (use 'rgb' or 'yuv')\n", mode);
        return EXIT_FAILURE;
    }

    if (in_w == 0 || in_h == 0 || out_w == 0 || out_h == 0) {
        fprintf(stderr, "Width/height invalide\n");
        return EXIT_FAILURE;
    }

    // For YUV420 input we need even sizes
    if ((in_w % 2u) != 0u || (in_h % 2u) != 0u) {
        fprintf(stderr,
                "For YUV420 the input width/height must be even (you have %ux%u)\n",
                in_w, in_h);
        return EXIT_FAILURE;
    }

    const char *out_dir = to_rgb ? OUTPUT_DIR_RGB : OUTPUT_DIR_YUV;

    if (ensure_dir_exists(out_dir) != 0) {
        fprintf(stderr, "Unable to create/verify output directory %s\n", out_dir);
        return EXIT_FAILURE;
    }

    DIR *dir = opendir(INPUT_DIR);
    if (!dir) {
        perror("opendir INPUT_DIR");
        return EXIT_FAILURE;
    }

    // YUV420: size = width * height * 3 / 2
    size_t y_plane      = (size_t)in_w * (size_t)in_h;
    size_t chroma_plane = (size_t)(in_w / 2u) * (size_t)(in_h / 2u);
    size_t expected_raw_size = y_plane + 2u * chroma_plane;

    size_t out_size = 0;

    if (to_rgb) {
        out_size = (size_t)out_w * (size_t)out_h * 3u;  // RGB24 at output resolution
    } else {
        out_size = expected_raw_size;                   // copy YUV420
    }

    uint8_t *raw_buf = (uint8_t *)malloc(expected_raw_size);
    if (!raw_buf) {
        fprintf(stderr, "malloc raw_buf failed\n");
        closedir(dir);
        return EXIT_FAILURE;
    }

    // temporary RGB buffer at input resolution (YUV->RGB)
    uint8_t *rgb_in_buf = NULL;
    if (to_rgb) {
        rgb_in_buf = (uint8_t *)malloc((size_t)in_w * (size_t)in_h * 3u);
        if (!rgb_in_buf) {
            fprintf(stderr, "malloc rgb_in_buf failed\n");
            closedir(dir);
            free(raw_buf);
            return EXIT_FAILURE;
        }
    }

    uint8_t *out_buf = (uint8_t *)malloc(out_size);
    if (!out_buf) {
        fprintf(stderr, "malloc out_buf failed\n");
        closedir(dir);
        free(raw_buf);
        free(rgb_in_buf);
        return EXIT_FAILURE;
    }

    struct dirent *de;
    int files_processed = 0;

    while ((de = readdir(dir)) != NULL) {
        // skip over . and ..
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        if (!has_suffix(de->d_name, ".raw")) {
            continue;
        }

        char in_path[512];
        snprintf(in_path, sizeof(in_path), "%s/%s", INPUT_DIR, de->d_name);

        FILE *fin = fopen(in_path, "rb");
        if (!fin) {
            fprintf(stderr, "Cannot open %s: %s\n", in_path, strerror(errno));
            continue;
        }

        size_t read_bytes = fread(raw_buf, 1, expected_raw_size, fin);
        fclose(fin);

        if (read_bytes != expected_raw_size) {
            fprintf(stderr,
                    "Unexpected size for %s (read %zu, expected %zu)\n",
                    in_path, read_bytes, expected_raw_size);
            continue;
        }

        vf_err_t st;

        if (to_rgb) {
            // 1) YUV420 (in_w x in_h) -> RGB (in_w x in_h)
            st = vf_yuv420_to_rgb24(raw_buf, expected_raw_size,
                                    rgb_in_buf,
                                    (size_t)in_w * (size_t)in_h * 3u,
                                    in_w, in_h);
            if (st != VF_SUCCESS) {
                fprintf(stderr,
                        "YUV420->RGB24 conversion failed for %s (status=%d)\n",
                        in_path, (int)st);
                continue;
            }

            if (out_w == in_w && out_h == in_h) {
                // without resizing: copy directly to out_buf
                memcpy(out_buf, rgb_in_buf, (size_t)in_w * (size_t)in_h * 3u);
            } else {
                // 2) RGB (in_w x in_h) -> RGB (out_w x out_h) with nearest-neighbor
                st = vf_rgb24_resize_nearest(rgb_in_buf,
                                             (size_t)in_w * (size_t)in_h * 3u,
                                             out_buf,
                                             out_size,
                                             in_w, in_h,
                                             out_w, out_h);
                if (st != VF_SUCCESS) {
                    fprintf(stderr,
                            "Resize RGB24 failed for %s (status=%d)\n",
                            in_path, (int)st);
                    continue;
                }
            }
        } else {
            // Copy YUV420 only
            st = vf_yuv420_copy(raw_buf, expected_raw_size,
                                out_buf, out_size,
                                in_w, in_h);
            if (st != VF_SUCCESS) {
                fprintf(stderr,
                        "YUV420 copy failed for %s (status=%d)\n",
                        in_path, (int)st);
                continue;
            }
        }

        const char *ext = to_rgb ? ".rgb" : ".yuv";

        char base_name[256];
        strncpy(base_name, de->d_name, sizeof(base_name));
        base_name[sizeof(base_name) - 1] = '\0';

        char *dot = strrchr(base_name, '.');
        if (dot) *dot = '\0';

        char out_path[512];
        snprintf(out_path, sizeof(out_path), "%s/%s%s", out_dir, base_name, ext);

        FILE *fout = fopen(out_path, "wb");
        if (!fout) {
            fprintf(stderr, "Cannot open %s for writing: %s\n",
                    out_path, strerror(errno));
            continue;
        }

        size_t written = fwrite(out_buf, 1, out_size, fout);
        fclose(fout);

        if (written != out_size) {
            fprintf(stderr,
                    "Partial writing for %s (written %zu, expected %zu)\n",
                    out_path, written, out_size);
            continue;
        }

        printf("OK: %s -> %s\n", in_path, out_path);
        ++files_processed;
    }

    closedir(dir);
    free(raw_buf);
    free(rgb_in_buf);
    free(out_buf);

    if (files_processed == 0) {
        log_error("Didn't processed any .raw file from %s\n", INPUT_DIR);

        return EXIT_FAILURE;
    }

    log_info("Processed %d files.\n", files_processed);

    return EXIT_SUCCESS;
}

