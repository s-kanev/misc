/*
 * Initialize counters.
 * Takes NULL-string terminated array of counter names.
 * Returns 0 on success.
 */ 
extern int init_counters(const char** counters);

/* Free up counter resources. */
extern void deinit_counters(void);

/* 
 * Start collecting counter values.
 * Returns 0 on success.
 */

extern int start_counters(void);


/* 
 * Stop collecting counter values.
 * Returns arrat of counter values.
 */
extern uint64_t *stop_counters(void);
