.mode tabs
.output ./sites/all/modules/pickem/data/accproj.tsv
select gid, week, visiting_school_name, host_school_name from game where (visiting_conf_name = 'acc' or host_conf_name = 'acc') and completed = 'N' and week != 15 and gid not in (15, 31, 39);

.output ./sites/all/modules/pickem/data/accrecords.tsv
select sr.*, s.conf_division from school_records sr join school s on (sr.sid = s.sid and conf_name = 'acc') where tag in ('overall', 'conf', 'div');

.output stdout
