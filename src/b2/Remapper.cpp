#include <shared/system.h>
#include "Remapper.h"
#include <shared/log.h>
#include <inttypes.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Remapper::Remapper()
    : Remapper(1, 1) {
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Remapper::Remapper(uint64_t num_steps, uint64_t num_items)
    : m_num_steps(num_steps)
    , m_num_items(num_items)
    , m_error(0) {
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint64_t Remapper::Step() {
    m_error += m_num_items;
    uint64_t n = m_error / m_num_steps;
    m_error %= m_num_steps;
    return n;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint64_t Remapper::GetNumUnits(uint64_t steps) const {
    uint64_t error = m_error + steps * m_num_items;
    uint64_t n = error / m_num_steps;
    return n;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
