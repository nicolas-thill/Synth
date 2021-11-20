#ifndef __MAPPING_TOGGLE_H
#define __MAPPING_TOGGLE_H

#include "mapping.h"

typedef struct mapping_toggle_s mapping_toggle_t;
typedef struct mapping_toggle_conf_s mapping_toggle_conf_t;
typedef struct mapping_toggle_data_s mapping_toggle_data_t;

struct mapping_toggle_conf_s {
	mapping_point_t center;
	float size;
	float note;
};

struct mapping_toggle_data_s {
	mapping_rect_t rect;
	boolean state;
};

struct mapping_toggle_s {
	mapping_t _inherited;
	mapping_toggle_conf_t conf;
	mapping_toggle_data_t data;
};

extern int mapping_toggle_create(mapping_toggle_conf_t *conf);

#endif /* __MAPPING_TOGGLE_H */
