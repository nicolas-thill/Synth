#ifndef __MAPPING_CIRCLE_H
#define __MAPPING_CIRCLE_H

#include "mapping.h"

typedef struct mapping_circle_s mapping_circle_t;
typedef struct mapping_circle_conf_s mapping_circle_conf_t;
typedef struct mapping_circle_data_s mapping_circle_data_t;

struct mapping_circle_conf_s {
	mapping_point_t center;
	float radius;
	float offset;
};

struct mapping_circle_data_s {
};

struct mapping_circle_s {
	mapping_t _inherited;
	mapping_circle_conf_t conf;
	mapping_circle_data_t data;
};

extern int mapping_circle_create(mapping_circle_conf_t *conf);

#endif /* __MAPPING_CIRCLE_H */
