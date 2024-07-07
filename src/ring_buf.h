#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct circular_buffer {
    void* data;
    size_t head;
    size_t tail;
    size_t max_size;
    size_t element_size;
} circular_buffer;


// 初始化循環緩衝區
extern void circular_buffer_init(circular_buffer* buf, size_t max_size, size_t element_size) ;

// 釋放循環緩衝區所佔用的記憶體
extern void circular_buffer_free(circular_buffer* buf) ;

// 將數據添加到循環緩衝區中
extern void circular_buffer_push(circular_buffer* buf, const void* data) ;

// 從循環緩衝區中讀取數據
extern void circular_buffer_pop(circular_buffer* buf, void* data) ;

// 返回循環緩衝區中的元素數量
extern size_t circular_buffer_size(const circular_buffer* buf) ;


// 判斷循環緩衝區是否已滿
extern bool circular_buffer_full(const circular_buffer* buf) ;

// 判斷循環緩衝區是否已空
extern bool circular_buffer_empty(const circular_buffer* buf) ;


#endif
