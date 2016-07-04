#ifndef GETOPT_H
#define GETOPT_H
#include <windows.h>
#ifdef __cplusplus
extern "C"
{
#endif
	extern int opterr, // if error message should be printed /
			optind, // index into parent argv vector /
			optopt, // character checked for validity /
			optreset; // reset getopt /

	extern char *optarg; // argument associated with option */
	extern wchar_t *optarg_w; // argument associated with option */

	int getopt(int nargc, char * const nargv[], const char *ostr);
	int getopt_w(int nargc, wchar_t * const nargv[], const wchar_t *ostr);
#ifdef __cplusplus
}
#endif
#endif