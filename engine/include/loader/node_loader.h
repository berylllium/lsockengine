#pragma once

#include "definitions.h"

#include "core/node.h"

bool lise_node_loader_load(const char* path, lise_node_abstract* out_node);

void lise_node_loader_free(lise_node_abstract* node);
