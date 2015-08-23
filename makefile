MAKEFLAGS = ks

srcdir = src
srv = ${srcdir}/main.py
	python ${srv}


.PHONY: srv
