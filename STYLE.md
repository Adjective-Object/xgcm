# Code Structure and Style Guide

## Code Structure

- **simple_hmap.c**: A dead-simple hashmap and linked list implementation
- **string_buffer.c**: Helper struct to make building strings character by
	character simpler
- **utils.c**: general purpose utility functions which are
	not intrinsically tied to xgcm

- **xgcm_traversal.c**: Traverse specified paths to find files to parse
- **xgcm_parser.c**: Parsing xgcm files and writing outputs
- **xgcm_conf.c**: Configuration struct and opts parsing
- **xgcm.c**: Launching, xgcm (routing input to conf and traversal)

## Code Style

The only hard rules are:

- indent using spaces
- methods returning a new address in memory should begin with the word "create" 
- swag harder than you swig
