#ifndef __MAPPING_H
#define __MAPPING_H

#include "blob.h"
#include "llist.h"


enum mapping_type_e {
	MAPPING_CIRCLE,
	MAPPING_GRID,
	MAPPING_POLYGON,
	MAPPING_SLIDER_C,
	MAPPING_SLIDER_H,
	MAPPING_SLIDER_V,
	MAPPING_TOGGLE,
	MAPPING_TOUCHPAD,
	MAPPING_TRIGGER,
	MAPPING_UNKNOWN
};

typedef enum mapping_type_e mapping_type_t;


typedef struct mapping_point_s mapping_point_t;

struct mapping_point_s {
	float X;
	float Y;
};


typedef struct mapping_rect_s mapping_rect_t;

struct mapping_rect_s {
	float Xmin;
	float Xmax;
	float Ymin;
	float Ymax;
};


typedef struct mapping_key_s mapping_key_t;

struct mapping_key_s {
	float X;
	float Y;
	float size;
	float note;
};


typedef struct mapping_switch_s mapping_switch_t;

struct mapping_switch_s {
	mapping_rect_t rect;
	boolean state;
};


typedef struct mapping_s mapping_t;

typedef int (*mapping_check_func_t)(mapping_t *mapping, blob_t *blob);

typedef struct mapping_impl_s mapping_impl_t;

struct mapping_impl_s {
	const char *name;
	const char *desc;
	mapping_type_t type;
	mapping_check_func_t func;
};

struct mapping_s {
	lnode_t node;
	u_int8_t id;
	mapping_impl_t *impl;
	u_int8_t data[0];
};

#define MAPPING(m)  ((mapping_t *)(m))
#define MAPPING_ID(m)  (MAPPING(m)->id)
#define MAPPING_IMPL(m)  (MAPPING(m)->impl)

extern int mapping_setup_all(void);
extern void mapping_update_all(void);

extern mapping_t *mapping_create(mapping_impl_t *impl, u_int8_t size);

#endif /* __MAPPING_H */
