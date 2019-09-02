#ifndef PIZZA_H_
#define PIZZA_H_

/*
 * slices -- read-only access to a char*
 */
typedef struct slice {
    const char* ptr;
    unsigned int len;
} slice;

slice slice_wrap_ptr(const char* p);
slice slice_wrap_ptr_len(const char* p, unsigned int l);

/*
 * buffer -- write-only access to a char*
 */
typedef struct buffer {
    char* ptr;
    unsigned int cap;
    unsigned int pos;
} buffer;


buffer* buffer_build(void);
void buffer_destroy(buffer* b);

unsigned int buffer_length(const buffer* b);

buffer* buffer_set_character(buffer* b, char c);
buffer* buffer_set_string(buffer* b, const char* s);

buffer* buffer_append_character(buffer* b, char c);
buffer* buffer_append_string(buffer* b, const char* s);

// convert between slice and buffer

buffer* slice_to_buffer(const slice s);
slice buffer_to_slice(const buffer* b);

const char* slice_to_string(const slice s, char* string);

void slice_dump(const slice s);

#endif
