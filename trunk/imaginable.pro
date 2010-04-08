TEMPLATE = subdirs
SUBDIRS = \
	core \
	plugins

fullclean.depends = FORCE
fullclean.commands += "rm -rf bin ; find . -name '*.pro.user' -exec rm '{}' \;"
distclean.depends = fullclean

QMAKE_EXTRA_TARGETS += fullclean distclean
