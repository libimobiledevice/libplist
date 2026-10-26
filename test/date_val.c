#include <stdio.h>
#include <stdint.h>
#include <plist/plist.h>

/* plist_get_date_val must return exactly what plist_set_date_val stored,
 * including dates before the epoch (negative seconds). */

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

static int check(int32_t sec, int32_t usec)
{
	int32_t s = 0, u = 0;
	plist_t d = plist_new_date(0, 0);
	plist_set_date_val(d, sec, usec);
	plist_get_date_val(d, &s, &u);
	plist_free(d);
	if (s != sec || u != usec) {
		printf("set(%d, %d) -> get(%d, %d)\n", sec, usec, s, u);
		return 1;
	}
	return 0;
}

int main(int argc, char** argv)
{
	int err = 0;
	int32_t sec, usec;

	err |= check(5, 6);
	err |= check(0, 999999);
	err |= check(-3, 250000);
	err |= check(-1, 1);
	err |= check(1000000000, 123456);
	for (sec = -2; sec <= 2 && !err; sec++) {
		for (usec = 0; usec < 1000000 && !err; usec += 7) {
			err |= check(sec, usec);
		}
	}
	return err;
}
