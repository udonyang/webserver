-- hearthstone

drop table if exists hearthstone;
create table if not exists hearthstone(
  id int(32) unsigned primary key auto_increment,
  player varchar(32) not null,
  player_deck varchar(32),
  opponent_deck varchar(32),
  is_win int(1) default 1,
  time timestamp default CURRENT_TIMESTAMP
) default charset utf8, engine myisam;
