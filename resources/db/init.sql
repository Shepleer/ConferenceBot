CREATE TABLE registrations (
    id INTEGER PRIMARY KEY,

    chat_id INTEGER NOT NULL,

    state INTEGER NOT NULL,

    message_id INTEGER NOT NULL DEFAULT 0,

    name TEXT NOT NULL DEFAULT '',

    company TEXT NOT NULL DEFAULT '',

    company_position TEXT NOT NULL DEFAULT '',

    phrase TEXT NOT NULL DEFAULT '',

    telegram_nickname TEXT NOT NULL DEFAULT ''
);