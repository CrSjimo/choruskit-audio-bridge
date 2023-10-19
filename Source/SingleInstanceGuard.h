#ifndef CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H
#define CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H

#include <juce_core/juce_core.h>

namespace boost::interprocess {
    class named_mutex;
}

class SingleInstanceGuard {
public:
    explicit SingleInstanceGuard(juce::StringRef key);
    ~SingleInstanceGuard();

    bool isPrimary();
private:
    std::unique_ptr<boost::interprocess::named_mutex> m_named_mutex;
    bool m_isPrimary = false;
};


#endif //CHORUSKITAUDIOBRIDGE_SINGLEINSTANCEGUARD_H
