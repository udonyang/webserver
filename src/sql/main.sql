-- hearthstone

create table if not exists hearthstone_deck(
  name varchar(32) not null,
  charactor varchar(32) not null,
  primary key (name, charactor)
) default charset utf8, engine myisam;

create table if not exists hearthstone_vs(
  self_deck_name varchar(32),
  opponent_deck_name varchar(32),
  is_win int(32) default 0,
  vstime timestamp default CURRENT_TIMESTAMP,
  primary key (self_deck_name, opponent_deck_name)
) default charset utf8, engine myisam;
