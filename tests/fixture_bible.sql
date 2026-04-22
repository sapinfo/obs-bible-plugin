CREATE TABLE bible_books (id INTEGER PRIMARY KEY, name TEXT, abbr TEXT, eng TEXT, testament TEXT);
CREATE TABLE bible (id INTEGER PRIMARY KEY, verse_ref TEXT, content TEXT, book TEXT, chapter INTEGER, verse INTEGER);

INSERT INTO bible_books VALUES (1, '창세기',   '창',   'Genesis', 'OT');
INSERT INTO bible_books VALUES (42,'누가복음', '눅',   'Luke',    'NT');
INSERT INTO bible_books VALUES (43,'요한복음', '요',   'John',    'NT');
INSERT INTO bible_books VALUES (62,'요한일서', '요일', '1 John',  'NT');

INSERT INTO bible VALUES (1,  '창1:1',  '태초에 하나님이 천지를 창조하시니라', '창', 1, 1);
INSERT INTO bible VALUES (2,  '창1:2',  '땅이 혼돈하고 공허하며',              '창', 1, 2);
INSERT INTO bible VALUES (3,  '눅1:1',  '우리 중에 이루어진 사실에 대하여',    '눅', 1, 1);
INSERT INTO bible VALUES (4,  '눅1:10', '모든 백성은 그 분향하는 시간에',      '눅', 1, 10);
INSERT INTO bible VALUES (5,  '눅10:1', '이 후에 주께서 따로 칠십 인을',       '눅', 10, 1);
INSERT INTO bible VALUES (6,  '요3:16', '하나님이 세상을 이처럼 사랑하사',     '요', 3, 16);
INSERT INTO bible VALUES (7,  '요일1:1','태초부터 있는 생명의 말씀에 관하여',  '요일', 1, 1);
