#include "mapping_trigger.h"
#include "midi_bus.h"

#define MAPPING_CONF(m)  (((mapping_trigger_t *)(m))->conf)
#define MAPPING_DATA(m)  (((mapping_trigger_t *)(m))->data)

int mapping_trigger_check(mapping_t *mapping, blob_t *blob);
void mapping_trigger_setup(mapping_t *mapping);

static mapping_impl_t mapping_trigger_impl = {
	.name = "trigger",
	.desc = "Trigger",
	.type = MAPPING_TRIGGER,
	.func = mapping_trigger_check,
};

int mapping_trigger_create(mapping_trigger_conf_t *conf) {
	mapping_t *mapping = mapping_create(&mapping_trigger_impl, sizeof(mapping_trigger_t));
	MAPPING_CONF(mapping) = *conf;
	mapping_trigger_setup(mapping);
	return 1;
}

void mapping_trigger_setup(mapping_t *mapping) {
	float half_size = round(MAPPING_CONF(mapping).size / 2);
	MAPPING_DATA(mapping).rect.Xmin = MAPPING_CONF(mapping).center.X - half_size;
	MAPPING_DATA(mapping).rect.Xmax = MAPPING_CONF(mapping).center.X + half_size;
	MAPPING_DATA(mapping).rect.Ymin = MAPPING_CONF(mapping).center.Y - half_size;
	MAPPING_DATA(mapping).rect.Ymax = MAPPING_CONF(mapping).center.Y + half_size;
}

int mapping_trigger_check(mapping_t *mapping, blob_t *blob) {
	if (blob->centroid.X > MAPPING_DATA(mapping).rect.Xmin &&
		blob->centroid.X < MAPPING_DATA(mapping).rect.Xmax &&
		blob->centroid.Y > MAPPING_DATA(mapping).rect.Ymin &&
		blob->centroid.Y < MAPPING_DATA(mapping).rect.Ymax) {
		if (!blob->lastState) {
			midi_enqueue_out(midi::NoteOn, MAPPING_CONF(mapping).note, 127);
#if defined(DEBUG_MAPPING)
				Serial.printf("\nDEBUG_MAPPING_TRIGGERS:\tNOTE_ON : %d", MAPPING_CONF(mapping).note);
#endif
		} else  if (!blob->state) {
			midi_enqueue_out(midi::NoteOff, MAPPING_CONF(mapping).note, 0);
#if defined(DEBUG_MAPPING)
				Serial.printf("\nDEBUG_MAPPING_TRIGGERS:\tNOTE_OFF : %d", MAPPING_CONF(mapping).note);
#endif
		}
	}
	return 0;
}
