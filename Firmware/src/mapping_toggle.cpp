#include "mapping_toggle.h"
#include "midi_bus.h"

#define MAPPING_CONF(m)  (((mapping_toggle_t *)(m))->conf)
#define MAPPING_DATA(m)  (((mapping_toggle_t *)(m))->data)

int mapping_toggle_check(mapping_t *mapping, blob_t *blob);
void mapping_toggle_setup(mapping_t *mapping);

static mapping_impl_t mapping_toggle_impl = {
	.name = "toggle",
	.desc = "Toggle",
	.type = MAPPING_TOGGLE,
	.func = mapping_toggle_check,
};

int mapping_toggle_create(mapping_toggle_conf_t *conf) {
	mapping_t *mapping = mapping_create(&mapping_toggle_impl, sizeof(mapping_toggle_t));
	MAPPING_CONF(mapping) = *conf;
	mapping_toggle_setup(mapping);
	return 1;
}

void mapping_toggle_setup(mapping_t *mapping) {
	float half_size = round(MAPPING_CONF(mapping).size / 2);
	MAPPING_DATA(mapping).rect.Xmin = MAPPING_CONF(mapping).center.X - half_size;
	MAPPING_DATA(mapping).rect.Xmax = MAPPING_CONF(mapping).center.X + half_size;
	MAPPING_DATA(mapping).rect.Ymin = MAPPING_CONF(mapping).center.Y - half_size;
	MAPPING_DATA(mapping).rect.Ymax = MAPPING_CONF(mapping).center.Y + half_size;
	MAPPING_DATA(mapping).state = false;
}

int mapping_toggle_check(mapping_t *mapping, blob_t *blob) {
	if (blob->centroid.X > MAPPING_DATA(mapping).rect.Xmin &&
		blob->centroid.X < MAPPING_DATA(mapping).rect.Xmax &&
		blob->centroid.Y > MAPPING_DATA(mapping).rect.Ymin &&
		blob->centroid.Y < MAPPING_DATA(mapping).rect.Ymax) {
		if (!blob->lastState) {
			MAPPING_DATA(mapping).state = !MAPPING_DATA(mapping).state;
			if (MAPPING_DATA(mapping).state) {
#if defined(DEBUG_MAPPING)
				Serial.printf("\nDEBUG_MAPPING_TOGGLES:\tNOTE_ON : %d", MAPPING_CONF(mapping).note);
#else
				midi_enqueue_out(midi::NoteOn, MAPPING_CONF(mapping).note, 127);
#endif
			} else {
#if defined(DEBUG_MAPPING)
				Serial.printf("\nDEBUG_MAPPING_TOGGLES:\tNOTE_OFF : %d", MAPPING_CONF(mapping).note);
#else
				midi_enqueue_out(midi::NoteOff, MAPPING_CONF(mapping).note, 0);
#endif
			}
		}
	}
	return 0;
}
