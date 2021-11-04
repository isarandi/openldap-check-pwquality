# OpenLDAP-check-pwquality

This is a small project to make [libpwquality](https://github.com/libpwquality/libpwquality) work as an OpenLDAP password policy module (`pwdCheckModule`).

The following installation instructions are for Ubuntu 18.04 LTS, but can be trivially adjusted to different distros.

Install some dependencies (assuming that OpenLDAP/slapd is already installed):

```bash
sudo apt install libpwquality-dev cracklib-runtime libdb-dev 
```

We need the OpenLDAP sources to compile our module. You can download the sources from the OpenLDAP website or use your package manager. On Ubuntu, enable the source packages in `/etc/apt/sources.list`, with e.g.

```bash
echo 'deb-src http://de.archive.ubuntu.com/ubuntu/ bionic main restricted' | sudo tee -a /etc/apt/sources.list
```

Get the source package and `make` it. This requires the libdbd-dev (BerkeleyDB) package as well.

```bash
cd /tmp
apt-get source slapd
cd openldap-2.4.45+dfsg/
./configure
make depend
```

Now clone this repo and `make` the library as follows:

```bash
cd /tmp
git clone https://github.com/isarandi/openldap-check-pwquality.git
cd openldap-check-pwquality
sudo make install LDAP_SRC=/tmp/openldap-2.4.45+dfsg CONFIG_PATH=/etc/ldap/pwquality.conf LDAP_LIBDIR=/usr/lib/ldap
```

At this point there should be a file `check_pwquality.so` under `/usr/lib/ldap` and you can simply add the attribute `pwdCheckModule: check_pwquality.so` to your LDAP password policy. How exactly this is done will depend on your particular LDAP layout. See [this blog post on Kifarunix](https://kifarunix.com/implement-openldap-password-policies/) for some guidance.

Now, all that remains is to configure pwquality at the chosen config file path (`/etc/ldap/pwquality.conf` by default). See `man pwquality.conf` for details on this. Since OpenLDAP is often run in a way that doesn't let it read files from just anywhere, make sure that your cracklib dictionary is readable to the LDAP server. In particular, it's best to place both the pwquality.conf file and the cracklib dictionary files under `/etc/ldap/` as well.

You can set up a beefy cracklib dictionary as follows:

```bash
cd /tmp
git clone https://github.com/cracklib/cracklib.git
cd cracklib/words
make
sudo cp cracklib-words /etc/cracklib/
sudo sed -i 's/cracklib_dictpath_src=""/cracklib_dictpath_src="\/etc\/cracklib\/cracklib-words"/' /etc/cracklib/cracklib.conf
sudo update-cracklib
sudo cp /var/cache/cracklib/cracklib_dict* /etc/ldap/
echo 'dictpath = /etc/ldap/cracklib_dict' | sudo tee -a /etc/ldap/pwquality.conf
```

And that's it!