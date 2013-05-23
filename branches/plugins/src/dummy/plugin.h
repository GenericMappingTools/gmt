#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

EXTERN_MSC int GMT_dummy1 (void *api_ctrl, int mode, void *args);
EXTERN_MSC int GMT_dummy2 (void *api_ctrl, int mode, void *args);

#ifdef __cplusplus /* Basic C++ support */
}
#endif
