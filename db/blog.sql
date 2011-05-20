CREATE TABLE [article] (
[art_id] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,
[title] NVARCHAR(256)  NOT NULL,
[keyword] NVARCHAR(128)  NOT NULL,
[filename] VARCHAR(128)  NOT NULL,
[content] NTEXT  NOT NULL,
[catid] VARCHAR(30)  NOT NULL,
[hit] INTEGER DEFAULT '0' NOT NULL,
[position] INTEGER DEFAULT '0' NOT NULL,
[post_time] INTEGER DEFAULT '0' NOT NULL,
[recommend] INTEGER DEFAULT '0' NOT NULL,
[comment_num] INTEGER  NOT NULL
);
CREATE TABLE [category] (
[sid] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,
[sortname] VARCHAR(30)  NOT NULL,
[sortdir] VARCHAR(30)  NOT NULL,
[pos] INTEGER DEFAULT '0' NOT NULL
);
CREATE TABLE [comment] (
[id] INTEGER  PRIMARY KEY AUTOINCREMENT NOT NULL,
[article_id] INTEGER DEFAULT '0' NOT NULL,
[author] VARCHAR(30)  NOT NULL,
[addr] INTEGER DEFAULT '0' NOT NULL,
[post_time] INTEGER DEFAULT '0' NOT NULL,
[content] NTEXT  NOT NULL
);
CREATE TABLE [friendlink] (
[id] INTEGER  NOT NULL PRIMARY KEY AUTOINCREMENT,
[site] NVARCHAR(100)  NOT NULL,
[url] VARCHAR(30)  NOT NULL,
[post_time] INTEGER  NOT NULL,
[pos] INTEGER DEFAULT '0' NOT NULL
);
CREATE INDEX [IDX_ARTICLE_CATID] ON [article](
[catid]  ASC
);
CREATE INDEX [IDX_ARTICLE_RECOMMEND] ON [article](
[recommend]  ASC
);
CREATE INDEX [IDX_CATEGORY_POS] ON [category](
[pos]  ASC
);
CREATE INDEX [IDX_CONTENT_ARTICLE_ID] ON [comment](
[article_id]  DESC
);
CREATE INDEX [IDX_FRIENDLINK_POS] ON [friendlink](
[pos]  ASC
);

INSERT INTO category(sortname, sortdir, pos) VALUES('php', 'php', 1);
INSERT INTO category(sortname, sortdir, pos) VALUES('mysql', 'mysql', 2);
INSERT INTO category(sortname, sortdir, pos) VALUES('nosql', 'nosql', 3);
INSERT INTO category(sortname, sortdir, pos) VALUES('linux', 'linux', 4);
INSERT INTO category(sortname, sortdir, pos) VALUES('javascript', 'javascript', 5);
INSERT INTO category(sortname, sortdir, pos) VALUES('生活杂事', 'life', 6);
INSERT INTO category(sortname, sortdir, pos) VALUES('我的开源项目', 'opensource', 7);
