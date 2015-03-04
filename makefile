MAKEFLAGS = ks

srv:
	csi srv/main.scm -q

.PHONY: srv
