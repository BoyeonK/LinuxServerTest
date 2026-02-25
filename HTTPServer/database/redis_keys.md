# Redis Key Structures

| Key Pattern | Type | TTL (수명) | Description | Example Value / Fields |
| :--- | :--- | :--- | :--- | :--- |
| `sess:<UUID>` | **String** | 1시간 (3600s) | 클라이언트 인증용 세션 | `"tetepiti149"` (유저 ID) |
| `user_sess:<ID>` | **String** | 1시간 (3600s) | 중복 로그인 방지용 | `"sess_1234abcd..."` (세션 UUID) |