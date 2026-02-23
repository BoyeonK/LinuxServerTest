function makeResponse(success, code, data = null, error = null) {
    return { success, code, data, error };
}

module.exports = { makeResponse };