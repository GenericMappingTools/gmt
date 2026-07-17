/*
 * gmt_update.c — Standalone GMT binary updater.
 *
 * Fetches a small text manifest from a URL, compares SHA-256 of listed
 * files against the local copies, downloads any that differ, verifies
 * the download, and replaces the local file. Uses libcurl (already a
 * GMT dependency) and an embedded public-domain SHA-256 implementation.
 *
 * IMPORTANT: this executable must NOT link against libgmt or any GMT
 * DLL. If it did, those DLLs would be locked while the updater runs,
 * preventing replacement. Only libcurl is linked. On Windows the
 * Restart Manager API is used (rstrtmgr) to report which processes
 * hold a lock on a target file.
 *
 * Manifest format (plain text, one record per line):
 *
 *   # comment
 *   version: 6.7.0
 *   build:   2026-05-23T14:22:00Z
 *   channel: win_x64    (OS_ARCH tag: {win,linux,macos}_{x64,arm64})
 *   file <relpath> <size> <sha256hex> <url>
 *   ...
 *   end
 *
 * Exit codes:
 *   0  nothing to do, or all files updated
 *   1  fetch/parse error
 *   2  one or more files failed to update
 *
 * Usage:
 *   gmt_update                       (default URL, auto-derived install root)
 *   gmt_update <manifest_url>
 *   gmt_update <manifest_url> --dry-run
 *
 * Inspect mode (no downloads, prints local vs remote diff):
 *   gmt_update --show [<manifest_url>]
 *
 * Generator mode (build a manifest from the local install):
 *   gmt_update --make-manifest [-o <file>] [--files <list>]
 *                              [--version <str>] [--root <dir>]
 *
 *   The manifest does NOT carry download URLs. At update time the updater
 *   resolves each file as a sibling of the manifest URL, i.e.
 *      download_url = dirname(manifest_url) + "/" + basename(relpath)
 *   So you just upload the manifest + all binaries to the same location
 *   (same GitHub release, same web dir, etc.) — no URL coordination needed.
 *
 * Install root: derived from gmt_update.exe's own path (parent of bin/).
 * Channel:       compile-time constant {win,linux,macos}_{x64,arm64}.
 *
 * Latest-release discovery: default URL points at GitHub's REST API
 *   (api.github.com/repos/.../releases/latest). The updater parses the JSON,
 *   picks the asset named "manifest_<channel>.txt", and follows its
 *   browser_download_url. No /download/-style URL is guessed by the updater;
 *   the asset URL is whatever GitHub returns in the JSON.
 *
 * Version gate: spawns `<install_root>/bin/gmt --version`, parses
 *   MAJOR.MINOR.PATCH_<git>[-dirty]_YYYY.MM.DD, compares to manifest
 *   `version:` on (semver, date). Refuses to apply older manifest. Refuses
 *   on dirty developer build unless --allow-dirty.
 */

/* WARNING: This code is still very experimental and not meant for public usage. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include "sha256.h"

#ifdef _WIN32
#  include <windows.h>
#  include <restartmanager.h>
#  pragma comment(lib, "rstrtmgr.lib")
#  define PATH_SEP "\\"
#  define OS_TAG  "win"
#else
#  include <unistd.h>
#  define PATH_SEP "/"
#  ifdef __APPLE__
#    include <mach-o/dyld.h>
#    define OS_TAG "macos"
#  else
#    define OS_TAG "linux"
#  endif
#endif

/* Arch tag: x64 (== amd64 / x86_64) or arm64 (== aarch64). */
#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__)
#  define ARCH_TAG "x64"
#elif defined(_M_ARM64) || defined(__aarch64__)
#  define ARCH_TAG "arm64"
#else
#  error "Unsupported processor architecture for gmt_update channel"
#endif

#define DEFAULT_CHANNEL OS_TAG "_" ARCH_TAG

#define MAX_FILES   128
#define MAX_PATH_S  512
#define MAX_URL     1024
#define SHA_HEXLEN  64

/* GitHub's REST API endpoint for "latest release" of the GMT repo. The
 * response is a JSON object with an "assets" array; each asset has a
 * "browser_download_url" we can fetch. We use this two-step pattern instead
 * of guessing a static URL because GitHub does NOT expose a direct latest-
 * asset path at /releases/latest/<asset>. */
#ifndef GMT_UPDATE_REPO_API
#  define GMT_UPDATE_REPO_API \
	"https://api.github.com/repos/GenericMappingTools/gmt/releases/latest"
#endif

#ifndef GMT_UPDATE_DEFAULT_URL
#  define GMT_UPDATE_DEFAULT_URL GMT_UPDATE_REPO_API
#endif

struct entry {
	char path[MAX_PATH_S];
	long size;
	char sha[SHA_HEXLEN + 1];
	char url[MAX_URL];
	int  needs_update;
	int  regression;   /* local mtime > manifest build → replacing would overwrite */
	                   /* a developer's just-rebuilt copy with an older release. */
};

struct manifest {
	char version[64];
	char build[32];
	char channel[16];
	struct entry f[MAX_FILES];
	int n;
};

/* Parsed view of a "MAJOR.MINOR.PATCH_<hash>[-dirty]_YYYY.MM.DD" string. */
struct ver {
	int  semver[3];     /* {6,7,0}                                       */
	char hash[32];      /* "d3435c5" or "8b76b26-dirty" or ""            */
	int  date[3];       /* {2026, 5, 19}                                 */
	int  is_dirty;      /* 1 if hash carries "-dirty" suffix             */
	int  valid;         /* 1 if we managed to parse all three tokens     */
};

/* ---------------- curl helpers ---------------- */

struct mem { char *buf; size_t len; };

static size_t mem_cb(void *p, size_t s, size_t n, void *ud) {
	struct mem *m = (struct mem *)ud;
	size_t add = s * n;
	char *nb = (char *)realloc(m->buf, m->len + add + 1);
	if (!nb) return 0;
	m->buf = nb;
	memcpy(m->buf + m->len, p, add);
	m->len += add;
	m->buf[m->len] = 0;
	return add;
}

static int http_get_mem(const char *url, struct mem *out) {
	CURL *c = curl_easy_init();
	CURLcode rc;
	long http_code = 0;
	char errbuf[CURL_ERROR_SIZE];
	if (!c) return -1;
	errbuf[0] = 0;
	curl_easy_setopt(c, CURLOPT_URL, url);
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, mem_cb);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, out);
	curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(c, CURLOPT_MAXREDIRS, 10L);
	curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURLSSLOPT_NATIVE_CA
	/* Use Windows/macOS system cert store when libcurl supports it. Avoids
	 * shipping a CA bundle and the classic "could not fetch" on Windows. */
	curl_easy_setopt(c, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA);
#endif
	curl_easy_setopt(c, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(c, CURLOPT_USERAGENT, "gmt_update/1.0");
	curl_easy_setopt(c, CURLOPT_TIMEOUT, 120L);
	rc = curl_easy_perform(c);
	curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &http_code);
	if (rc != CURLE_OK) {
		fprintf(stderr, "  curl: %s (code=%d, http=%ld)\n",
		        errbuf[0] ? errbuf : curl_easy_strerror(rc),
		        (int)rc, http_code);
	}
	curl_easy_cleanup(c);
	return (rc == CURLE_OK) ? 0 : -1;
}

static int http_get_file(const char *url, const char *path) {
	FILE *fp;
	CURL *c;
	CURLcode rc;
	long http_code = 0;
	char errbuf[CURL_ERROR_SIZE];
	fp = fopen(path, "wb");
	if (!fp) return -1;
	c = curl_easy_init();
	if (!c) { fclose(fp); return -1; }
	errbuf[0] = 0;
	curl_easy_setopt(c, CURLOPT_URL, url);
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, NULL);  /* default fwrite */
	curl_easy_setopt(c, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(c, CURLOPT_MAXREDIRS, 10L);
	curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURLSSLOPT_NATIVE_CA
	curl_easy_setopt(c, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA);
#endif
	curl_easy_setopt(c, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(c, CURLOPT_USERAGENT, "gmt_update/1.0");
	curl_easy_setopt(c, CURLOPT_TIMEOUT, 600L);
	rc = curl_easy_perform(c);
	curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &http_code);
	if (rc != CURLE_OK) {
		fprintf(stderr, "  curl: %s (code=%d, http=%ld) url=%s\n",
		        errbuf[0] ? errbuf : curl_easy_strerror(rc),
		        (int)rc, http_code, url);
	}
	curl_easy_cleanup(c);
	fclose(fp);
	if (rc != CURLE_OK) { remove(path); return -1; }
	return 0;
}

/* ---------------- GitHub API resolver ----------------
 *
 * If <in_url> points at GitHub's "releases/latest" REST endpoint, fetch the
 * JSON, locate the asset whose name == "manifest_<channel>.txt", and copy
 * its browser_download_url into <out_url>. Otherwise pass <in_url> through.
 *
 * The JSON shape we care about:
 *   { ..., "assets": [
 *       { "name": "manifest_win_x64.txt", ...,
 *         "browser_download_url": "https://github.com/.../<asset>" },
 *       ...
 *   ] }
 *
 * Parsing strategy: scan for every "browser_download_url":"..." string and
 * pick the one whose value ends in "/manifest_<channel>.txt". Sufficient
 * because GitHub asset URLs always end in the asset filename. No real JSON
 * parser is pulled in; the cost is a couple of strstr/strchr passes.
 */
static int resolve_manifest_url(const char *in_url, const char *channel,
                                char *out_url, size_t out_sz) {
	struct mem mm = {0, 0};
	char needle[128];
	size_t needle_len;
	const char *p;
	int ok = -1;

	if (!strstr(in_url, "api.github.com/repos/")) {
		snprintf(out_url, out_sz, "%s", in_url);
		return 0;
	}

	fprintf(stdout, "resolving latest release via GitHub API: %s\n", in_url);
	if (http_get_mem(in_url, &mm) != 0) {
		fprintf(stderr, "ERROR: GitHub API request failed\n");
		free(mm.buf);
		return -1;
	}

	snprintf(needle, sizeof needle, "/manifest_%s.txt", channel);
	needle_len = strlen(needle);

	p = mm.buf;
	while (p && (p = strstr(p, "\"browser_download_url\"")) != NULL) {
		const char *colon = strchr(p, ':');
		const char *q1, *q2;
		size_t n;
		if (!colon) break;
		q1 = strchr(colon, '"');
		if (!q1) break;
		q2 = strchr(q1 + 1, '"');
		if (!q2) break;
		n = (size_t)(q2 - q1 - 1);
		if (n >= needle_len && n + 1 < out_sz) {
			if (memcmp(q2 - needle_len, needle, needle_len) == 0) {
				memcpy(out_url, q1 + 1, n);
				out_url[n] = 0;
				fprintf(stdout, "resolved -> %s\n", out_url);
				ok = 0;
				break;
			}
		}
		p = q2 + 1;
	}

	if (ok != 0)
		fprintf(stderr, "ERROR: latest release has no asset named 'manifest_%s.txt'\n", channel);

	free(mm.buf);
	return ok;
}

/* ---------------- SHA-256 of local file ---------------- */

static int sha256_file(const char *path, char hex_out[SHA_HEXLEN + 1]) {
	FILE *fp;
	SHA256_CTX ctx;
	unsigned char buf[8192];
	unsigned char digest[32];
	size_t r;
	int i;

	fp = fopen(path, "rb");
	if (!fp) return -1;
	sha256_init(&ctx);
	while ((r = fread(buf, 1, sizeof buf, fp)) > 0)
		sha256_update(&ctx, buf, r);
	fclose(fp);
	sha256_final(&ctx, digest);
	for (i = 0; i < 32; i++) sprintf(hex_out + i*2, "%02x", digest[i]);
	hex_out[SHA_HEXLEN] = 0;
	return 0;
}

/* ---------------- mtime / timestamp helpers ---------------- */

/* Return mtime of a file as time_t (0 on error). Used to detect "local copy
 * is newer than what the manifest is shipping" — a developer regression. */
static time_t file_mtime(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return (time_t)0;
	return st.st_mtime;
}

/* Parse ISO-8601 UTC like "2026-05-23T14:22:00Z" into time_t. Returns 0 on
 * any parse error so callers can detect "no usable timestamp". */
static time_t parse_iso8601_utc(const char *s) {
	struct tm tm;
	int Y, Mo, D, h, m, sec;
	if (!s || !*s) return (time_t)0;
	if (sscanf(s, "%d-%d-%dT%d:%d:%dZ", &Y, &Mo, &D, &h, &m, &sec) != 6)
		return (time_t)0;
	memset(&tm, 0, sizeof tm);
	tm.tm_year = Y - 1900;
	tm.tm_mon  = Mo - 1;
	tm.tm_mday = D;
	tm.tm_hour = h;
	tm.tm_min  = m;
	tm.tm_sec  = sec;
#ifdef _WIN32
	return _mkgmtime(&tm);
#else
	return timegm(&tm);
#endif
}

/* ---------------- manifest parser ---------------- */

static int parse_manifest(const char *text, struct manifest *m) {
	const char *p = text;
	char line[2048];

	memset(m, 0, sizeof *m);

	while (*p) {
		const char *nl = strchr(p, '\n');
		size_t len = nl ? (size_t)(nl - p) : strlen(p);
		if (len >= sizeof line) return -1;
		memcpy(line, p, len);
		line[len] = 0;
		while (len && (line[len-1] == '\r' || line[len-1] == ' ' || line[len-1] == '\t'))
			line[--len] = 0;
		p = nl ? nl + 1 : p + len;

		if (line[0] == '#' || line[0] == 0) continue;

		if (!strncmp(line, "version:", 8))      sscanf(line + 8, " %63s", m->version);
		else if (!strncmp(line, "build:", 6))   sscanf(line + 6, " %31s", m->build);
		else if (!strncmp(line, "channel:", 8)) sscanf(line + 8, " %15s", m->channel);
		else if (!strncmp(line, "file ", 5)) {
			struct entry *e;
			int nf;
			if (m->n >= MAX_FILES) {
				fprintf(stderr, "manifest: too many file entries (max %d)\n", MAX_FILES);
				return -1;
			}
			e = &m->f[m->n];
			e->url[0] = 0;
			nf = sscanf(line + 5, " %511s %ld %64s %1023s", e->path, &e->size, e->sha, e->url);
			if (nf != 3 && nf != 4) {
				fprintf(stderr, "manifest: malformed file line: %s\n", line);
				return -1;
			}
			m->n++;
		}
		else if (!strcmp(line, "end")) break;
	}
	return 0;
}

/* ---------------- Windows: report holders of a locked file ---------------- */

#ifdef _WIN32
static void warn_who_holds(const char *path) {
	DWORD session = 0;
	WCHAR key[CCH_RM_SESSION_KEY + 1] = {0};
	WCHAR wpath[MAX_PATH];
	PCWSTR files[1];
	UINT n_proc = 16, n_needed = 0;
	RM_PROCESS_INFO info[16];
	DWORD reason = 0;

	if (RmStartSession(&session, 0, key) != ERROR_SUCCESS) return;
	MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);
	files[0] = wpath;
	if (RmRegisterResources(session, 1, files, 0, NULL, 0, NULL) != ERROR_SUCCESS) {
		RmEndSession(session);
		return;
	}
	if (RmGetList(session, &n_needed, &n_proc, info, &reason) == ERROR_SUCCESS) {
		UINT i;
		if (n_proc == 0)
			fprintf(stderr, "    (no process explicitly holds the file, but write was refused)\n");
		for (i = 0; i < n_proc; i++)
			fprintf(stderr, "    held by PID %lu (%ls)\n", info[i].Process.dwProcessId, info[i].strAppName);
	}
	RmEndSession(session);
}
#else
static void warn_who_holds(const char *path) { (void)path; }
#endif

/* ---------------- URL helpers ---------------- */
/*
 * Strip the last path segment of `url` into `out` (no trailing slash). If
 * there's no '/' after the scheme, copy the whole thing.
 */
static void url_dirname(const char *url, char *out, size_t cap) {
	const char *q = strchr(url, '?');
	size_t len = q ? (size_t)(q - url) : strlen(url);
	size_t i;
	if (len >= cap) len = cap - 1;
	memcpy(out, url, len);
	out[len] = 0;
	for (i = len; i > 0; i--) {
		if (out[i-1] == '/') {
			/* Avoid stripping the "//" right after scheme. */
			if (i >= 2 && out[i-2] == '/') break;
			out[i-1] = 0;
			return;
		}
	}
}

/* ---------------- atomic swap ---------------- */
/*
 * Return values:
 *   0  applied immediately
 *   1  deferred: file staged as <target>.pending (target was locked)
 *  -1  error
 */
static int swap_in(const char *target, const char *staged) {
#ifdef _WIN32
	char pending[MAX_PATH_S * 2];
	DWORD err;

	/* Path A: direct replace. Works when nobody has target loaded. */
	if (MoveFileExA(staged, target, MOVEFILE_REPLACE_EXISTING))
		return 0;

	err = GetLastError();
	if (err != ERROR_SHARING_VIOLATION && err != ERROR_ACCESS_DENIED &&
	    err != ERROR_USER_MAPPED_FILE) {
		fprintf(stderr, "    swap failed err=%lu\n", err);
		return -1;
	}

	/* Path B: target locked. Stage as <target>.pending. */
	warn_who_holds(target);
	snprintf(pending, sizeof pending, "%s.pending", target);
	DeleteFileA(pending);
	if (!MoveFileExA(staged, pending, MOVEFILE_REPLACE_EXISTING)) {
		fprintf(stderr, "    could not stage pending err=%lu\n", GetLastError());
		return -1;
	}
	return 1;
#else
	if (rename(staged, target) != 0) {
		fprintf(stderr, "    rename failed errno=%d\n", errno);
		return -1;
	}
	return 0;
#endif
}

/* ---------------- per-file update driver ---------------- */

static int update_one(const char *install_root, struct entry *e, int *out_deferred) {
	char target[MAX_PATH_S * 2];
	char staged[MAX_PATH_S * 2 + 8];
	char got[SHA_HEXLEN + 1];
	int rc;

	snprintf(target, sizeof target, "%s" PATH_SEP "%s", install_root, e->path);
	snprintf(staged, sizeof staged, "%s.new", target);

	fprintf(stderr, "  fetch  %s\n", e->path);
	if (http_get_file(e->url, staged) != 0) {
		fprintf(stderr, "    download failed: %s\n", e->url);
		return -1;
	}
	if (sha256_file(staged, got) != 0 || strcmp(got, e->sha) != 0) {
		fprintf(stderr, "    sha mismatch\n      got  %s\n      want %s\n", got, e->sha);
		remove(staged);
		return -1;
	}
	rc = swap_in(target, staged);
	if (rc < 0) {
		remove(staged);
		return -1;
	}
	if (rc == 1) {
		fprintf(stderr, "    LOCKED — staged as %s.pending (apply on next load)\n", target);
		(*out_deferred)++;
	}
	else {
		fprintf(stderr, "    ok\n");
	}
	return 0;
}

/* ---------------- self-locate ---------------- */
/*
 * Discover the install root by inspecting our own executable path. We assume
 * gmt_update lives in <install_root>/bin/, so the parent of the exe's dir is
 * the install root.
 *
 * Symlink handling: any of the path components may be a symlink (a common
 * case on Windows is `C:\programs\bin` -> some-real-build-dir). We must
 * resolve to the *real* target before stripping, otherwise we end up at
 * a parent of the symlink rather than the parent of the real bin/.
 *   Windows: GetFinalPathNameByHandleA() returns the canonical path with
 *            symlinks/junctions/mount-points resolved. Comes back as
 *            "\\?\C:\real\path" -- strip the prefix.
 *   POSIX  : realpath() canonicalises the whole path.
 */
static int self_install_root(char out[MAX_PATH_S]) {
	char exe[MAX_PATH_S];
	char real[MAX_PATH_S];
	char *s1, *s2;
#ifdef _WIN32
	HANDLE h;
	DWORD n = GetModuleFileNameA(NULL, exe, sizeof exe);
	if (n == 0 || n >= sizeof exe) return -1;
	h = CreateFileA(exe, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		/* Could not open — fall back to the un-resolved exe path */
		strncpy(real, exe, sizeof real - 1);
		real[sizeof real - 1] = 0;
	}
	else {
		DWORD m = GetFinalPathNameByHandleA(h, real, sizeof real, 0);
		CloseHandle(h);
		if (m == 0 || m >= sizeof real) {
			strncpy(real, exe, sizeof real - 1);
			real[sizeof real - 1] = 0;
		}
		/* Strip "\\?\" prefix that GetFinalPathNameByHandle prepends. */
		if (!strncmp(real, "\\\\?\\", 4)) memmove(real, real + 4, strlen(real + 4) + 1);
	}
#elif defined(__APPLE__)
	uint32_t sz = sizeof exe;
	if (_NSGetExecutablePath(exe, &sz) != 0) return -1;
	if (!realpath(exe, real)) {
		strncpy(real, exe, sizeof real - 1);
		real[sizeof real - 1] = 0;
	}
#else
	ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
	if (n <= 0 || (size_t)n >= sizeof exe) return -1;
	exe[n] = 0;
	if (!realpath(exe, real)) {
		strncpy(real, exe, sizeof real - 1);
		real[sizeof real - 1] = 0;
	}
#endif
	/* Strip basename → dir of exe */
	s1 = strrchr(real, '/');
	s2 = strrchr(real, '\\');
	if (s1 < s2) s1 = s2;
	if (!s1) return -1;
	*s1 = 0;
	/* Strip one more component → install root (parent of bin/) */
	s1 = strrchr(real, '/');
	s2 = strrchr(real, '\\');
	if (s1 < s2) s1 = s2;
	if (!s1) return -1;
	*s1 = 0;
	strncpy(out, real, MAX_PATH_S - 1);
	out[MAX_PATH_S - 1] = 0;
	return 0;
}

/* ---------------- version handling ---------------- */
/*
 * Parse "MAJOR.MINOR.PATCH_<git>[-dirty]_YYYY.MM.DD" — what `gmt --version`
 * prints. Returns 1 on success, 0 on failure. The `-dirty` suffix may be
 * present on developer builds with uncommitted changes (Linux/Mac path);
 * Windows tends to not include it but the parser handles either way.
 */
static int parse_version(const char *s, struct ver *v) {
	char buf[64];
	char *tok1, *tok2, *tok3, *save;
	(void)save;
	memset(v, 0, sizeof *v);
	if (!s || !*s) return 0;
	strncpy(buf, s, sizeof buf - 1);
	buf[sizeof buf - 1] = 0;
	tok1 = strtok(buf, "_");
	tok2 = tok1 ? strtok(NULL, "_") : NULL;
	tok3 = tok2 ? strtok(NULL, "_") : NULL;
	if (!tok1 || !tok2 || !tok3) return 0;
	if (sscanf(tok1, "%d.%d.%d", &v->semver[0], &v->semver[1], &v->semver[2]) != 3)
		return 0;
	strncpy(v->hash, tok2, sizeof v->hash - 1);
	v->hash[sizeof v->hash - 1] = 0;
	v->is_dirty = (strstr(v->hash, "-dirty") != NULL);
	if (sscanf(tok3, "%d.%d.%d", &v->date[0], &v->date[1], &v->date[2]) != 3)
		return 0;
	v->valid = 1;
	return 1;
}

/* Order on (semver, date). Hash ignored. -1 a<b, 0 equal, +1 a>b. */
static int vercmp(const struct ver *a, const struct ver *b) {
	int i;
	for (i = 0; i < 3; i++) {
		if (a->semver[i] != b->semver[i])
			return (a->semver[i] < b->semver[i]) ? -1 : 1;
	}
	for (i = 0; i < 3; i++) {
		if (a->date[i] != b->date[i])
			return (a->date[i] < b->date[i]) ? -1 : 1;
	}
	return 0;
}

/* Build a command string that runs `gmt --version` and captures stdout. */
static int run_capture(const char *cmd, char out[64]) {
	FILE *fp;
	size_t n;
	char *nl;
	out[0] = 0;
#ifdef _WIN32
	fp = _popen(cmd, "r");
#else
	fp = popen(cmd, "r");
#endif
	if (!fp) return -1;
	if (!fgets(out, 64, fp)) { out[0] = 0; }
#ifdef _WIN32
	_pclose(fp);
#else
	pclose(fp);
#endif
	n = strlen(out);
	while (n && (out[n-1] == '\n' || out[n-1] == '\r' || out[n-1] == ' ' || out[n-1] == '\t'))
		out[--n] = 0;
	nl = strchr(out, '\n');
	if (nl) *nl = 0;
	return out[0] ? 0 : -1;
}

/*
 * Try `<install_root>/bin/gmt --version` first. If it fails (e.g. install
 * layout differs from <root>/bin/<exe>), fall back to bare `gmt --version`
 * which uses PATH lookup.
 */
static int probe_local_version(const char *install_root, char out[64]) {
	char cmd[MAX_PATH_S * 2];
#ifdef _WIN32
	snprintf(cmd, sizeof cmd, "\"%s\\bin\\gmt.exe\" --version 2>NUL", install_root);
#else
	snprintf(cmd, sizeof cmd, "\"%s/bin/gmt\" --version 2>/dev/null", install_root);
#endif
	if (run_capture(cmd, out) == 0) return 0;
	/* Fallback: rely on PATH. */
#ifdef _WIN32
	snprintf(cmd, sizeof cmd, "gmt.exe --version 2>NUL");
#else
	snprintf(cmd, sizeof cmd, "gmt --version 2>/dev/null");
#endif
	return run_capture(cmd, out);
}

/* ---------------- manifest generator ---------------- */

/* Windows defaults shared across x64 and arm64 — the _w64 suffix is just
 * GMT's BITAGE-based DLL_RENAME (set in ConfigUser*.cmake), independent of
 * the ISA. If a future build switches naming on ARM64, override with --files.
 */
static const char *DEFAULT_FILES_WIN[] = {
	"bin/gmt_w64.dll",
	"bin/postscriptlight_w64.dll",
	"bin/gmt_plugins/supplements_w64.dll",
	"bin/gmt.exe",
	NULL
};

static const char *DEFAULT_FILES_LINUX[] = {
	"lib/libgmt.so",
	"lib/libpostscriptlight.so",
	"lib/libsupplements.so",
	"bin/gmt",
	NULL
};

static const char *DEFAULT_FILES_MACOS[] = {
	"lib/libgmt.dylib",
	"lib/libpostscriptlight.dylib",
	"lib/libsupplements.dylib",
	"bin/gmt",
	NULL
};

static long file_size_bytes(const char *path) {
	FILE *fp = fopen(path, "rb");
	long n;
	if (!fp) return -1;
	if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return -1; }
	n = ftell(fp);
	fclose(fp);
	return n;
}

static const char *path_basename(const char *p) {
	const char *s1 = strrchr(p, '/');
	const char *s2 = strrchr(p, '\\');
	const char *s  = (s1 > s2) ? s1 : s2;
	return s ? s + 1 : p;
}

/* Strip leading/trailing whitespace in-place. Return new start. */
static char *trim(char *s) {
	char *e;
	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
	e = s + strlen(s);
	while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
	return s;
}

/* Load file list (one relpath per line, '#' comments). Caller frees *out. */
static int load_files_from(const char *list_path, char ***out, int *n_out) {
	FILE *fp = fopen(list_path, "r");
	char buf[MAX_PATH_S];
	char **list = NULL;
	int n = 0, cap = 0;
	if (!fp) { fprintf(stderr, "ERROR: cannot open %s\n", list_path); return -1; }
	while (fgets(buf, sizeof buf, fp)) {
		char *t = trim(buf);
		if (*t == 0 || *t == '#') continue;
		if (n == cap) {
			cap = cap ? cap * 2 : 16;
			list = (char **)realloc(list, cap * sizeof *list);
			if (!list) { fclose(fp); return -1; }
		}
		list[n] = strdup(t);
		if (!list[n]) { fclose(fp); return -1; }
		n++;
	}
	fclose(fp);
	*out = list;
	*n_out = n;
	return 0;
}

static int make_manifest(int argc, char **argv) {
	const char *install_root = NULL;
	const char *version      = NULL;
	const char *channel      = DEFAULT_CHANNEL;
	const char *out_path     = NULL;
	const char *files_from   = NULL;
	const char **defaults = NULL;
	char **listed = NULL;
	int n_listed = 0;
	FILE *out;
	int i, emitted = 0, skipped = 0;
	char probed[64];
	char root_buf[MAX_PATH_S];
	struct ver pv;

	/* argv[0] is "--make-manifest". Only flags after that:
	 *   -o <file>, --files <listfile>, --version <str>, --root <dir>
	 */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-o") && i + 1 < argc) {
			out_path = argv[++i];
		}
		else if (!strcmp(argv[i], "--files") && i + 1 < argc) {
			files_from = argv[++i];
		}
		else if (!strcmp(argv[i], "--version") && i + 1 < argc) {
			version = argv[++i];
		}
		else if (!strcmp(argv[i], "--root") && i + 1 < argc) {
			install_root = argv[++i];
		}
		else {
			fprintf(stderr,
			    "ERROR: unexpected argument: %s\n"
			    "Usage: gmt_update --make-manifest [-o <output_file>] [--files <listfile>] [--version <str>] [--root <dir>]\n"
			    "       (channel auto-set to compile-time default: %s)\n",
			    argv[i], DEFAULT_CHANNEL);
			return 1;
		}
	}
	if (!install_root) {
		if (self_install_root(root_buf) != 0) {
			fprintf(stderr, "ERROR: cannot determine own install root\n");
			return 1;
		}
		install_root = root_buf;
	}

	/* Auto-probe version from local gmt --version unless overridden. */
	if (!version) {
		if (probe_local_version(install_root, probed) != 0) {
			fprintf(stderr,
			    "ERROR: cannot probe version. Tried:\n"
			    "         \"%s\" " PATH_SEP "bin" PATH_SEP "gmt --version\n"
			    "         gmt --version  (via PATH)\n"
			    "       Both failed. Either:\n"
			    "         - point --root <dir> at the correct GMT install\n"
			    "         - or pass --version <str> to set it explicitly.\n",
			    install_root);
			return 1;
		}
		version = probed;
		fprintf(stderr, "probed local version: %s\n", version);
	}
	if (parse_version(version, &pv) && pv.is_dirty) {
		fprintf(stderr, "WARNING: version contains '-dirty' — manifest reflects an uncommitted build.\n"
		                "Do NOT publish this manifest to end users.\n");
	}

	/* Pick file list */
	if (files_from) {
		if (load_files_from(files_from, &listed, &n_listed) != 0) return 1;
	}
	else {
#if defined(_WIN32)
		defaults = DEFAULT_FILES_WIN;
#elif defined(__APPLE__)
		defaults = DEFAULT_FILES_MACOS;
#else
		defaults = DEFAULT_FILES_LINUX;
#endif
	}

	/* Default output filename mirrors the default download URL: writing
	 * to manifest_<channel>.txt in the current directory means the file
	 * can be uploaded as-is and the updater will find it by the same name. */
	{
		static char default_out[64];
		if (!out_path) {
			snprintf(default_out, sizeof default_out, "manifest_%s.txt", DEFAULT_CHANNEL);
			out_path = default_out;
		}
	}
	out = fopen(out_path, "w");
	if (!out) {
		fprintf(stderr, "ERROR: cannot write %s\n", out_path);
		return 1;
	}

	fprintf(out, "# GMT update manifest -- generated by gmt_update --make-manifest\n");
	fprintf(out, "# install_root: %s\n", install_root);
	fprintf(out, "\n");
	fprintf(out, "version: %s\n", version);
	{
		time_t t = time(NULL);
		struct tm *g = gmtime(&t);
		fprintf(out, "build:   %04d-%02d-%02dT%02d:%02d:%02dZ\n", g->tm_year + 1900, g->tm_mon + 1, g->tm_mday,
		        g->tm_hour, g->tm_min, g->tm_sec);
	}
	fprintf(out, "channel: %s\n\n", channel);

	{
		int count = listed ? n_listed : 0;
		if (!listed) { while (defaults[count]) count++; }
		for (i = 0; i < count; i++) {
			const char *rel = listed ? listed[i] : defaults[i];
			char full[MAX_PATH_S * 2];
			char hex[SHA_HEXLEN + 1];
			long sz;
			snprintf(full, sizeof full, "%s" PATH_SEP "%s", install_root, rel);
			sz = file_size_bytes(full);
			if (sz < 0) {
				fprintf(stderr, "  SKIP   %s (not found)\n", full);
				skipped++;
				continue;
			}
			if (sha256_file(full, hex) != 0) {
				fprintf(stderr, "  SKIP   %s (sha read failed)\n", full);
				skipped++;
				continue;
			}
			fprintf(out, "file %-40s %12ld %s\n", rel, sz, hex);
			emitted++;
		}
	}

	fprintf(out, "\nend\n");
	fclose(out);

	if (listed) {
		for (i = 0; i < n_listed; i++) free(listed[i]);
		free(listed);
	}

	fprintf(stderr, "manifest: %d emitted, %d skipped -> %s\n", emitted, skipped, out_path);
	return (emitted > 0) ? 0 : 1;
}

/* ---------------- inspect mode ---------------- */

static int do_show(const char *manifest_url, const char *install_root) {
	struct mem mm = {0, 0};
	struct manifest m;
	int i, stale = 0, missing = 0, pending = 0;
	char local_ver[64];
	struct ver vl, vr;
	int have_local, have_remote, cmp = 0;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	char resolved[MAX_URL];
	if (resolve_manifest_url(manifest_url, DEFAULT_CHANNEL, resolved, sizeof resolved) != 0) {
		curl_global_cleanup();
		return 1;
	}
	manifest_url = strdup(resolved);  /* small leak at program exit */

	fprintf(stdout, "manifest URL: %s\n", manifest_url);
	if (http_get_mem(manifest_url, &mm) != 0) {
		fprintf(stderr, "ERROR: could not fetch manifest\n");
		curl_global_cleanup();
		return 1;
	}
	if (parse_manifest(mm.buf, &m) != 0) {
		fprintf(stderr, "ERROR: could not parse manifest\n");
		free(mm.buf);
		curl_global_cleanup();
		return 1;
	}
	free(mm.buf);

	have_local  = (probe_local_version(install_root, local_ver) == 0) && parse_version(local_ver, &vl);
	have_remote = parse_version(m.version, &vr);

	fprintf(stdout, "local  version: %s%s\n", have_local ? local_ver : "<unknown>",
	        (have_local && vl.is_dirty) ? "   [DIRTY BUILD]" : "");
	fprintf(stdout, "remote version: %s\n", m.version);
	fprintf(stdout, "remote build:   %s\n", m.build);
	fprintf(stdout, "remote channel: %s\n", m.channel);
	if (have_local && have_remote) {
		cmp = vercmp(&vr, &vl);
		fprintf(stdout, "version delta:  %s\n",
		        cmp > 0 ? "REMOTE NEWER (upgrade available)" :
		        cmp < 0 ? "REMOTE OLDER (downgrade would be refused)" :
		                  "same");
	}
	fprintf(stdout, "install root:   %s\n", install_root);
	fprintf(stdout, "files in manifest: %d\n\n", m.n);

	fprintf(stdout, "%-8s %-40s %-12s %-12s %s\n", "STATUS", "PATH", "LOCAL", "REMOTE", "PENDING?");
	for (i = 0; i < m.n; i++) {
		struct entry *e = &m.f[i];
		char target[MAX_PATH_S * 2];
		char pend[MAX_PATH_S * 2 + 16];
		char have[SHA_HEXLEN + 1];
		char short_local[13];
		char short_remote[13];
		const char *status;
		int has_pending;
		FILE *fp;

		snprintf(target, sizeof target, "%s" PATH_SEP "%s", install_root, e->path);
		snprintf(pend,   sizeof pend,   "%s.pending", target);

		fp = fopen(pend, "rb");
		has_pending = (fp != NULL);
		if (fp) fclose(fp);
		if (has_pending) pending++;

		if (sha256_file(target, have) != 0) {
			status = "MISSING";
			strcpy(short_local, "-");
			missing++;
		}
		else if (strcmp(have, e->sha) != 0) {
			status = "STALE";
			memcpy(short_local, have, 12); short_local[12] = 0;
			stale++;
		}
		else {
			status = "current";
			memcpy(short_local, have, 12); short_local[12] = 0;
		}
		memcpy(short_remote, e->sha, 12); short_remote[12] = 0;

		fprintf(stdout, "%-8s %-40s %-12s %-12s %s\n", status, e->path, short_local, short_remote, has_pending ? "yes" : "");
	}
	fprintf(stdout, "\nsummary: %d stale, %d missing, %d pending\n", stale, missing, pending);

	curl_global_cleanup();
	return 0;
}

/* ---------------- help ---------------- */

static void print_help(FILE *out) {
	fprintf(out,
"gmt_update — fetch & apply GMT binary updates from a remote manifest.\n"
"             (compile-time channel: " DEFAULT_CHANNEL ")\n"
"\n"
"USAGE\n"
"  gmt_update [<manifest_url>] [--dry-run] [--allow-dirty] [--root <dir>]\n"
"      Diff local install vs remote manifest, download and atomically\n"
"      replace any stale files. Install root is auto-derived from the\n"
"      gmt_update executable's own path (parent of bin/). Before any\n"
"      destructive replacement happens you are asked 'Proceed? [y/N]';\n"
"      any answer other than 'y' aborts cleanly.\n"
"      <manifest_url>   default: " GMT_UPDATE_DEFAULT_URL "\n"
"      --dry-run        list what would change; do not download (no prompt).\n"
"      --allow-dirty    permit running against a dirty developer build.\n"
"      --root <dir>     override auto-derived install root.\n"
"\n"
"  gmt_update --show [<manifest_url>] [--root <dir>]\n"
"      Inspect-only. Prints local vs remote version, per-file SHA diff,\n"
"      and flags any leftover '<file>.pending' from a previous locked run.\n"
"\n"
"  gmt_update --make-manifest [-o <file>] [--files <list>] [--version <str>] [--root <dir>]\n"
"      Generate a manifest from the local install. Hashes each file with\n"
"      SHA-256 and emits one 'file' line per artefact. The manifest carries\n"
"      no download URLs — at update time the updater resolves each file\n"
"      as a sibling of the manifest URL itself. Upload the manifest and\n"
"      the binaries to the same location.\n"
"        -o <file>      write to file. Default: ./manifest_" DEFAULT_CHANNEL ".txt\n"
"                       (same basename the updater asks for via GitHub API).\n"
"        --files <lst>  override default OS list; one relpath per line, # comments.\n"
"        --version <s>  override auto-probed version. Default = run\n"
"                       `<root>/bin/gmt --version` and use that string.\n"
"        --root <dir>   hash a tree other than this binary's own install.\n"
"\n"
"  gmt_update --help | -h\n"
"      Show this help and exit.\n"
"\n"
"VERSION GATE\n"
"  gmt_update spawns `<install_root>/bin/gmt --version` to learn the local\n"
"  version (e.g. 6.7.0_d3435c5_2026.05.19, or 6.7.0_8b76b26-dirty_... on a\n"
"  developer build). Compares to manifest `version:` on (semver, date).\n"
"  Refuses to apply an older manifest. Refuses on dirty builds unless\n"
"  --allow-dirty is given.\n"
"\n"
"NOTES\n"
"  Files locked by a running GMT process are staged as <file>.pending and\n"
"  applied on the next run when the lock is released. On Windows the\n"
"  Restart Manager API reports which PIDs hold a lock.\n");
}

/* ---------------- main ---------------- */

int main(int argc, char **argv) {
	const char *manifest_url;
	const char *install_root;
	int dry_run;

	if (argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		print_help(stdout);
		return 0;
	}
	if (argc > 1 && !strcmp(argv[1], "--make-manifest")) {
		return make_manifest(argc - 1, argv + 1);
	}
	if (argc > 1 && !strcmp(argv[1], "--show")) {
		const char *url  = GMT_UPDATE_DEFAULT_URL;
		const char *root = NULL;
		char root_buf[MAX_PATH_S];
		int k, pos = 0;
		for (k = 2; k < argc; k++) {
			if (!strcmp(argv[k], "--root") && k + 1 < argc) {
				root = argv[++k];
			}
			else if (argv[k][0] == '-' && argv[k][1] == '-') {
				fprintf(stderr, "ERROR: unknown flag %s\n", argv[k]);
				return 1;
			}
			else if (pos++ == 0) {
				url = argv[k];
			}
			else {
				fprintf(stderr, "ERROR: extra arg %s\n", argv[k]);
				return 1;
			}
		}
		if (!root) {
			if (self_install_root(root_buf) != 0) {
				fprintf(stderr, "ERROR: cannot determine install root. Pass --root <dir>.\n");
				return 1;
			}
			root = root_buf;
		}
		return do_show(url, root);
	}

	{
		int allow_dirty = 0;
		int pos = 0, k;
		struct mem mm = {0, 0};
		struct manifest m;
		char local_ver[64];
		char root_buf[MAX_PATH_S];
		struct ver vl, vr;
		int have_local, have_remote, cmp;
		int i, stale = 0, errors = 0, deferred = 0;
		int regressions = 0, kept_local = 0, apply_regressions = 0;
		time_t build_time = 0;

		manifest_url = GMT_UPDATE_DEFAULT_URL;
		install_root = NULL;
		dry_run = 0;
		for (k = 1; k < argc; k++) {
			const char *a = argv[k];
			if (!strcmp(a, "--dry-run"))                       dry_run = 1;
			else if (!strcmp(a, "--allow-dirty"))              allow_dirty = 1;
			else if (!strcmp(a, "--root") && k + 1 < argc)     install_root = argv[++k];
			else if (a[0] == '-' && a[1] == '-') {
				fprintf(stderr, "ERROR: unknown flag %s\n", a);
				return 1;
			}
			else {
				switch (pos++) {
					case 0: manifest_url = a; break;
					default:
						fprintf(stderr, "ERROR: extra positional argument: %s\n", a);
						return 1;
				}
			}
		}
		if (!install_root) {
			if (self_install_root(root_buf) != 0) {
				fprintf(stderr,
				    "ERROR: cannot determine own install root. "
				    "Pass --root <dir>.\n");
				return 1;
			}
			install_root = root_buf;
		}

		curl_global_init(CURL_GLOBAL_DEFAULT);

		{
			char resolved[MAX_URL];
			if (resolve_manifest_url(manifest_url, DEFAULT_CHANNEL,
			                         resolved, sizeof resolved) != 0) {
				curl_global_cleanup();
				return 1;
			}
			manifest_url = strdup(resolved);  /* leak at program exit */
		}

		fprintf(stderr, "manifest URL: %s\n", manifest_url);
		if (http_get_mem(manifest_url, &mm) != 0) {
			fprintf(stderr, "ERROR: could not fetch manifest\n");
			curl_global_cleanup();
			return 1;
		}
		if (parse_manifest(mm.buf, &m) != 0) {
			fprintf(stderr, "ERROR: could not parse manifest\n");
			free(mm.buf);
			curl_global_cleanup();
			return 1;
		}
		free(mm.buf);
		build_time = parse_iso8601_utc(m.build);

		fprintf(stderr, "manifest: version=%s build=%s channel=%s files=%d\n",
		        m.version, m.build, m.channel, m.n);
		fprintf(stderr, "install root: %s\n", install_root);

		/* version gate */
		have_local  = (probe_local_version(install_root, local_ver) == 0)
		              && parse_version(local_ver, &vl);
		have_remote = parse_version(m.version, &vr);

		if (!have_local) {
			fprintf(stderr, "WARNING: could not probe local version (skipping version gate)\n");
		}
		else {
			fprintf(stderr, "local version: %s\n", local_ver);
			if (vl.is_dirty && !allow_dirty) {
				fprintf(stderr,
				    "REFUSING: local build is dirty (uncommitted changes). "
				    "Updates would overwrite local work.\n"
				    "         Pass --allow-dirty to override.\n");
				curl_global_cleanup();
				return 1;
			}
			if (have_remote) {
				cmp = vercmp(&vr, &vl);
				if (cmp < 0) {
					fprintf(stderr,
					    "REFUSING: remote manifest (%s) is older than local (%s).\n"
					    "         No legitimate downgrade path exists. Aborting.\n",
					    m.version, local_ver);
					curl_global_cleanup();
					return 1;
				}
			}
			else {
				fprintf(stderr, "WARNING: manifest version unparseable; gate skipped\n");
			}
		}

		/* diff phase */
	for (i = 0; i < m.n; i++) {
		struct entry *e = &m.f[i];
		char target[MAX_PATH_S * 2];
		char have[SHA_HEXLEN + 1];
		snprintf(target, sizeof target, "%s" PATH_SEP "%s", install_root, e->path);
		if (sha256_file(target, have) != 0) {
			e->needs_update = 1; stale++;
			fprintf(stderr, "  MISSING     %s\n", e->path);
		}
		else if (strcmp(have, e->sha) != 0) {
			e->needs_update = 1; stale++;
			/* Regression check: a STALE file whose local mtime is newer than
			 * the manifest's build timestamp almost certainly means the user
			 * just rebuilt it locally — replacing would erase that work. */
			if (build_time > 0) {
				time_t lmt = file_mtime(target);
				if (lmt > build_time) {
					e->regression = 1;
					regressions++;
					fprintf(stderr,
					    "  WARN-NEWER  %s  (local mtime is newer than manifest build)\n",
					    e->path);
				}
				else {
					fprintf(stderr, "  STALE       %s\n", e->path);
				}
			}
			else {
				fprintf(stderr, "  STALE       %s\n", e->path);
			}
		}
		else {
			fprintf(stderr, "  current     %s\n", e->path);
		}
	}

	if (stale == 0) {
		fprintf(stderr, "nothing to do\n");
		curl_global_cleanup();
		return 0;
	}
	if (dry_run) {
		fprintf(stderr, "%d file(s) would update (dry-run)", stale);
		if (regressions > 0)
			fprintf(stderr, "; %d would be DOWNGRADES (local newer than manifest)",
			        regressions);
		fprintf(stderr, "\n");
		curl_global_cleanup();
		return 0;
	}

	/* Confirmation gate — a real update is destructive (replaces binaries
	 * in install_root), so require an interactive y/N from the user. Empty
	 * input, EOF, or anything not starting with 'y'/'Y' aborts cleanly. */
	{
		char ans[16];
		fprintf(stderr,
		    "\n%d file(s) will be downloaded and replace local copies under\n"
		    "  %s\n", stale, install_root);
		if (regressions > 0) {
			fprintf(stderr,
			    "WARNING: %d of those files are NEWER on disk than the manifest's\n"
			    "         build timestamp (%s).\n"
			    "         Replacing them would OVERWRITE local newer copies\n"
			    "         (typically a just-rebuilt developer DLL).\n",
			    regressions, m.build);
		}
		fprintf(stderr, "Proceed? [y/N] ");
		fflush(stderr);
		if (!fgets(ans, sizeof ans, stdin) || (ans[0] != 'y' && ans[0] != 'Y')) {
			fprintf(stderr, "aborted by user — no files changed\n");
			curl_global_cleanup();
			return 0;
		}
	}

	/* Second prompt, only when there are regressions: ask whether to also
	 * overwrite the locally-newer files. Default no → those are skipped. */
	if (regressions > 0) {
		char ans2[16];
		fprintf(stderr,
		    "Also overwrite the %d locally-newer file(s) (DOWNGRADE)? [y/N] ",
		    regressions);
		fflush(stderr);
		if (fgets(ans2, sizeof ans2, stdin) && (ans2[0] == 'y' || ans2[0] == 'Y'))
			apply_regressions = 1;
		else
			fprintf(stderr, "keeping local newer file(s); only non-regression files will update\n");
	}

	/* apply phase */
	{
		char base_dir[MAX_URL];
		url_dirname(manifest_url, base_dir, sizeof base_dir);
		for (i = 0; i < m.n; i++) {
			struct entry *e = &m.f[i];
			if (!e->needs_update) continue;
			if (e->regression && !apply_regressions) {
				fprintf(stderr, "  KEEP-LOCAL  %s\n", e->path);
				kept_local++;
				continue;
			}
			if (e->url[0] == 0) {
				/* No URL column in manifest → resolve as sibling of manifest. */
				snprintf(e->url, sizeof e->url, "%s/%s",
				         base_dir, path_basename(e->path));
			}
			if (update_one(install_root, e, &deferred) != 0) errors++;
		}
	}

	curl_global_cleanup();

	if (errors) {
		fprintf(stderr,
		    "DONE with %d error(s), %d applied, %d deferred, %d kept-local\n",
		    errors, stale - errors - deferred - kept_local, deferred, kept_local);
		return 2;
	}
	fprintf(stderr, "DONE %d file(s) updated, %d deferred, %d kept-local\n",
	        stale - deferred - kept_local, deferred, kept_local);
	return 0;
	}
}
