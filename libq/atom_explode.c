/*
 * Copyright 2005-2008 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /var/cvsroot/gentoo-projects/portage-utils/libq/atom_explode.c,v 1.23 2008/01/16 07:09:07 vapier Exp $
 *
 * Copyright 2005-2008 Ned Ludd        - <solar@gentoo.org>
 * Copyright 2005-2008 Mike Frysinger  - <vapier@gentoo.org>
 */

typedef enum { VER_ALPHA=0, VER_BETA, VER_PRE, VER_RC, VER_NORM, VER_P } atom_suffixes;
const char * const atom_suffixes_str[] = { "_alpha", "_beta", "_pre", "_rc", "_/*bogus*/", "_p" };

typedef struct {
	atom_suffixes suffix;
	unsigned int sint;
} atom_suffix;

typedef struct {
	/* XXX: we don't provide PF ... */
	char *CATEGORY;
	char *PN;
	unsigned int PR_int;
	char letter;
	atom_suffix *suffixes;
	char *PV, *PVR;
	char *P, *SLOT;
} depend_atom;

#ifdef _USE_CACHE
static depend_atom *_atom_cache = NULL;
static size_t _atom_cache_len = 0;
#endif

depend_atom *atom_explode(const char *atom);
depend_atom *atom_explode(const char *atom)
{
	depend_atom *ret;
	char *ptr;
	size_t len, slen;
	int i;

	/* we allocate mem for atom struct and two strings (strlen(atom)).
	 * the first string is for CAT/PN/PV while the second is for PVR.
	 * PVR needs 3 extra bytes for possible implicit '-r0'. */
	slen = strlen(atom);
	len = sizeof(*ret) + (slen + 1) * sizeof(*atom) * 3 + 3;
#ifdef _USE_CACHE
	if (len <= _atom_cache_len) {
		ret = _atom_cache;
		memset(ret, 0x00, len);
	} else {
		free(_atom_cache);
		_atom_cache = ret = xzalloc(len);
		_atom_cache_len = len;
	}
#else
	ret = xzalloc(len);
#endif
	ptr = (char*)ret;
	ret->P = ptr + sizeof(*ret);
	ret->PVR = ret->P + slen + 1;
	ret->CATEGORY = ret->PVR + slen + 1 + 3;
	strcpy(ret->CATEGORY, atom);

	/* eat file name crap */
	if ((ptr = strstr(ret->CATEGORY, ".ebuild")) != NULL)
		*ptr = '\0';

	/* chip off the trailing [:SLOT] as needed */
	if ((ptr = strrchr(ret->CATEGORY, ':')) != NULL) {
		ret->SLOT = ptr + 1;
		*ptr = '\0';
	}

	/* break up the CATEOGRY and PVR */
	if ((ptr = strrchr(ret->CATEGORY, '/')) != NULL) {
		ret->PN = ptr + 1;
		*ptr = '\0';
		/* eat extra crap in case it exists */
		if ((ptr = strrchr(ret->CATEGORY, '/')) != NULL)
			ret->CATEGORY = ptr + 1;
	} else {
		ret->PN = ret->CATEGORY;
		ret->CATEGORY = NULL;
	}
	strcpy(ret->PVR, ret->PN);

	/* find -r# */
	ptr = ret->PN + strlen(ret->PN) - 1;
	while (*ptr && ptr > ret->PN) {
		if (!isdigit(*ptr)) {
			if (ptr[0] == 'r' && ptr[-1] == '-') {
				ret->PR_int = atoi(ptr + 1);
				ptr[-1] = '\0';
			} else
				strcat(ret->PVR, "-r0");
			break;
		}
		--ptr;
	}
	strcpy(ret->P, ret->PN);

	/* break out all the suffixes */
	int sidx = 0;
	ret->suffixes = xrealloc(ret->suffixes, sizeof(atom_suffix) * (sidx + 1));
	ret->suffixes[sidx].sint = 0;
	ret->suffixes[sidx].suffix = VER_NORM;
	while ((ptr = strrchr(ret->PN, '_')) != NULL) {
		for (i = 0; i < ARRAY_SIZE(atom_suffixes_str); ++i) {
			if (strncmp(ptr, atom_suffixes_str[i], strlen(atom_suffixes_str[i])))
				continue;

			/* check this is a real suffix and not _p hitting mod_perl */
			char *tmp_ptr = ptr;
			tmp_ptr += strlen(atom_suffixes_str[i]);
			ret->suffixes[sidx].sint = atoi(tmp_ptr);
			while (isdigit(*tmp_ptr))
				++tmp_ptr;
			if (*tmp_ptr)
				goto no_more_suffixes;
			ret->suffixes[sidx].suffix = i;

			++sidx;
			*ptr = '\0';

			ret->suffixes = xrealloc(ret->suffixes, sizeof(atom_suffix) * (sidx + 1));
			ret->suffixes[sidx].sint = 0;
			ret->suffixes[sidx].suffix = VER_NORM;
			break;
		}
		if (*ptr)
			break;
	}
 no_more_suffixes:
	--sidx;
	for (i = 0; i < sidx; ++i, --sidx) {
		atom_suffix t = ret->suffixes[sidx];
		ret->suffixes[sidx] = ret->suffixes[i];
		ret->suffixes[i] = t;
	}

	/* allow for 1 optional suffix letter */
	ptr = ret->PN + strlen(ret->PN);
	if (ptr[-1] >= 'a' && ptr[-1] <= 'z') {
		ret->letter = ptr[-1];
		--ptr;
	}

	/* eat the trailing version number [-.0-9]+ */
	bool has_pv = false;
	while (--ptr > ret->PN)
		if (*ptr == '-') {
			if (has_pv)
				*ptr = '\0';
			break;
		} else if (*ptr != '.' && !isdigit(*ptr))
			break;
		else
			has_pv = true;
	if (has_pv) {
		ret->PV = ret->P + (ptr - ret->PN) + 1;
	} else {
		/* atom has no version */
		ret->PV = ret->PVR = NULL;
		ret->letter = 0;
	}

	return ret;
}

void atom_implode(depend_atom *atom);
void atom_implode(depend_atom *atom)
{
	if (!atom)
		errf("Atom is empty !");
	free(atom->suffixes);
#ifndef _USE_CACHE
	free(atom);
#endif
}
