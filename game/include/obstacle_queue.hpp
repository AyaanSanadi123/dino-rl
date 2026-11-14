// obstacle_queue.hpp
#pragma once
#include <stdbool.h>
#include "compat.hpp"

#define MAX_OBSTACLES 5

namespace dino {

    struct Obstacle {
        float x;
        float y;
        float width;
        float height;
        bool is_danger; // true while ahead of/overlapping the Dino; false once cleared
    };

    struct ObstacleQueue {
        struct Obstacle buffer[MAX_OBSTACLES];
        int head;  // Index of the oldest (leftmost) obstacle
        int tail;  // Index where the next new obstacle will be written
        int count; // Current number of obstacles on screen (0 to 5)
    };

    // Reset the queue to empty
    HOST_DEVICE FORCE_INLINE void queue_clear(struct ObstacleQueue* q) {
        q->head = 0;
        q->tail = 0;
        q->count = 0;
    }

    HOST_DEVICE FORCE_INLINE bool queue_is_empty(const struct ObstacleQueue* q) {
        return q->count == 0;
    }

    HOST_DEVICE FORCE_INLINE bool queue_is_full(const struct ObstacleQueue* q) {
        return q->count >= MAX_OBSTACLES;
    }

    // Push a new obstacle to the back (right side of screen)
    HOST_DEVICE FORCE_INLINE bool queue_push(struct ObstacleQueue* q, struct Obstacle obs) {
        if (q->count >= MAX_OBSTACLES) {
            return false;
        }
        q->buffer[q->tail] = obs;
        q->tail = (q->tail + 1) % MAX_OBSTACLES;
        q->count++;
        return true;
    }

    // Pop the oldest obstacle from the front (once it goes off the left edge of screen)
    HOST_DEVICE FORCE_INLINE void queue_pop(struct ObstacleQueue* q) {
        if (q->count <= 0) {
            return;
        }
        q->head = (q->head + 1) % MAX_OBSTACLES;
        q->count--;
    }

    // Access the i-th active obstacle in logical order (0 = closest/leftmost, count-1 = furthest/rightmost)
    HOST_DEVICE FORCE_INLINE struct Obstacle* queue_get(struct ObstacleQueue* q, int i) {
        int physical_idx = (q->head + i) % MAX_OBSTACLES;
        return &q->buffer[physical_idx];
    }

    // Read-only access for const functions like check_collision and renderer
    HOST_DEVICE FORCE_INLINE const struct Obstacle* queue_get_const(const struct ObstacleQueue* q, int i) {
        int physical_idx = (q->head + i) % MAX_OBSTACLES;
        return &q->buffer[physical_idx];
    }

    // Pointer to the oldest (leftmost) obstacle on screen
    HOST_DEVICE FORCE_INLINE struct Obstacle* queue_front(struct ObstacleQueue* q) {
        return &q->buffer[q->head];
    }

    // Pointer to the newest (rightmost) obstacle on screen
    HOST_DEVICE FORCE_INLINE struct Obstacle* queue_back(struct ObstacleQueue* q) {
        int last_idx = (q->tail + MAX_OBSTACLES - 1) % MAX_OBSTACLES;
        return &q->buffer[last_idx];
    }

}