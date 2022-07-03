#define MIXBUFSIZE 1024
#define DSPMIXBUFSIZE 16
#define MIXRATE 32000

#define	EXPORT(func) \
		.global func;\
		.type func,% function;\
		func:
