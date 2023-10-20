#include "SingleInstanceGuard.h"

#include <juce_core/juce_core.h>

#include <boost/interprocess/sync/file_lock.hpp>


using namespace boost::interprocess;

SingleInstanceGuard::SingleInstanceGuard(juce::StringRef key) {

    juce::File f(key);
    f.create();
    m_lock = std::make_unique<file_lock>(key);
    if (m_lock->try_lock())
        m_isPrimary = true;
}

SingleInstanceGuard::~SingleInstanceGuard() {
    if (m_isPrimary) {
        m_lock->unlock();
    }
}

bool SingleInstanceGuard::isPrimary() const {
    return m_isPrimary;
}
