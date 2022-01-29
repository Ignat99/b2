#ifndef HEADER_8CB0B7841DB5498CBBE2C2B077C9DB85
#define HEADER_8CB0B7841DB5498CBBE2C2B077C9DB85
#include "cpp_begin.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef ASSERT_ENABLED
#define ASSERT_ENABLED 0
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#if ASSERT_ENABLED

void LogAssertFailed(const char *file, int line, const char *function, const char *expr);
void PRINTF_LIKE(1, 2) LogAssertElaboration(const char *fmt, ...);
void HandleAssertFailed(void);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ASSERT(EXPR)                                              \
    BEGIN_MACRO {                                                 \
        if (!(EXPR)) {                                            \
            LogAssertFailed(__FILE__, __LINE__, __func__, #EXPR); \
            if (IsDebuggerAttached()) {                           \
                BREAK();                                          \
            }                                                     \
            HandleAssertFailed();                                 \
        }                                                         \
    }                                                             \
    END_MACRO

#define ASSERTF(EXPR, ...)                                        \
    BEGIN_MACRO {                                                 \
        if (!(EXPR)) {                                            \
            LogAssertFailed(__FILE__, __LINE__, __func__, #EXPR); \
            LogAssertElaboration(__VA_ARGS__);                    \
            if (IsDebuggerAttached()) {                           \
                BREAK();                                          \
            }                                                     \
            HandleAssertFailed();                                 \
        }                                                         \
    }                                                             \
    END_MACRO

#else

#define ASSERT(EXPR) ((void)0)
#define ASSERTF(...) ((void)0)

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "cpp_end.h"
#endif //HEADER_8CB0B7841DB5498CBBE2C2B077C9DB85
