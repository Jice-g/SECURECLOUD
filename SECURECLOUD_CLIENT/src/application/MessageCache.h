// Application Layer: MessageCache.h

namespace application { 

class MessageCache {

private:

    // Maps a ConversationId to the list of messages currently in memory
    std::unordered_map<ConversationId, std::vector<Message>> m_cache;

public:
    bool has(const ConversationId& id) const {
        return m_cache.find(id) != m_cache.end();
    }

    const std::vector<Message>& get(const ConversationId& id) const {
        return m_cache.at(id);
    }

    void store(const ConversationId& id, std::vector<Message> messages) {
        m_cache[id] = std::move(messages);
    }
};

}