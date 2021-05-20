/**
 * Implementations of type-specific scheduled variables, whose vtable methods
 * are aware of the size and layout of their respective payloads.
 */

#include "ssm-types.h"
#include "ssm-queue.h" /* For managing inner queues */
#include <stddef.h>    /* For offsetof */

/**
 * Unit type "implementation", which uses underlying the pure event
 * implementation and nothing more.
 */
void (*const initialize_unit)(unit_svt *) = &initialize_event;

/**
 * Implementation of container_of that falls back to ISO C99 when GNU C is not
 * available (from https://stackoverflow.com/a/10269925/10497710)
 */
#ifdef __GNUC__
#define member_type(type, member) __typeof__(((type *)0)->member)
#else
#define member_type(type, member) const void
#endif
#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(member_type(type, member) *){ptr} -                       \
            offsetof(type, member)))

/**
 * Scalar definition helper macro
 */
#define DEFINE_SCHED_VARIABLE_SCALAR(payload_t)                                \
  static sel_t update_##payload_t(struct sv *sv) {                             \
    payload_t##_svt *v = container_of(sv, payload_t##_svt, sv);                \
    v->value = v->later_value;                                                 \
    return SELECTOR_ROOT;                                                      \
  }                                                                            \
  static void assign_##payload_t(struct sv *sv, priority_t prio,               \
                                 const any_t value, sel_t _selector) {         \
    payload_t##_svt *v = container_of(sv, payload_t##_svt, sv);                \
    v->value = (payload_t)value;                                               \
    assign_event(sv, prio);                                                    \
  }                                                                            \
  static void later_##payload_t(struct sv *sv, ssm_time_t then,                \
                                const any_t value, sel_t _selector) {          \
    payload_t##_svt *v = container_of(sv, payload_t##_svt, sv);                \
    v->later_value = (payload_t)value;                                         \
    later_event(sv, then);                                                     \
  }                                                                            \
  static const struct svtable vtable_##payload_t = {                           \
      .update = update_##payload_t,                                            \
      .assign = assign_##payload_t,                                            \
      .later = later_##payload_t,                                              \
      .sel_max = 0,                                                            \
  };                                                                           \
  void initialize_##payload_t(payload_t##_svt *v, payload_t init_value) {      \
    initialize_event(&v->sv);                                                  \
    v->value = init_value;                                                     \
    v->sv.vtable = &vtable_##payload_t;                                        \
  }

/**
 * Define implementation for scalar types
 */
DEFINE_SCHED_VARIABLE_SCALAR(bool)
DEFINE_SCHED_VARIABLE_SCALAR(i8)
DEFINE_SCHED_VARIABLE_SCALAR(i16)
DEFINE_SCHED_VARIABLE_SCALAR(i32)
DEFINE_SCHED_VARIABLE_SCALAR(i64)
DEFINE_SCHED_VARIABLE_SCALAR(u8)
DEFINE_SCHED_VARIABLE_SCALAR(u16)
DEFINE_SCHED_VARIABLE_SCALAR(u32)
DEFINE_SCHED_VARIABLE_SCALAR(u64)
