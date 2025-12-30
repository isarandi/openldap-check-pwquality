CC=gcc

# Where to find the OpenLDAP sources
LDAP_SRC=/tmp/openldap-2.6.10
LDAP_LIBDIR=/usr/lib/ldap
CONFIG_PATH=/etc/ldap/pwquality.conf
CC_FLAGS=-g -O2 -Wall -fpic

all: 	check_pwquality.so

check_pwquality.o: check_pwquality.c
	$(CC) $(CC_FLAGS) -c -I $(LDAP_SRC)/include -I $(LDAP_SRC)/servers/slapd -DCONFIG_PATH="\"$(CONFIG_PATH)\"" $<

check_pwquality.so: check_pwquality.o
	$(CC) -shared -o $@ $< -lpwquality

install: check_pwquality.so
	cp -f $< $(LDAP_LIBDIR)/

clean:
	$(RM) check_pwquality.o check_pwquality.so
