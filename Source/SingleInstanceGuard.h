#ifndef CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H
#define CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H

#include <juce_core/juce_core.h>

namespace boost::interprocess {
    class file_lock;
}

class SingleInstanceGuard {
public:
    explicit SingleInstanceGuard(juce::StringRef key);
    ~SingleInstanceGuard();

    bool isPrimary() const;
private:
    std::unique_ptr<boost::interprocess::file_lock> m_lock;
    bool m_isPrimary = false;
};


#endif //CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H
