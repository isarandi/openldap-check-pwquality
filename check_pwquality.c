/*
    check_pwquality - password policy module for OpenLDAP using libpwquality

    Copyright (C) 2021 Istvan Sarandi <istvan.sarandi@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <string.h>
#include <portable.h>
#include <slap.h>
#include <pwquality.h>
#include <syslog.h>

int check_password(const char *pPasswd, char **ppErrStr, const Entry *pEntry)
{
    // Get pwquality settings
    pwquality_settings_t *pwq;
    int rv;
    void *auxerror;
    char buf[PWQ_MAX_ERROR_MESSAGE_LEN];
    pwq = pwquality_default_settings();
    if (!pwq) {
        syslog(LOG_ERR, "Error in check_pwquality: %s\n", pwquality_strerror(buf, sizeof(buf), PWQ_ERROR_MEM_ALLOC, NULL));
        return EXIT_FAILURE;
    }
    if ((rv=pwquality_read_config(pwq, CONFIG_PATH, &auxerror)) != 0) {
        pwquality_free_settings(pwq);
        syslog(LOG_ERR, "Error in check_pwquality: %s\n", pwquality_strerror(buf, sizeof(buf), rv, auxerror));
        return EXIT_FAILURE;
    }

    // Get username between equals sign and comma
    if (!pEntry)
        return EXIT_FAILURE;
    char *equals_ptr = strchr(pEntry->e_name.bv_val, '=');
    if (!equals_ptr)
        return EXIT_FAILURE;
    char *comma_ptr = strchr(equals_ptr + 1, ',');
    if (!comma_ptr)
        return EXIT_FAILURE;
    size_t namelen = comma_ptr - equals_ptr - 1;
    char *username = malloc(namelen + 1);
    strncpy(username, equals_ptr + 1, namelen);
    username[namelen] = '\0';

    // Check quality
    rv = pwquality_check(pwq, pPasswd, NULL, username, &auxerror);
    pwquality_free_settings(pwq);
    free(username);

    if (rv >= 0) {
        return LDAP_SUCCESS;
    } else {
        const char *prefix = "The password is too weak: ";
        size_t prefix_len = strlen(prefix);
        const char *errstr = pwquality_strerror(buf, sizeof(buf), rv, auxerror);
        *ppErrStr = malloc(prefix_len + PWQ_MAX_ERROR_MESSAGE_LEN);
        strncpy(*ppErrStr, prefix, prefix_len);
        strncpy((*ppErrStr) + prefix_len, errstr, PWQ_MAX_ERROR_MESSAGE_LEN);
        return EXIT_FAILURE;
    }
}
