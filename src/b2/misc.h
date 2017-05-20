#ifndef HEADER_36EF7AB4014F402AB918752A470F8048
#define HEADER_36EF7AB4014F402AB918752A470F8048

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct tm;
struct SDL_RendererInfo;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_PixelFormat;
class BBCMicro;
struct ROM;

#include <vector>
#include <string>
#include <functional>
#include <shared/log.h>
#include <memory>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DumpRendererInfo(Log *log,const struct SDL_RendererInfo *info);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SetRenderScaleQualityHint(bool filter);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string strprintf(const char *fmt,...) PRINTF_LIKE(1,2);
std::string strprintfv(const char *fmt,va_list v);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string GetFlagsString(uint32_t value,const char *(*get_name_fn)(int));

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string GetMicrosecondsString(uint64_t num_microseconds);
std::string Get2MHzCyclesString(uint64_t num_2MHz_cycles);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Should really use this consistently. There's very few places in the
// C++ code where the minor cost will be a problem. A legacy of the
// project's C legacy...

struct SDL_Deleter {
    void operator()(SDL_Window *w) const;
    void operator()(SDL_Renderer *r) const;
    void operator()(SDL_Texture *t) const;
    void operator()(SDL_Surface *s) const;
    void operator()(SDL_PixelFormat *p) const;
};

template<class T>
using SDLUniquePtr=std::unique_ptr<T,SDL_Deleter>;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

SDL_PixelFormat *ClonePixelFormat(const SDL_PixelFormat *pixel_format);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string GetUniqueName(std::string suggested_name,
                          std::function<const void *(const std::string &)> find,
                          const void *ignore);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct tm GetUTCTimeNow();
struct tm GetLocalTimeNow();
std::string GetTimeString(const struct tm &t);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template<class T>
static T ValueChanged(T *value_ptr,T &&new_value) {
    T change=*value_ptr^new_value;

    *value_ptr=new_value;
    
    return change;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class AudioDeviceLock {
public:
    explicit AudioDeviceLock(uint32_t device);
    ~AudioDeviceLock();

    AudioDeviceLock(AudioDeviceLock &&)=delete;
    AudioDeviceLock &operator=(AudioDeviceLock &&)=delete;

    AudioDeviceLock(const AudioDeviceLock &)=delete;
    AudioDeviceLock &operator=(const AudioDeviceLock &)=delete;
protected:
private:
    uint32_t m_device;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
