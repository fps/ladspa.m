/*
  ladspa.m.instrument lv2 plugin
  
  Copyright 2011-2012 David Robillard <d@drobilla.net>
  Copyright 2013-2014 Florian Paul Schmidt <mista.tapas@gmx.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#pragma once

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

#define INSTRUMENT_URI               "http://fps.io/ladspa.m.lv2/instrument"
#define INSTRUMENT__instrument_path  INSTRUMENT_URI "#instrument_path"
#define INSTRUMENT__instrument_chunk INSTRUMENT_URI "#instrument_chunk"
#define INSTRUMENT__applyInstrument  INSTRUMENT_URI "#applyInstrument"
#define INSTRUMENT__freeInstrument   INSTRUMENT_URI "#freeInstrumen"

typedef struct 
{
	LV2_URID atom_Blank;
	LV2_URID atom_Path;
	LV2_URID atom_Chunk;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_URID;
	LV2_URID atom_eventTransfer;
	LV2_URID applyInstrument;
	LV2_URID instrument_path;
	LV2_URID instrument_chunk;
	LV2_URID freeInstrument;
	LV2_URID midi_Event;
	LV2_URID patch_Set;
	LV2_URID patch_Get;
	LV2_URID patch_Message;
	LV2_URID patch_property;
	LV2_URID patch_value;
} InstrumentURIs;

static inline void
map_uris
(
	LV2_URID_Map* map, 
	InstrumentURIs* uris
)
{
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
	uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Chunk          = map->map(map->handle, LV2_ATOM__Chunk);
	uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
	uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
	uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
	uris->applyInstrument    = map->map(map->handle, INSTRUMENT__applyInstrument);
	uris->freeInstrument     = map->map(map->handle, INSTRUMENT__freeInstrument);
	uris->instrument_path    = map->map(map->handle, INSTRUMENT__instrument_path);
	uris->instrument_chunk   = map->map(map->handle, INSTRUMENT__instrument_chunk);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
	uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
	uris->patch_Get          = map->map(map->handle, LV2_PATCH__Get);
	uris->patch_Message      = map->map(map->handle, LV2_PATCH__Message);
	uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
	uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
}

static inline void
print_urid(LV2_URID_Unmap* unmap, LV2_URID urid)
{
	std::cout << unmap->unmap(unmap->handle, urid) << std::endl;
}

static inline void
print_atom
(
	LV2_Atom_Forge* forge,
	const InstrumentURIs* uris,
	LV2_URID_Unmap *unmap,
	LV2_Atom *atom
)
{
	if (lv2_atom_forge_is_object_type(forge, atom->type)) 
	{
		const LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
		print_urid(unmap, obj->body.otype);
	}
	else
	{
		print_urid(unmap, atom->type);
	}
}

static inline LV2_Atom*
write_set_file
(
	LV2_Atom_Forge* forge,
	const InstrumentURIs* uris,
	const char* filename,
	const size_t filename_len
)
{
	std::cout << "write_set_file" << filename << std::endl;
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object
	(
		forge, &frame, 0, uris->patch_Set
	);

	lv2_atom_forge_property_head(forge, uris->patch_property, 0);
	lv2_atom_forge_urid(forge, uris->instrument_path);
	lv2_atom_forge_property_head(forge, uris->patch_value, 0);
	lv2_atom_forge_path(forge, filename, filename_len);

	lv2_atom_forge_pop(forge, &frame);

	return set;
}

static inline const LV2_Atom*
read_set_file
(
	const InstrumentURIs*     uris,
	const LV2_Atom_Object*    obj
)
{
	std::cout << "read_set_file" << std::endl;
	if (obj->body.otype != uris->patch_Set) 
	{
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get property URI. */
	const LV2_Atom* property = NULL;
	lv2_atom_object_get(obj, uris->patch_property, &property, 0);
	if (!property) 
	{
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	} 
	else if (property->type != uris->atom_URID) 
	{
		fprintf(stderr, "Malformed set message has non-URID property.\n");
		return NULL;
	} 
	else if (((LV2_Atom_URID*)property)->body != uris->instrument_path) 
	{
		fprintf(stderr, "Set message for unknown property.\n");
		return NULL;
	}

	// print_urid(self->unmap, property);
	/* Get value. */
	const LV2_Atom* file_path = NULL;
	lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);

	//print_ev_type(uris, file_path->type);
	
	if (!file_path) 
	{
		fprintf(stderr, "Malformed set message has no value.\n");
		return NULL;
	} 
	else if (file_path->type != uris->atom_Path) 
	{
		fprintf(stderr, "Set message value is not a Path.\n");
		return NULL;
	}

	return file_path;
}
