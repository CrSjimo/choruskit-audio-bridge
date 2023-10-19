#include "SingleInstanceGuard.h"

#include <boost/interprocess/sync/named_mutex.hpp>

using namespace boost::interprocess;

SingleInstanceGuard::SingleInstanceGuard(juce::StringRef key) : m_named_mutex(new named_mutex(open_or_create, key)) {
    if (m_named_mutex->try_lock())
        m_isPrimary = true;
}

SingleInstanceGuard::~SingleInstanceGuard() {
    if (m_isPrimary) {
        m_named_mutex->unlock();
    }
}

bool SingleInstanceGuard::isPrimary() {
    return m_isPrimary;
}
