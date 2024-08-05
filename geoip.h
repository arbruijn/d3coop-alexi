#pragma once

#include <stdio.h>
//#include<stdlib.h>
//#include <time.h>
//#include <windows.h>
#include <sys/types.h>


#define SEGMENT_RECORD_LENGTH 3
#define STANDARD_RECORD_LENGTH 3
#define ORG_RECORD_LENGTH 4
#define MAX_RECORD_LENGTH 4
#define NUM_DB_TYPES 20

/* 128 bit address in network order */
//typedef struct in6_addr geoipv6_t;
 
#define GEOIP_CHKBIT_V6(bit,ptr) (ptr[((127UL - bit) >> 3)] & (1UL << (~(127 - bit) & 7)))

typedef struct GeoIPTag
{
  FILE *GeoIPDatabase;
  char *file_path;
	unsigned char *cache;
	unsigned char *index_cache;
	unsigned int *databaseSegments;
	char databaseType;
	time_t mtime;
	int flags;
	off_t	size;
	char record_length;
	int charset; /* 0 iso-8859-1 1 utf8 */
	int record_iter; /* used in GeoIP_next_record */
	int netmask; /* netmask of last lookup - set using depth in _GeoIP_seek_record */
	time_t last_mtime_check;
} GeoIP;


typedef enum
{
	GEOIP_CHARSET_ISO_8859_1 = 0,
	GEOIP_CHARSET_UTF8       = 1
} GeoIPCharset;

typedef enum
{
	GEOIP_STANDARD = 0,
	GEOIP_MEMORY_CACHE = 1,
	GEOIP_CHECK_CACHE = 2,
	GEOIP_INDEX_CACHE = 4,
	GEOIP_MMAP_CACHE = 8,
} GeoIPOptions;

typedef enum
{
	GEOIP_COUNTRY_EDITION     = 1,
	GEOIP_REGION_EDITION_REV0 = 7,
	GEOIP_CITY_EDITION_REV0   = 6,
	GEOIP_ORG_EDITION         = 5,
	GEOIP_ISP_EDITION         = 4,
	GEOIP_CITY_EDITION_REV1   = 2,
	GEOIP_REGION_EDITION_REV1 = 3,
	GEOIP_PROXY_EDITION       = 8,
	GEOIP_ASNUM_EDITION       = 9,
	GEOIP_NETSPEED_EDITION    = 10,
	GEOIP_DOMAIN_EDITION      = 11,
        GEOIP_COUNTRY_EDITION_V6  = 12,
} GeoIPDBTypes;


extern const char * GeoIP_country_name[253];

#ifdef DLL
#define GEOIP_API __declspec(dllexport)
#else
#define GEOIP_API
#endif  /* DLL */

GEOIP_API GeoIP* GeoIP_new(int flags);
GEOIP_API GeoIP* GeoIP_open(const char * filename, int flags);
GEOIP_API void GeoIP_delete(GeoIP* gi);
GEOIP_API int GeoIP_id_by_ipnum (GeoIP* gi, unsigned long ipnum);

