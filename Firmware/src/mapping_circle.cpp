#include "mapping_circle.h"

#define MAPPING_CONF(m)  (((mapping_circle_t *)(m))->conf)
#define MAPPING_DATA(m)  (((mapping_circle_t *)(m))->data)

int mapping_circle_check(mapping_t *mapping, blob_t *blob);
void mapping_circle_setup(mapping_t *mapping);

static mapping_impl_t mapping_circle_impl = {
	.name = "circle",
	.desc = "Circle",
	.type = MAPPING_CIRCLE,
	.func = mapping_circle_check,
};

int mapping_circle_create(mapping_circle_conf_t *conf) {
	mapping_t *mapping = mapping_create(&mapping_circle_impl, sizeof(mapping_circle_t));
	MAPPING_CONF(mapping) = *conf;
	mapping_circle_setup(mapping);
	return 1;
}

void mapping_circle_setup(mapping_t *mapping) {
}

int mapping_circle_check(mapping_t *mapping, blob_t *blob) {
	float x = blob->centroid.X - MAPPING_CONF(mapping).radius;
	float y = blob->centroid.Y - MAPPING_CONF(mapping).radius;
	float radius = sqrt(x * x + y * y);
	if (radius < MAPPING_CONF(mapping).radius) {
		float theta = 0;
		// Rotation of Axes through an angle without shifting Origin
		float posX = x * cos(MAPPING_CONF(mapping).offset) + y * sin(MAPPING_CONF(mapping).offset);
		float posY = -x * sin(MAPPING_CONF(mapping).offset) + y * cos(MAPPING_CONF(mapping).offset);
		if (posX == 0 && 0 < posY) {
			theta = PiII;
		} else if (posX == 0 && posY < 0) {
			theta = IIIPiII;
		} else if (posX < 0) {
			theta = atanf(posY / posX) + PI;
		} else if (posY < 0) {
			theta = atanf(posY / posX) + IIPi;
		} else {
			theta = atanf(posY / posX);
		}
#if defined(DEBUG_MAPPING_CIRCLE)
		Serial.printf("\nDEBUG_MAPPING_CIRCLES:\tCircleID:\t%d\tradius:\t%fTheta:\t%f", MAPPING_ID(mapping), radius, theta);
#endif
		return 1;
	};
	return 0;
}
