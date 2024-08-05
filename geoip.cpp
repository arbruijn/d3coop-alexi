// parts of MaxMind's GeoIP library

#include "geoip.h"
#include <stdlib.h>
#include "types.h"
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __unix__
#include <sys/mman.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif
// Alex :  added for Linux
#define GEOIPDATADIR "."


#define COUNTRY_BEGIN 16776960
#define STATE_BEGIN_REV0 16700000
#define STATE_BEGIN_REV1 16000000
#define STRUCTURE_INFO_MAX_SIZE 20
#define DATABASE_INFO_MAX_SIZE 100
#define MAX_ORG_RECORD_LENGTH 300
#define US_OFFSET 1
#define CANADA_OFFSET 677
#define WORLD_OFFSET 1353
#define FIPS_RANGE 360

const char GeoIP_country_code[253][3] = { "--","AP","EU","AD","AE","AF","AG","AI","AL","AM","AN",
	"AO","AQ","AR","AS","AT","AU","AW","AZ","BA","BB",
	"BD","BE","BF","BG","BH","BI","BJ","BM","BN","BO",
	"BR","BS","BT","BV","BW","BY","BZ","CA","CC","CD",
	"CF","CG","CH","CI","CK","CL","CM","CN","CO","CR",
	"CU","CV","CX","CY","CZ","DE","DJ","DK","DM","DO",
	"DZ","EC","EE","EG","EH","ER","ES","ET","FI","FJ",
	"FK","FM","FO","FR","FX","GA","GB","GD","GE","GF",
	"GH","GI","GL","GM","GN","GP","GQ","GR","GS","GT",
	"GU","GW","GY","HK","HM","HN","HR","HT","HU","ID",
	"IE","IL","IN","IO","IQ","IR","IS","IT","JM","JO",
	"JP","KE","KG","KH","KI","KM","KN","KP","KR","KW",
	"KY","KZ","LA","LB","LC","LI","LK","LR","LS","LT",
	"LU","LV","LY","MA","MC","MD","MG","MH","MK","ML",
	"MM","MN","MO","MP","MQ","MR","MS","MT","MU","MV",
	"MW","MX","MY","MZ","NA","NC","NE","NF","NG","NI",
	"NL","NO","NP","NR","NU","NZ","OM","PA","PE","PF",
	"PG","PH","PK","PL","PM","PN","PR","PS","PT","PW",
	"PY","QA","RE","RO","RU","RW","SA","SB","SC","SD",
	"SE","SG","SH","SI","SJ","SK","SL","SM","SN","SO",
	"SR","ST","SV","SY","SZ","TC","TD","TF","TG","TH",
	"TJ","TK","TM","TN","TO","TL","TR","TT","TV","TW",
	"TZ","UA","UG","UM","US","UY","UZ","VA","VC","VE",
	"VG","VI","VN","VU","WF","WS","YE","YT","RS","ZA",
	"ZM","ME","ZW","A1","A2","O1","AX","GG","IM","JE",
  "BL","MF"};

static const unsigned num_GeoIP_countries = (unsigned)(sizeof(GeoIP_country_code)/sizeof(GeoIP_country_code[0]));

const char GeoIP_country_code3[253][4] = { "--","AP","EU","AND","ARE","AFG","ATG","AIA","ALB","ARM","ANT",
	"AGO","AQ","ARG","ASM","AUT","AUS","ABW","AZE","BIH","BRB",
	"BGD","BEL","BFA","BGR","BHR","BDI","BEN","BMU","BRN","BOL",
	"BRA","BHS","BTN","BV","BWA","BLR","BLZ","CAN","CC","COD",
	"CAF","COG","CHE","CIV","COK","CHL","CMR","CHN","COL","CRI",
	"CUB","CPV","CX","CYP","CZE","DEU","DJI","DNK","DMA","DOM",
	"DZA","ECU","EST","EGY","ESH","ERI","ESP","ETH","FIN","FJI",
	"FLK","FSM","FRO","FRA","FX","GAB","GBR","GRD","GEO","GUF",
	"GHA","GIB","GRL","GMB","GIN","GLP","GNQ","GRC","GS","GTM",
	"GUM","GNB","GUY","HKG","HM","HND","HRV","HTI","HUN","IDN",
	"IRL","ISR","IND","IO","IRQ","IRN","ISL","ITA","JAM","JOR",
	"JPN","KEN","KGZ","KHM","KIR","COM","KNA","PRK","KOR","KWT",
	"CYM","KAZ","LAO","LBN","LCA","LIE","LKA","LBR","LSO","LTU",
	"LUX","LVA","LBY","MAR","MCO","MDA","MDG","MHL","MKD","MLI",
	"MMR","MNG","MAC","MNP","MTQ","MRT","MSR","MLT","MUS","MDV",
	"MWI","MEX","MYS","MOZ","NAM","NCL","NER","NFK","NGA","NIC",
	"NLD","NOR","NPL","NRU","NIU","NZL","OMN","PAN","PER","PYF",
	"PNG","PHL","PAK","POL","SPM","PCN","PRI","PSE","PRT","PLW",
	"PRY","QAT","REU","ROU","RUS","RWA","SAU","SLB","SYC","SDN",
	"SWE","SGP","SHN","SVN","SJM","SVK","SLE","SMR","SEN","SOM",
	"SUR","STP","SLV","SYR","SWZ","TCA","TCD","TF","TGO","THA",
	"TJK","TKL","TKM","TUN","TON","TLS","TUR","TTO","TUV","TWN",
	"TZA","UKR","UGA","UM","USA","URY","UZB","VAT","VCT","VEN",
	"VGB","VIR","VNM","VUT","WLF","WSM","YEM","YT","SRB","ZAF",
	"ZMB","MNE","ZWE","A1","A2","O1","ALA","GGY","IMN","JEY",
  "BLM","MAF"};

const char * GeoIP_country_name[253] = {"N/A","Asia/Pacific Region","Europe","Andorra","United Arab Emirates","Afghanistan","Antigua and Barbuda","Anguilla","Albania","Armenia","Netherlands Antilles",
	"Angola","Antarctica","Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia and Herzegovina","Barbados",
	"Bangladesh","Belgium","Burkina Faso","Bulgaria","Bahrain","Burundi","Benin","Bermuda","Brunei Darussalam","Bolivia",
	"Brazil","Bahamas","Bhutan","Bouvet Island","Botswana","Belarus","Belize","Canada","Cocos (Keeling) Islands","Congo, The Democratic Republic of the",
	"Central African Republic","Congo","Switzerland","Cote D'Ivoire","Cook Islands","Chile","Cameroon","China","Colombia","Costa Rica",
	"Cuba","Cape Verde","Christmas Island","Cyprus","Czech Republic","Germany","Djibouti","Denmark","Dominica","Dominican Republic",
	"Algeria","Ecuador","Estonia","Egypt","Western Sahara","Eritrea","Spain","Ethiopia","Finland","Fiji",
	"Falkland Islands (Malvinas)","Micronesia, Federated States of","Faroe Islands","France","France, Metropolitan","Gabon","United Kingdom","Grenada","Georgia","French Guiana",
	"Ghana","Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial Guinea","Greece","South Georgia and the South Sandwich Islands","Guatemala",
	"Guam","Guinea-Bissau","Guyana","Hong Kong","Heard Island and McDonald Islands","Honduras","Croatia","Haiti","Hungary","Indonesia",
	"Ireland","Israel","India","British Indian Ocean Territory","Iraq","Iran, Islamic Republic of","Iceland","Italy","Jamaica","Jordan",
	"Japan","Kenya","Kyrgyzstan","Cambodia","Kiribati","Comoros","Saint Kitts and Nevis","Korea, Democratic People's Republic of","Korea, Republic of","Kuwait",
	"Cayman Islands","Kazakhstan","Lao People's Democratic Republic","Lebanon","Saint Lucia","Liechtenstein","Sri Lanka","Liberia","Lesotho","Lithuania",
	"Luxembourg","Latvia","Libyan Arab Jamahiriya","Morocco","Monaco","Moldova, Republic of","Madagascar","Marshall Islands","Macedonia","Mali",
	"Myanmar","Mongolia","Macau","Northern Mariana Islands","Martinique","Mauritania","Montserrat","Malta","Mauritius","Maldives",
	"Malawi","Mexico","Malaysia","Mozambique","Namibia","New Caledonia","Niger","Norfolk Island","Nigeria","Nicaragua",
	"Netherlands","Norway","Nepal","Nauru","Niue","New Zealand","Oman","Panama","Peru","French Polynesia",
	"Papua New Guinea","Philippines","Pakistan","Poland","Saint Pierre and Miquelon","Pitcairn Islands","Puerto Rico","Palestinian Territory","Portugal","Palau",
	"Paraguay","Qatar","Reunion","Romania","Russian Federation","Rwanda","Saudi Arabia","Solomon Islands","Seychelles","Sudan",
	"Sweden","Singapore","Saint Helena","Slovenia","Svalbard and Jan Mayen","Slovakia","Sierra Leone","San Marino","Senegal","Somalia","Suriname",
	"Sao Tome and Principe","El Salvador","Syrian Arab Republic","Swaziland","Turks and Caicos Islands","Chad","French Southern Territories","Togo","Thailand",
	"Tajikistan","Tokelau","Turkmenistan","Tunisia","Tonga","Timor-Leste","Turkey","Trinidad and Tobago","Tuvalu","Taiwan",
	"Tanzania, United Republic of","Ukraine","Uganda","United States Minor Outlying Islands","United States","Uruguay","Uzbekistan","Holy See (Vatican City State)","Saint Vincent and the Grenadines","Venezuela",
	"Virgin Islands, British","Virgin Islands, U.S.","Vietnam","Vanuatu","Wallis and Futuna","Samoa","Yemen","Mayotte","Serbia","South Africa",
	"Zambia","Montenegro","Zimbabwe","Anonymous Proxy","Satellite Provider","Other","Aland Islands","Guernsey","Isle of Man","Jersey",
  "Saint Barthelemy","Saint Martin"};

/* Possible continent codes are AF, AS, EU, NA, OC, SA for Africa, Asia, Europe, North America, Oceania
and South America. */

const char GeoIP_country_continent[253][3] = {"--","AS","EU","EU","AS","AS","SA","SA","EU","AS","SA",
	"AF","AN","SA","OC","EU","OC","SA","AS","EU","SA",
	"AS","EU","AF","EU","AS","AF","AF","SA","AS","SA",
	"SA","SA","AS","AF","AF","EU","SA","NA","AS","AF",
	"AF","AF","EU","AF","OC","SA","AF","AS","SA","SA",
	"SA","AF","AS","AS","EU","EU","AF","EU","SA","SA",
	"AF","SA","EU","AF","AF","AF","EU","AF","EU","OC",
	"SA","OC","EU","EU","EU","AF","EU","SA","AS","SA",
	"AF","EU","SA","AF","AF","SA","AF","EU","SA","SA",
	"OC","AF","SA","AS","AF","SA","EU","SA","EU","AS",
	"EU","AS","AS","AS","AS","AS","EU","EU","SA","AS",
	"AS","AF","AS","AS","OC","AF","SA","AS","AS","AS",
	"SA","AS","AS","AS","SA","EU","AS","AF","AF","EU",
	"EU","EU","AF","AF","EU","EU","AF","OC","EU","AF",
	"AS","AS","AS","OC","SA","AF","SA","EU","AF","AS",
	"AF","NA","AS","AF","AF","OC","AF","OC","AF","SA",
	"EU","EU","AS","OC","OC","OC","AS","SA","SA","OC",
	"OC","AS","AS","EU","SA","OC","SA","AS","EU","OC",
	"SA","AS","AF","EU","AS","AF","AS","OC","AF","AF",
	"EU","AS","AF","EU","EU","EU","AF","EU","AF","AF",
	"SA","AF","SA","AS","AF","SA","AF","AF","AF","AS",
	"AS","OC","AS","AF","OC","AS","AS","SA","OC","AS",
	"AF","EU","AF","OC","NA","SA","AS","EU","SA","SA",
	"SA","SA","AS","OC","OC","OC","AS","AF","EU","AF",
	"AF","EU","AF","--","--","--","EU","EU","EU","EU",
  "SA","SA"};

const char * GeoIPDBDescription[NUM_DB_TYPES] = {NULL, "GeoIP Country Edition", "GeoIP City Edition, Rev 1", "GeoIP Region Edition, Rev 1", "GeoIP ISP Edition", "GeoIP Organization Edition", "GeoIP City Edition, Rev 0", "GeoIP Region Edition, Rev 0","GeoIP Proxy Edition","GeoIP ASNum Edition","GeoIP Netspeed Edition","GeoIP Domain Name Edition", "GeoIP Country V6 Edition"};

char * custom_directory = NULL;


char *_GeoIP_full_path_to(const char *file_name) {
	int len;
	char *path = (char*)malloc(sizeof(char) * 1024);

	if (custom_directory == NULL){
#if !defined(WIN32) && !defined(WIN64)
		memset(path, 0, sizeof(char) * 1024);
		snprintf(path, sizeof(char) * 1024 - 1, "%s/%s", GEOIPDATADIR, file_name);
#else
		char buf[MAX_PATH], *p, *q = NULL;
		memset(buf, 0, sizeof(buf));
		len = GetModuleFileName(GetModuleHandle(NULL), buf, sizeof(buf) - 1);
		for (p = buf + len; p > buf; p--)
			if (*p == '\\')
				{
					if (!q)
						q = p;
					else
						*p = '/';
				}
		*q = 0;
		memset(path, 0, sizeof(char) * 1024);
		snprintf(path, sizeof(char) * 1024 - 1, "%s/%s", buf, file_name);
#endif
	} else {
		len = strlen(custom_directory);
		if (custom_directory[len-1] != '/') {
			snprintf(path, sizeof(char) * 1024 - 1, "%s/%s",custom_directory, file_name);
		} else {
			snprintf(path, sizeof(char) * 1024 - 1, "%s%s", custom_directory, file_name);
		}
	}
	return path;
}

char ** GeoIPDBFileName = NULL;

void _GeoIP_setup_dbfilename() {
	if (NULL == GeoIPDBFileName) {
		GeoIPDBFileName = (char**)malloc(sizeof(char *) * NUM_DB_TYPES);
		memset(GeoIPDBFileName, 0, sizeof(char *) * NUM_DB_TYPES);

		GeoIPDBFileName[GEOIP_COUNTRY_EDITION]		= _GeoIP_full_path_to("GeoIP.dat");
		GeoIPDBFileName[GEOIP_REGION_EDITION_REV0]	= _GeoIP_full_path_to("GeoIPRegion.dat");
		GeoIPDBFileName[GEOIP_REGION_EDITION_REV1]	= _GeoIP_full_path_to("GeoIPRegion.dat");
		GeoIPDBFileName[GEOIP_CITY_EDITION_REV0]	= _GeoIP_full_path_to("GeoIPCity.dat");
		GeoIPDBFileName[GEOIP_CITY_EDITION_REV1]	= _GeoIP_full_path_to("GeoIPCity.dat");
		GeoIPDBFileName[GEOIP_ISP_EDITION]		= _GeoIP_full_path_to("GeoIPISP.dat");
		GeoIPDBFileName[GEOIP_ORG_EDITION]		= _GeoIP_full_path_to("GeoIPOrg.dat");
		GeoIPDBFileName[GEOIP_PROXY_EDITION]		= _GeoIP_full_path_to("GeoIPProxy.dat");
		GeoIPDBFileName[GEOIP_ASNUM_EDITION]		= _GeoIP_full_path_to("GeoIPASNum.dat");
		GeoIPDBFileName[GEOIP_NETSPEED_EDITION]		= _GeoIP_full_path_to("GeoIPNetSpeed.dat");
		GeoIPDBFileName[GEOIP_DOMAIN_EDITION]		= _GeoIP_full_path_to("GeoIPDomain.dat");
                GeoIPDBFileName[GEOIP_COUNTRY_EDITION_V6]       = _GeoIP_full_path_to("GeoIPv6.dat");
	}
}


static
void _setup_segments(GeoIP * gi) {
	int i, j;
	unsigned char delim[3];
	unsigned char buf[SEGMENT_RECORD_LENGTH];

	gi->databaseSegments = NULL;

	/* default to GeoIP Country Edition */
	gi->databaseType = GEOIP_COUNTRY_EDITION;
	gi->record_length = STANDARD_RECORD_LENGTH;
	fseek(gi->GeoIPDatabase, -3l, SEEK_END);
	for (i = 0; i < STRUCTURE_INFO_MAX_SIZE; i++) {
		fread(delim, 1, 3, gi->GeoIPDatabase);
		if (delim[0] == 255 && delim[1] == 255 && delim[2] == 255) {
			fread(&gi->databaseType, 1, 1, gi->GeoIPDatabase);
			if (gi->databaseType >= 106) {
				/* backwards compatibility with databases from April 2003 and earlier */
				gi->databaseType -= 105;
			}

			if (gi->databaseType == GEOIP_REGION_EDITION_REV0) {
				/* Region Edition, pre June 2003 */
				gi->databaseSegments = (u32*)malloc(sizeof(int));
				gi->databaseSegments[0] = STATE_BEGIN_REV0;
			} else if (gi->databaseType == GEOIP_REGION_EDITION_REV1) {
				/* Region Edition, post June 2003 */
				gi->databaseSegments = (u32*)malloc(sizeof(int));
				gi->databaseSegments[0] = STATE_BEGIN_REV1;
			} else if (gi->databaseType == GEOIP_CITY_EDITION_REV0 ||
								 gi->databaseType == GEOIP_CITY_EDITION_REV1 ||
								 gi->databaseType == GEOIP_ORG_EDITION ||
								 gi->databaseType == GEOIP_ISP_EDITION ||
								 gi->databaseType == GEOIP_ASNUM_EDITION) {
				/* City/Org Editions have two segments, read offset of second segment */
				gi->databaseSegments = (u32*)malloc(sizeof(int));
				gi->databaseSegments[0] = 0;
				fread(buf, SEGMENT_RECORD_LENGTH, 1, gi->GeoIPDatabase);
				for (j = 0; j < SEGMENT_RECORD_LENGTH; j++) {
					gi->databaseSegments[0] += (buf[j] << (j * 8));
				}
				if (gi->databaseType == GEOIP_ORG_EDITION ||
						gi->databaseType == GEOIP_ISP_EDITION)
					gi->record_length = ORG_RECORD_LENGTH;
			}
			break;
		} else {
			fseek(gi->GeoIPDatabase, -4l, SEEK_CUR);
		}
	}
	if (gi->databaseType == GEOIP_COUNTRY_EDITION ||
			gi->databaseType == GEOIP_PROXY_EDITION ||
			gi->databaseType == GEOIP_NETSPEED_EDITION ||
			gi->databaseType == GEOIP_COUNTRY_EDITION_V6 ) {
		gi->databaseSegments = (u32*)malloc(sizeof(int));
		gi->databaseSegments[0] = COUNTRY_BEGIN;
	}
}


unsigned int _GeoIP_seek_record (GeoIP *gi, unsigned long ipnum)
{
	int depth;
	unsigned int x;
	unsigned char stack_buffer[2 * MAX_RECORD_LENGTH];
	const unsigned char *buf = (gi->cache == NULL) ? stack_buffer : NULL;
	unsigned int offset = 0;

	const unsigned char * p;
	int j;

// Alex off
//	_check_mtime(gi);
	for (depth = 31; depth >= 0; depth--)
	{
		if (gi->cache == NULL && gi->index_cache == NULL)
		{
			/* read from disk */
			fseek(gi->GeoIPDatabase, (long)gi->record_length * 2 * offset, SEEK_SET);
			fread(stack_buffer,gi->record_length,2,gi->GeoIPDatabase);
		}
		else if (gi->index_cache == NULL)
		{
			/* simply point to record in memory */
			buf = gi->cache + (long)gi->record_length * 2 *offset;
		}
		else
		{
			buf = gi->index_cache + (long)gi->record_length * 2 * offset;
		}

		if (ipnum & (1 << depth))
		{
			/* Take the right-hand branch */
			if ( gi->record_length == 3 )
			{
				/* Most common case is completely unrolled and uses constants. */
				x =   (buf[3*1 + 0] << (0*8))
					+ (buf[3*1 + 1] << (1*8))
					+ (buf[3*1 + 2] << (2*8));

			}
			else
			{
				/* General case */
				j = gi->record_length;
				p = &buf[2*j];
				x = 0;
				do
				{
					x <<= 8;
					x += *(--p);
				} while ( --j );
			}

		}
		else
		{
			/* Take the left-hand branch */
			if ( gi->record_length == 3 )
			{
				/* Most common case is completely unrolled and uses constants. */
				x =   (buf[3*0 + 0] << (0*8))
					+ (buf[3*0 + 1] << (1*8))
					+ (buf[3*0 + 2] << (2*8));
			}
			else
			{
				/* General case */
				j = gi->record_length;
				p = &buf[1*j];
				x = 0;
				do
				{
					x <<= 8;
					x += *(--p);
				} while ( --j );
			}
		}

		if (x >= gi->databaseSegments[0])
		{
			gi->netmask = 32 - depth;
			return x;
		}
		offset = x;
	}

	/* shouldn't reach here */
	fprintf(stderr,"Error Traversing Database for ipnum = %lu - Perhaps database is corrupt?\n",ipnum);
	return 0;
}

GeoIP* GeoIP_new (int flags) {
	GeoIP * gi;
	_GeoIP_setup_dbfilename();
	gi = GeoIP_open (GeoIPDBFileName[GEOIP_COUNTRY_EDITION], flags);
	return gi;
}

GeoIP* GeoIP_open (const char * filename, int flags) {
	struct stat buf;
	GeoIP * gi;
	size_t len;

	gi = (GeoIP *)malloc(sizeof(GeoIP));
	if (gi == NULL)
		return NULL;
	len = sizeof(char) * (strlen(filename)+1);
	gi->file_path = (char*)malloc(len);
	if (gi->file_path == NULL) {
		free(gi);
		return NULL;
	}
	strncpy(gi->file_path, filename, len);
	gi->GeoIPDatabase = fopen(filename,"rb");
	if (gi->GeoIPDatabase == NULL) {
		fprintf(stderr,"Error Opening file %s\n",filename);
		free(gi->file_path);
		free(gi);
		return NULL;
	} else {
		if (flags & (GEOIP_MEMORY_CACHE | GEOIP_MMAP_CACHE) ) {
			if (fstat(_fileno(gi->GeoIPDatabase), &buf) == -1) {
				fprintf(stderr,"Error stating file %s\n",filename);
				free(gi->file_path);
				free(gi);
				return NULL;
			}
			gi->mtime = buf.st_mtime;
			gi->size = buf.st_size;

// Alex off :  it just bugs Linux with 'PROT_READ'
#if 1
			/* MMAP added my Peter Shipley */
			if ( flags & GEOIP_MMAP_CACHE )
			{
#if !defined(WIN32) && !defined(WIN64)
			    gi->cache = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fileno(gi->GeoIPDatabase), 0);
			    if ( gi->cache == MAP_FAILED ) {
				fprintf(stderr,"Error mmaping file %s\n",filename);
				free(gi->file_path);
				free(gi);
				return NULL;
			    }
#endif
			}
			else
#endif
			{
			    gi->cache = (unsigned char *) malloc(sizeof(unsigned char) * buf.st_size);

			    if (gi->cache != NULL) {
				if (fread(gi->cache, sizeof(unsigned char), buf.st_size, gi->GeoIPDatabase) != (size_t) buf.st_size) {
					fprintf(stderr,"Error reading file %s\n",filename);
					free(gi->cache);
					free(gi->file_path);
					free(gi);
					return NULL;
				}
			    }
			}
		} else {
			if (flags & GEOIP_CHECK_CACHE) {
				if (fstat(_fileno(gi->GeoIPDatabase), &buf) == -1) {
					fprintf(stderr,"Error stating file %s\n",filename);
					free(gi->file_path);
					free(gi);
					return NULL;
				}
				gi->mtime = buf.st_mtime;
			}
			gi->cache = NULL;
		}
		gi->flags = flags;
		gi->charset = GEOIP_CHARSET_ISO_8859_1;

		_setup_segments(gi);
		if (flags & GEOIP_INDEX_CACHE) {                        
			gi->index_cache = (unsigned char *) malloc(sizeof(unsigned char) * ((gi->databaseSegments[0] * (long)gi->record_length * 2)));
			if (gi->index_cache != NULL) {
				fseek(gi->GeoIPDatabase, 0, SEEK_SET);
				if (fread(gi->index_cache, sizeof(unsigned char), gi->databaseSegments[0] * (long)gi->record_length * 2, gi->GeoIPDatabase) != (size_t) (gi->databaseSegments[0]*(long)gi->record_length * 2)) {
					fprintf(stderr,"Error reading file %s\n",filename);
					free(gi->databaseSegments);
					free(gi->index_cache);
					free(gi);
					return NULL;
				}
			}
		} else {
			gi->index_cache = NULL;
		}
		return gi;
	}
}

void GeoIP_delete (GeoIP* gi)
{
	if (gi == NULL )
		return;
	if (gi->GeoIPDatabase != NULL)
		fclose(gi->GeoIPDatabase);
	if (gi->cache != NULL) {
	    if ( gi->flags & GEOIP_MMAP_CACHE ) {
#if !defined(WIN32) && !defined(WIN64)
		munmap(gi->cache, gi->size);
#endif
	    } else {
		free(gi->cache);
	    }
	    gi->cache = NULL;
	}
	if (gi->index_cache != NULL)
		free(gi->index_cache);
	if (gi->file_path != NULL)
		free(gi->file_path);
	if (gi->databaseSegments != NULL)
		free(gi->databaseSegments);
	free(gi);
}


int GeoIP_id_by_ipnum (GeoIP* gi, unsigned long ipnum)
{
	int ret;
	if (ipnum == 0) {
		return 0;
	}
	if (gi->databaseType != GEOIP_COUNTRY_EDITION && 
			gi->databaseType != GEOIP_PROXY_EDITION &&
			gi->databaseType != GEOIP_NETSPEED_EDITION) {
		printf("Invalid database type %s, expected %s\n",
					 GeoIPDBDescription[(int)gi->databaseType],
					 GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
		return 0;
	}
	ret = _GeoIP_seek_record(gi, ipnum) - COUNTRY_BEGIN;
	return ret;
}

