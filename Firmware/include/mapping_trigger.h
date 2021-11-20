#ifndef __MAPPING_TRIGGER_H
#define __MAPPING_TRIGGER_H

#include "mapping.h"

typedef struct mapping_trigger_s mapping_trigger_t;
typedef struct mapping_trigger_data_s mapping_trigger_data_t;
typedef struct mapping_trigger_conf_s mapping_trigger_conf_t;

struct mapping_trigger_conf_s {
	mapping_point_t center;
	float size;
	float note;
};

struct mapping_trigger_data_s {
	mapping_rect_t rect;
};

struct mapping_trigger_s {
	mapping_t _inherited;
	mapping_trigger_conf_t conf;
	mapping_trigger_data_t data;
};

extern int mapping_trigger_create(mapping_trigger_conf_t *conf);

#endif /* __MAPPING_TRIGGER_H */
