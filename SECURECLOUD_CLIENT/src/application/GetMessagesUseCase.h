// Application Layer: GetMessagesUseCase.h


namespace application {

class GetMessagesUseCase {

private:

    IMessageRepository& m_repo;
    MessageCache& m_cache;

public:
    std::vector<Message> execute(const ConversationId& convId) {
        // 1. Check the cache (Hot Stack)
        if (m_cache.has(convId)) {
            return m_cache.get(convId); // Fast!
        }

        // 2. Not in cache, load from SQLite (via Repository)
        // The repo guarantees order by createdAt ASC.
        auto messages = m_repo.findLastMessages(convId, 50);

        // 3. Store in cache for next time
        m_cache.store(convId, messages);

        return messages;
    }
};

}