const REDIS_KEYS = {
    session: (uuid) => `sess:${uuid}`,
    
    userSession: (userId) => `user_sess:${userId}`,
    
    matchTicket: (ticketId) => `ticket:${ticketId}`,
};

module.exports = REDIS_KEYS;