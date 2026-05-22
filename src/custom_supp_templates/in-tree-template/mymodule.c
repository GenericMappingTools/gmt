/*--------------------------------------------------------------------
 *
 *  In-tree custom supplement template — module source.
 *  See ../README.md for the supplement mechanism overview.
 *
 *  Rename this file, the THIS_MODULE_* macros, and the GMT_mymodule
 *  function below to your real module name. The macro values are read
 *  by GMT's build-time generator (gen_gmt_moduleinfo_h in
 *  cmake/modules/GmtGenExtraHeaders.cmake) to populate the supplement's
 *  module table — they ARE the module's identity at runtime, not just
 *  documentation.
 *
 *--------------------------------------------------------------------*/

#define THIS_MODULE_CLASSIC_NAME    "mymodule"
#define THIS_MODULE_MODERN_NAME     "mymodule"
#define THIS_MODULE_LIB             "mymodules"   /* must match DLL basename */
#define THIS_MODULE_PURPOSE         "One-line description of what this module does"
#define THIS_MODULE_KEYS            ""            /* see gmt_resources.h for the key syntax */
#define THIS_MODULE_NEEDS           ""            /* "Jg" if module needs a projection, etc. */
#define THIS_MODULE_OPTIONS         "-Vh"         /* common options accepted via GMT_Parse_Common */

#include "gmt_dev.h"

/* Per-module option-parsing control struct. Replace with the real
 * fields you need. */
struct MYMODULE_CTRL {
    struct MYMODULE_A {  /* -A */
        bool active;
    } A;
};

static void *New_Ctrl(struct GMT_CTRL *GMT) {
    struct MYMODULE_CTRL *C = gmt_M_memory(GMT, NULL, 1, struct MYMODULE_CTRL);
    return C;
}

static void Free_Ctrl(struct GMT_CTRL *GMT, struct MYMODULE_CTRL *C) {
    if (!C) return;
    gmt_M_free(GMT, C);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
    const char *name = gmt_show_name_and_purpose(API, THIS_MODULE_LIB,
                                                 THIS_MODULE_MODERN_NAME,
                                                 THIS_MODULE_PURPOSE);
    (void)name;
    if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;

    GMT_Usage(API, 0, "usage: %s [-A] [%s] [%s]\n", THIS_MODULE_MODERN_NAME,
              GMT_V_OPT, GMT_PAR_OPT);
    if (level == GMT_SYNOPSIS) return GMT_MODULE_SYNOPSIS;

    GMT_Usage(API, 1, "\n-A Example flag.");
    GMT_Option(API, "V,.");
    return GMT_MODULE_USAGE;
}

static int parse(struct GMT_CTRL *GMT, struct MYMODULE_CTRL *Ctrl,
                 struct GMT_OPTION *options) {
    unsigned int n_errors = 0;
    for (struct GMT_OPTION *opt = options; opt; opt = opt->next) {
        switch (opt->option) {
            case 'A':
                Ctrl->A.active = true;
                break;
            default:
                n_errors += gmt_default_option_error(GMT, opt);
                break;
        }
    }
    return n_errors ? GMT_PARSE_ERROR : GMT_NOERROR;
}

EXTERN_MSC int GMT_mymodule(void *V_API, int mode, void *args) {
    struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API);
    struct GMT_OPTION *options = NULL;
    struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
    struct MYMODULE_CTRL *Ctrl = NULL;
    int error = 0;

    if (API == NULL) return GMT_NOT_A_SESSION;
    if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
    options = GMT_Create_Options(API, mode, args);
    if (API->error) return API->error;

    if (!options || options->option == GMT_OPT_USAGE)
        return usage(API, GMT_USAGE);
    if (options->option == GMT_OPT_SYNOPSIS)
        return usage(API, GMT_SYNOPSIS);

    GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME,
                          THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL,
                          &options, &GMT_cpy);

    Ctrl = New_Ctrl(GMT);
    if ((error = parse(GMT, Ctrl, options)) != 0) goto cleanup;

    /* ---- module body goes here ---- */

cleanup:
    Free_Ctrl(GMT, Ctrl);
    gmt_end_module(GMT, GMT_cpy);
    return error ? error : GMT_NOERROR;
}
