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
struct _SDL_Joystick;
struct _SDL_GameController;
class BBCMicro;
struct ROM;
class Messages;

#include "conf.h"
#include <vector>
#include <string>
#include <functional>
#include <shared/log.h>
#include <memory>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DumpRendererInfo(Log *log, const struct SDL_RendererInfo *info);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SetRenderScaleQualityHint(bool filter);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string strprintf(const char *fmt, ...) PRINTF_LIKE(1, 2);
std::string strprintfv(const char *fmt, va_list v);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string GetFlagsString(uint32_t value, const char *(*get_name_fn)(int));

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string GetCloneImpedimentsDescription(uint32_t impediments);
//std::string GetMicrosecondsString(uint64_t num_microseconds);
std::string GetCycleCountString(CycleCount cycle_count);

// 0         1         2
// 012345687901234567890123456
// 18,446,744,073,709,551,616

static const size_t MAX_UINT64_THOUSANDS_LEN = 26;
void GetThousandsString(char *str, uint64_t value);

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
    void operator()(_SDL_Joystick *p) const;
    void operator()(_SDL_GameController *p) const;
};

template <class T>
using SDLUniquePtr = std::unique_ptr<T, SDL_Deleter>;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SDL_SurfaceLocker {
  public:
    explicit SDL_SurfaceLocker(SDL_Surface *surface);
    ~SDL_SurfaceLocker();

    SDL_SurfaceLocker(const SDL_SurfaceLocker &) = delete;
    SDL_SurfaceLocker &operator=(const SDL_SurfaceLocker &) = delete;
    SDL_SurfaceLocker(const SDL_SurfaceLocker &&) = delete;
    SDL_SurfaceLocker &operator=(SDL_SurfaceLocker &&) = delete;

    bool IsLocked() const;
    void Unlock();

  protected:
  private:
    SDL_Surface *m_surface = nullptr;
};

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

template <class T>
static T ValueChanged(T *value_ptr, T &&new_value) {
    T change = *value_ptr ^ new_value;

    *value_ptr = new_value;

    return change;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class AudioDeviceLock {
  public:
    explicit AudioDeviceLock(uint32_t device);
    ~AudioDeviceLock();

    AudioDeviceLock(AudioDeviceLock &&) = delete;
    AudioDeviceLock &operator=(AudioDeviceLock &&) = delete;

    AudioDeviceLock(const AudioDeviceLock &) = delete;
    AudioDeviceLock &operator=(const AudioDeviceLock &) = delete;

  protected:
  private:
    uint32_t m_device;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ForEachLine(const std::string &str, std::function<void(const std::string::const_iterator &a, const std::string::const_iterator &b)> fun);

std::vector<std::string> GetSplitString(const std::string &str, const std::string &separator_chars);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool GetBoolFromString(bool *value, const std::string &str);
bool GetBoolFromString(bool *value, const char *str);

bool GetUInt8FromString(uint8_t *value, const std::string &str, int radix = 0);
bool GetUInt8FromString(uint8_t *value, const char *str, int radix = 0);

bool GetUInt16FromString(uint16_t *value, const std::string &str, int radix = 0);
bool GetUInt16FromString(uint16_t *value, const char *str, int radix = 0);

bool GetUInt32FromString(uint32_t *value, const std::string &str, int radix = 0);
bool GetUInt32FromString(uint32_t *value, const char *str, int radix = 0);

bool GetUInt64FromString(uint64_t *value, const std::string &str, int radix = 0);
bool GetUInt64FromString(uint64_t *value, const char *str, int radix = 0);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// returns true on success, or false on failure.
bool GetBBCASCIIFromUTF8(std::string *ascii,
                         const std::vector<uint8_t> &data,
                         uint32_t *bad_codepoint_ptr,
                         const uint8_t **bad_char_start_ptr,
                         int *bad_char_len_ptr);

// returns 0 on success, or the unsupported codepoint on failure.q
uint32_t GetBBCASCIIFromISO88511(std::string *ascii,
                                 const std::vector<uint8_t> &data);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Should this be part of GetBBCASCIIFromXXX???
void FixBBCASCIINewlines(std::string *str);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Exapnds to a string literal: mu, then "s", UTF8-encoded
#define MICROSECONDS_UTF8 "\xc2\xb5s"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
