#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

/*
 * Initialize counters.
 * Takes NULL-string terminated array of counter names.
 * Returns 0 on success.
 */ 
EXTERN int init_counters(const char** counters);

/* Free up counter resources. */
EXTERN void deinit_counters(void);

/* 
 * Start collecting counter values.
 * Returns 0 on success.
 */

EXTERN int start_counters(void);


/* 
 * Stop collecting counter values.
 * Returns array of counter values.
 * reset_counters != 0 will reset counter state.
 * reset_counters == 0 will keep accumulating them.
 */
EXTERN uint64_t *stop_counters(int reset_counters);

/* Same as the above function with reset_counters == 0.
 * Needed so we can enter it with just a jmp. */
EXTERN uint64_t *pause_counters();
