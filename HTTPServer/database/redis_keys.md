# Redis Key Structures

| Key Pattern | Type | TTL (수명) | Description | Example Value / Fields |
| :--- | :--- | :--- | :--- | :--- |
| `item_meta` | **Hash** | 영구(None) | 아이템 마스터 정보 | `item_name: "돌", item_type: "resource", description: "그냥 돌"` (이름, 분류, 설명) |
| `sess:<UUID>` | **String** | 1시간 (3600s) | 클라이언트 인증용 세션 | `"tetepiti149"` (유저 ID) |
| `user_sess:<ID>` | **String** | 1시간 (3600s) | 중복 로그인 방지용 | `"sess_1234abcd..."` (세션 UUID) |
