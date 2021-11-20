#include "mapping.h"

static u_int8_t mapping_count = 0;
static llist_t mapping_list;

int mapping_setup_all(void)
{
	llist_raz(&mapping_list);
	return 0;
}

void mapping_update_all(void)
{
	for (mapping_t *mapping = (mapping_t *)ITERATOR_START_FROM_HEAD(&mapping_list); mapping != NULL; mapping = (mapping_t *)ITERATOR_NEXT(mapping)) {
		for (blob_t *blob = (blob_t *)ITERATOR_START_FROM_HEAD(&llist_blobs); blob != NULL; blob = (blob_t *)ITERATOR_NEXT(blob)) {
			mapping->impl->func(mapping, blob);
		}
	}
}

mapping_t *mapping_create(mapping_impl_t *impl, u_int8_t size)
{
	mapping_t *mapping = (mapping_t *)malloc(size);
	if (mapping == NULL) {
		return NULL;
	}
	mapping->id = ++mapping_count;
	mapping->impl = impl;
	llist_push_back(&mapping_list, mapping);
	return mapping;
}
