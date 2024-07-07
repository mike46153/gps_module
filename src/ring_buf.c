#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ring_buf.h"

/* Ring bufffer / Circular buffer / Circular Queue implementation
 * The queue grows from the head and shrinks from the tail
 */
#if 0
typedef struct circular_buffer {
    void* data;
    size_t head;
    size_t tail;
    size_t max_size;
    size_t element_size;
} circular_buffer;
#endif
// 初始化循環緩衝區
void circular_buffer_init(circular_buffer* buf, size_t max_size, size_t element_size) {

	if(buf == NULL) {
	        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
	        exit(EXIT_FAILURE);
	}

    buf->data = malloc(max_size * element_size);
    buf->head = 0;
    buf->tail = 0;
    buf->max_size = max_size;
    buf->element_size = element_size;
}

// 釋放循環緩衝區所佔用的記憶體
void circular_buffer_free(circular_buffer* buf) {
	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}

    free(buf->data);
}

// 將數據添加到循環緩衝區中
void circular_buffer_push(circular_buffer* buf, const void* data) {
	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}
    memcpy(buf->data + buf->head * buf->element_size, data, buf->element_size);
    buf->head = (buf->head + 1) % buf->max_size;
    if (buf->head == buf->tail) {
        buf->tail = (buf->tail + 1) % buf->max_size;
    }
}

// 從循環緩衝區中讀取數據
void circular_buffer_pop(circular_buffer* buf, void* data) {
	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}

	if (buf->head == buf->tail) {
	        fprintf(stderr, "Error: circular_buffer_pop - Buffer is empty\n");
	        exit(EXIT_FAILURE);
	}
    memcpy(data, buf->data + buf->tail * buf->element_size, buf->element_size);
    buf->tail = (buf->tail + 1) % buf->max_size;
}

// 返回循環緩衝區中的元素數量
size_t circular_buffer_size(const circular_buffer* buf) {
	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}

    if (buf->head >= buf->tail) {
        return buf->head - buf->tail;
    } else {
        return buf->max_size - buf->tail + buf->head;
    }
}


// 判斷循環緩衝區是否已滿
bool circular_buffer_full(const circular_buffer* buf) {
	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}
    return (buf->head + 1) % buf->max_size == buf->tail;
}

// 判斷循環緩衝區是否已空
bool circular_buffer_empty(const circular_buffer* buf) {

	if(buf == NULL) {
		        fprintf(stderr, "Error: circular_buffer_full - Invalid pointer\n");
		        exit(EXIT_FAILURE);
	}
    return buf->head == buf->tail;
}
